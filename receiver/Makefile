CC=gcc

KVER ?= $(shell uname -r)
KVER_MAJOR ?= $(word 1, $(subst ., ,$(KVER))).$(word 2, $(subst ., ,$(KVER)))

.PHONY: qubes-psu-client
client:
	$(CC) $(CFLAGS) qubes-psu-client.c -o qubes-psu-client -I/lib/modules/$(KVER)/build/include/ -I/usr/include/json-c/ -ljson-c
