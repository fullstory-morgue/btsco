/*
 *
 *  Bluetooth low-complexity, subband codec (SBC) decoder
 *
 *  Copyright (C) 2004  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include "sbc.h"

#define BUF_SIZE 8192

static void decode(char *filename, char *audiodevice, int tofile)
{
	unsigned char buf[BUF_SIZE], *stream;
	struct stat st;
	off_t filesize;
	sbc_t sbc;
	int fd, ad, pos, streamlen, framelen, count, format = AFMT_S16_BE;

	if (stat(filename, &st) < 0) {
		fprintf(stderr, "Can't get size of file %s: %s\n",
						filename, strerror(errno));
		return;
	}

	filesize = st.st_size;
	stream = malloc(st.st_size);

	if (!stream) {
		fprintf(stderr, "Can't allocate memory for %s: %s\n",
						filename, strerror(errno));
		return;
	}

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Can't open file %s: %s\n",
						filename, strerror(errno));
		goto free;
	}

	if (read(fd, stream, st.st_size) != st.st_size) {
		fprintf(stderr, "Can't read content of %s: %s\n",
						filename, strerror(errno));
		close(fd);
		goto free;
	}

	close(fd);

	pos = 0;
	streamlen = st.st_size;

	ad = open(audiodevice, O_WRONLY | (tofile ? (O_CREAT | O_TRUNC) : 0), tofile ? 0644 : 0);
	if (ad < 0) {
		fprintf(stderr, "Can't open audio device %s: %s\n",
						audiodevice, strerror(errno));
		goto free;
	}

	sbc_init(&sbc, SBC_NULL);

	framelen = sbc_decode(&sbc, stream, streamlen);
	printf("%d Hz, %d channels\n", sbc.rate, sbc.channels);
	if (!tofile) {
		if (ioctl(ad, SNDCTL_DSP_SETFMT, &format) < 0) {
			fprintf(stderr, "Can't set audio format on %s: %s\n",
							audiodevice, strerror(errno));
			goto close;
		}
		if (ioctl(ad, SNDCTL_DSP_CHANNELS, &sbc.channels) < 0) {
			fprintf(stderr, "Can't set number of channels on %s: %s\n",
							audiodevice, strerror(errno));
			goto close;
		}

		if (ioctl(ad, SNDCTL_DSP_SPEED, &sbc.rate) < 0) {
			fprintf(stderr, "Can't set audio rate on %s: %s\n",
							audiodevice, strerror(errno));
			goto close;
		}
	}

	count = 0;
	while (framelen > 0) {
		// we have completed an sbc_decode at this point
		// sbc.len is the length of the frame we just decoded
		// count is the number of decoded bytes yet to be written

		if (count + sbc.len > BUF_SIZE) {
			// buffer is too full to stuff decoded audio in
			// so it must be written to the device
			write(ad, buf, count);
			count = 0;
		}

		// sanity check
		if(count + sbc.len > BUF_SIZE) {
			fprintf(stderr, "buffer size of %d is too small for decoded data (%d)\n", BUF_SIZE, sbc.len + count);
			exit(1);
		}

		// move the latest decoded data into buf and increase the count
		memcpy(buf + count, sbc.data, sbc.len);
		count += sbc.len;

		// push the pointer in the file forward to the next bit to be decoded
		// tell the decoder to decode up to the remaining length of the file (!)
		pos += framelen;
		framelen = sbc_decode(&sbc, stream + pos, streamlen - pos);
	}

	if (count > 0)
		write(ad, buf, count);

close:
	sbc_finish(&sbc);

	close(ad);

free:
	free(stream);
}

static void usage(void)
{
	printf("SBC decoder utility ver %s\n", VERSION);
	printf("Copyright (c) 2004  Marcel Holtmann\n\n");

	printf("Usage:\n"
		"\tsbcdec [options] file(s)\n"
		"\n");

	printf("Options:\n"
		"\t-h, --help           Display help\n"
		"\t-d, --device <dsp>   Sound device\n"
		"\t-v, --verbose        Verbose mode\n"
		"\t-f, --file           Decode to a file\n"
		"\n");
}

static struct option main_options[] = {
	{ "help",	0, 0, 'h' },
	{ "device",	1, 0, 'd' },
	{ "verbose",	0, 0, 'v' },
	{ "file",	1, 0, 'f' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
	char *device = NULL;
	char *file = NULL;
	int i, opt, verbose = 0, tofile = 0;

	while ((opt = getopt_long(argc, argv, "+hd:vf:", main_options, NULL)) != -1) {
		switch(opt) {
		case 'h':
			usage();
			exit(0);

		case 'd':
			device = strdup(optarg);
			break;

		case 'v':
			verbose = 1;
			break;
		case 'f' :
			file = strdup(optarg);
			tofile = 1;
			break;

		default:
			exit(1);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc < 1) {
		usage();
		exit(1);
	}

	for (i = 0; i < argc; i++)
		decode(argv[i], device ? device : file ? file : "/dev/dsp", tofile);

	if (device)
		free(device);

	return 0;
}
