CFLAGS_dummy_psu.o := -DDEBUG

obj-m += dummy_psu.o

all:
	sudo ln -sf /home/user/dummy-psu2/dummy_psu.h /usr/lib/modules/$(shell uname -r)/build/include/uapi/linux/
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	sudo rmmod dummy_psu || true
	rm -f client
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test: clean all
	sudo insmod dummy_psu.ko

.PHONY: client
client:
	gcc client.c -o client -I/lib/modules/$(shell uname -r)/build/include/
