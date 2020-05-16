obj-m+=clarona.o

all:
	 make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	  $(CC) test-clarona.c -o test-clarona
prepare:
	sudo apt-get install linux-headers-$(uname -r)
	cp -v /usr/src/linux-headers-$(uname -r)/Module.symvers .
clean:
	 make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	 rm -f test-clarona
insert:
	sudo insmod clarona.ko
remove:
	sudo rmmod clarona
dieharder:
	cat /dev/clarona-rdrand | dieharder -a -g 200
