# Makefile for vsensor kernel module
#
# Usage:
#   make          – build vsensor.ko
#   make clean    – remove build artifacts
#   make load     – insmod vsensor.ko  (requires root)
#   make unload   – rmmod vsensor      (requires root)
#   make reload   – unload + load

KDIR   ?= /lib/modules/$(shell uname -r)/build
PWD    := $(shell pwd)

obj-m += vsensor.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load:
	sudo insmod vsensor.ko

unload:
	sudo rmmod vsensor

reload: unload load

.PHONY: all clean load unload reload
