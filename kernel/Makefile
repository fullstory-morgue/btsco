obj-m += snd-bt-sco.o

snd-bt-sco-objs := btsco.o

ifneq ($(__KERNEL__),)
ifndef CONFIG_SND_HWDEP
$(error CONFIG_SND_HWDEP needs to be set in the kernel configuration)
endif
endif


default:
	@make -C /lib/modules/`uname -r`/build M=`pwd` modules

install:
	@make -C /lib/modules/`uname -r`/build M=`pwd` modules_install

clean:
	@make -C /lib/modules/`uname -r`/build M=`pwd` clean
