VERSION := $(shell cat version 2>/dev/null)

KVER ?= $(shell uname -r)
KVER_MAJOR ?= $(word 1, $(subst ., ,$(KVER))).$(word 2, $(subst ., ,$(KVER)))

# BEGIN LOCAL BUILD PART
obj-m += module/dummy_psu.o

.PHONY: module
module:
	$(MAKE) -C /lib/modules/$(KVER)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(KVER)/build M=$(PWD) clean

client:
	$(MAKE) -C vm client

test: module
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
	
	# Header waiting for upstream
	install -m 664 vm/power_supply.h-$(KVER_MAJOR) $(DESTDIR)/usr/src/dummy_psu-$(VERSION)/power_supply.h

	# dom0 part
	mkdir -p $(DESTDIR)/etc/qubes-rpc
	install -m 775 dom0/qubes.PowerSupply $(DESTDIR)/etc/qubes-rpc/

	# VM part
	mkdir -p $(DESTDIR)/usr/bin/qubes $(DESTDIR)/usr/lib/systemd/system $(DESTDIR)/etc/qubes/post-install.d
	install -m 755 vm/20-dummy-psu.sh $(DESTDIR)/etc/qubes/post-install.d
	install -m 775 vm/qubes-psu-client $(DESTDIR)/usr/bin
	install -m 664 vm/qubes-psu-client.service vm/module-load-dummy-psu.service $(DESTDIR)/usr/lib/systemd/system
