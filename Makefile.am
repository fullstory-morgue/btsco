#
#  $Id: Makefile.am,v 1.18 2006/02/10 22:19:01 bmidgley Exp $
#

SUBDIRS = sbc avdtp alsa-plugins

bin_PROGRAMS = btsco

noinst_PROGRAMS = btsco2 a2play a2recv avrecv avsnd

btsco_LDADD = @BLUEZ_LIBS@ @ALSA_LIBS@ -lbluetooth

btsco2_LDADD = @BLUEZ_LIBS@ @ALSA_LIBS@ -lbluetooth

a2play_LDADD = @BLUEZ_LIBS@ -lpthread -lbluetooth sbc/libsbc.a

a2recv_LDADD = @BLUEZ_LIBS@ -lbluetooth sbc/libsbc.a

avrecv_LDADD = @BLUEZ_LIBS@ -lbluetooth

avsnd_LDADD = @BLUEZ_LIBS@ -lbluetooth

AM_CFLAGS = @ALSA_CFLAGS@ @BLUEZ_CFLAGS@

MAINTAINERCLEANFILES = Makefile.in \
	aclocal.m4 configure config.h.in \
	missing install-sh mkinstalldirs
