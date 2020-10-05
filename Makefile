VERSION := $(shell cat version 2>/dev/null)

KVER ?= $(shell uname -r)
KVER_MAJOR ?= $(word 1, $(subst ., ,$(KVER))).$(word 2, $(subst ., ,$(KVER)))

ifeq (1,$(DEBUG))
export CFLAGS ?= -O2 -g -pipe -Wall -Werror=format-security -Wp,-D_FORTIFY_SOURCE=2 -Wp,-D_GLIBCXX_ASSERTIONS -fexceptions -fstack-protector-strong -grecord-gcc-switches -m64 -mtune=generic -fasynchronous-unwind-tables -fstack-clash-protection -fcf-protection
endif

# BEGIN LOCAL BUILD PART
obj-m += module/dummy_psu.o

.PHONY: module
module:
	$(MAKE) -C /lib/modules/$(KVER)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(KVER)/build M=$(PWD) clean

client:
	$(MAKE) -C receiver client

test: module client
	sudo rmmod dummy_psu || true
	sudo insmod module/dummy_psu.ko
ifeq (1,$(DEBUG))
	sudo dmesg -n8
	echo "file $(PWD)/module/dummy_psu.c +p" | sudo tee /sys/kernel/debug/dynamic_debug/control
endif
# END LOCAL BUILD PART

install:
	# module part
	mkdir -p $(DESTDIR)/usr/src/dummy_psu-$(VERSION)
	install -m 664 module/dummy_psu.c $(DESTDIR)/usr/src/dummy_psu-$(VERSION)/dummy_psu.c
	install -m 664 module/dummy_psu.h $(DESTDIR)/usr/src/dummy_psu-$(VERSION)/dummy_psu.h
	install -m 664 module/dkms.conf.in $(DESTDIR)/usr/src/dummy_psu-$(VERSION)/dkms.conf
	install -m 664 module/Makefile.dkms $(DESTDIR)/usr/src/dummy_psu-$(VERSION)/Makefile
	sed -i 's/@VERSION@/$(VERSION)/g; s/@NAME@/dummy_psu/g' $(DESTDIR)/usr/src/dummy_psu-$(VERSION)/dkms.conf

	# sender part
	mkdir -p $(DESTDIR)/etc/qubes-rpc
	install -m 775 sender/qubes.PowerSupply $(DESTDIR)/etc/qubes-rpc/

	# receiver part
	mkdir -p $(DESTDIR)/usr/bin/qubes $(DESTDIR)/usr/lib/systemd/system $(DESTDIR)/etc/qubes/post-install.d
	install -m 755 receiver/20-dummy-psu.sh $(DESTDIR)/etc/qubes/post-install.d
	install -m 775 receiver/qubes-psu-client $(DESTDIR)/usr/bin
	install -m 664 receiver/qubes-psu-client@.service receiver/module-load-dummy-psu.service $(DESTDIR)/usr/lib/systemd/system
