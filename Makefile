obj-m+=antelabs-clarona.o

# CFLAGS_antelabs-clarona.o :=

all:
	 make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	  $(CC) test-antelabs-clarona.c -o test-antelabs-clarona
prepare:
	sudo apt-get install linux-headers-$(uname -r)
	cp -v /usr/src/linux-headers-$(uname -r)/Module.symvers .
clean:
	 make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	 rm -f test-antelabs-clarona
insert:
	sudo insmod antelabs-clarona.ko
remove:
	sudo rmmod antelabs_clarona
dieharder:
	cat /dev/clarona-rdrand | dieharder -a -g 200
