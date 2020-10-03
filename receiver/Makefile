CC=gcc

KVER ?= $(shell uname -r)
KVER_MAJOR ?= $(word 1, $(subst ., ,$(KVER))).$(word 2, $(subst ., ,$(KVER)))

.PHONY: qubes-psu-client
client:
	# Header waiting for upstream
	cp power_supply.h-$(KVER_MAJOR) power_supply.h
	$(CC) $(CFLAGS) qubes-psu-client.c -o qubes-psu-client -I/lib/modules/$(KVER)/build/include/ -I/usr/include/json-c/ -ljson-c
