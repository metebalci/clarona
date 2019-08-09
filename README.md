# Clarona RDSEED &amp; RDRAND Character Device Driver

## Description

Clarona is a Linux driver providing /dev/clarona-rdseed and /dev/clarona-rdrand character devices. These read-only character devices enable direct-access to the output of RDSEED and RDRAND instructions in supported architectures.

Detailed explanation of Intel's implementation can be read here: <https://software.intel.com/en-us/articles/intel-digital-random-number-generator-drng-software-implementation-guide>

## Installation

Just use make and then insmod. You need to have kernel headers.

## Supported Architectures

Driver is tested on Linux Kernel 4.15.0, Ubuntu 18.04 x86_64 running on an Intel Xeon E5-1245 v5.

cat /proc/cpuinfo output includes:
```
vendor_id   : GenuineIntel
cpu family  : 6
model       : 94
model name  : Intel(R) Xeon(R) CPU E3-1245 v5 @ 3.50GHz
flags       : many flags including rdrand and rdseed
```

## How to Use

The character devices /dev/clarona-rdseed and /dev/clarona-rdrand can be read by any means.

If you want to use them to feed Java Secure Random algorithms, there are two ways:

1. run JVM with -Djava.security.egd=/dev/clarona-rdseed (this will use /dev/clarona-rdseed instead of /dev/random)
2. implement a custom Secure Random algorithm to use one or both character devices

## Performance

20 MBit/sec True Random Bit Stream (RDSEED output) generation throughput can be easily achieved. Actual number depends on various factors.

```
$ sudo dd if=/dev/clarona-rdseed of=out bs=8 count=1MB
1000000+0 records in
1000000+0 records out
8000000 bytes (8.0 MB, 7.6 MiB) copied, 3.02168 s, 2.6 MB/s
```
