/*
 * RDSEED & RDRAND Character Device Driver
 *
 * Copyright (C) 2017 by Mete Balci <info@metebalci.com>
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "clarona"
#define CLASS_NAME "clarona"
#define DEVICE_NAME_RDSEED "clarona-rdseed"
#define DEVICE_NAME_RDRAND "clarona-rdrand"

#define MINOR_RDSEED 1
#define MINOR_RDRAND 2

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Mete Balci <info@metebalci.com>");
MODULE_DESCRIPTION ("RDSEED & RDRAND Character Device Driver");
MODULE_VERSION ("0.1");

static int majorNumber;
static struct class *charClass = NULL;
static struct device *rdseedDevice = NULL;
static struct device *rdrandDevice = NULL;

static int dev_open (struct inode *, struct file *);
static ssize_t rdseed_dev_read (struct file *, char *, size_t, loff_t *);
static ssize_t rdrand_dev_read (struct file *, char *, size_t, loff_t *);

static const struct file_operations fops = {
  .open = dev_open,
};

static const struct file_operations rdseed_fops = {
  .read = rdseed_dev_read,
};

static const struct file_operations rdrand_fops = {
  .read = rdrand_dev_read,
};

static int
is_rdrand_available (void)
{

  unsigned int ecx;

  asm volatile ("movl $1, %%eax; cpuid; movl %%ecx, %0":"=q" (ecx));

  if (ecx & 0x40000000)
    {
      return 1;
    }
  else
    {
      return 0;
    }

}

static int
is_rdseed_available (void)
{

  unsigned int ebx;

  asm
    volatile ("movl $7, %%eax; mov $0, %%ecx; cpuid; movl %%ebx, %0":"=q"
	      (ebx));

  if (ebx & 0x00040000)
    {
      return 1;
    }
  else
    {
      return 0;
    }

}

static int __init
clarona_init (void)
{

  size_t sizeOfUnsignedLong;
  int rdseed_available, rdrand_available;

  printk (KERN_INFO "clarona: Initializing...\n");

  rdseed_available = is_rdseed_available ();
  rdrand_available = is_rdrand_available ();

  printk (KERN_INFO "clarona: rdseed available: %s\n",
	  rdseed_available ? "yes" : "no");
  printk (KERN_INFO "clarona: rdrand available: %s\n",
	  rdrand_available ? "yes" : "no");

  if (!rdseed_available && !rdrand_available)
    {
      printk (KERN_ALERT
	      "clarona: Neither rdrand nor rdseed is available\n");
      return -1;
    }

  // dynamically try to register to get a major number
  majorNumber = register_chrdev (0, DEVICE_NAME, &fops);

  if (majorNumber < 0)
    {
      printk (KERN_ALERT
	      "clarona: Failed to register a major number\n");
      return majorNumber;
    }
  else
    {
      printk (KERN_INFO
	      "clarona: Registered with major number: %d\n",
	      majorNumber);
    }

  // register the device class
  charClass = class_create (THIS_MODULE, CLASS_NAME);
  if (IS_ERR (charClass))
    {
      unregister_chrdev (majorNumber, DEVICE_NAME);
      printk (KERN_ALERT
	      "clarona: Failed to register device class\n");
      return PTR_ERR (charClass);
    }
  else
    {
      printk (KERN_INFO "clarona: Class registered correctly\n");
    }

  if (rdseed_available)
    {

      // register the rdseed device driver
      rdseedDevice =
	device_create (charClass, NULL, MKDEV (majorNumber, MINOR_RDSEED),
		       NULL, DEVICE_NAME_RDSEED);
      if (IS_ERR (rdseedDevice))
	{
	  class_unregister (charClass);
	  class_destroy (charClass);
	  unregister_chrdev (majorNumber, DEVICE_NAME);
	  printk (KERN_ALERT
		  "clarona: Failed to create the rdseed device\n");
	  return PTR_ERR (rdseedDevice);
	}
      else
	{
	  printk (KERN_INFO
		  "clarona: rdseed device created (%d,%d)\n",
		  majorNumber, MINOR_RDSEED);
	}
    }
  else
    {
      printk (KERN_WARNING "clarona: rdseed is not available");
    }

  if (rdrand_available)
    {

      // register the rdrand device driver
      rdrandDevice =
	device_create (charClass, NULL, MKDEV (majorNumber, MINOR_RDRAND),
		       NULL, DEVICE_NAME_RDRAND);
      if (IS_ERR (rdrandDevice))
	{
	  if (rdseedDevice != NULL)
	    {
	      device_destroy (charClass, MKDEV (majorNumber, MINOR_RDSEED));
	    }
	  class_unregister (charClass);
	  class_destroy (charClass);
	  unregister_chrdev (majorNumber, DEVICE_NAME);
	  printk (KERN_ALERT
		  "clarona: Failed to create the rdrand device\n");
	  return PTR_ERR (rdseedDevice);
	}
      else
	{
	  printk (KERN_INFO
		  "clarona: rdrand device created (%d,%d)\n",
		  majorNumber, MINOR_RDRAND);
	}

    }
  else
    {
      printk (KERN_WARNING "clarona: rdrand is not available");
    }

  sizeOfUnsignedLong = sizeof (unsigned long);

  printk (KERN_INFO "clarona: sizeof(unsigned long)=%lu\n",
	  sizeof (unsigned long));

  if (sizeOfUnsignedLong == 8)
    {
      // rdseed and rdrand output is maximum 8 bytes, it is the best if we always use this
      printk (KERN_INFO
	      "clarona: 8 is optimum and good for operation\n");
    }
  else if (sizeOfUnsignedLong < 8)
    {
      // in this case, I guess asm code in rd_dev_read runs but not sure what happens
      printk (KERN_INFO
	      "clarona: <8 is not optimum, maybe harmful to operation\n");
      return -1;
    }
  else if (sizeOfUnsignedLong > 8)
    {
      // in this case, probably asm code in rd_dev_read fails
      printk (KERN_INFO
	      "clarona: >8 is not optimum, potentially harmful to operation\n");
      return -1;
    }

  printk (KERN_INFO "clarona: Initialization done.\n");

  return 0;
}

static void __exit
clarona_exit (void)
{
  printk (KERN_INFO "clarona: Exiting...\n");

  device_destroy (charClass, MKDEV (majorNumber, MINOR_RDRAND));
  device_destroy (charClass, MKDEV (majorNumber, MINOR_RDSEED));

  class_unregister (charClass);
  class_destroy (charClass);

  unregister_chrdev (majorNumber, DEVICE_NAME);

  printk (KERN_INFO "clarona: Exit done.\n");
}

static int
dev_open (struct inode *inode, struct file *filp)
{

  int minor;
  minor = iminor (inode);

  if (minor == MINOR_RDSEED)
    {
      filp->f_op = &rdseed_fops;
      return 0;
    }
  else if (minor == MINOR_RDRAND)
    {
      filp->f_op = &rdrand_fops;
      return 0;
    }
  else
    {
      return -ENXIO;
    }

}

static ssize_t
rd_dev_read (int opId, struct file *filep, char *buffer, size_t len,
	     loff_t * offset)
{

  unsigned long randout;
  unsigned char flag = 0;
  char *ops[] = { "rdseed", "rdrand" };
  char *op = ops[opId];

  if (opId == 0)
    {

      asm volatile ("rdseed %0; setc %1":"=q" (randout), "=q" (flag));

    }
  else if (opId == 1)
    {

      asm volatile ("rdrand %0; setc %1":"=q" (randout), "=q" (flag));

    }
  else
    {
      return 0;
    }

  // printk (KERN_DEBUG "clarona: %s ret=%lx\n", op, randout);
  // printk (KERN_DEBUG "clarona: %s flg=0x%x\n", op, flag);

  if (flag == 1)
    {

      size_t lencopied;

      // printk (KERN_DEBUG "clarona: %s return is valid", op);

      if (len > sizeof (unsigned long))
	{
	  lencopied = sizeof (unsigned long);
	}
      else
	{
	  lencopied = len;
	}

      // printk (KERN_DEBUG
	    //  "clarona: copying %lu bytes to user space\n",
	    //  lencopied);

      if (copy_to_user (buffer, ((char *) &randout), lencopied))
	{
	  return -EFAULT;
	}

      return lencopied;

    }
  else
    {

      printk (KERN_WARNING
	      "clarona: %s return is invalid, flag=0x%x\n", op,
	      flag);

      return 0;

    }

}

static ssize_t
rdseed_dev_read (struct file *filep, char *buffer, size_t len,
		 loff_t * offset)
{
  return rd_dev_read (0, filep, buffer, len, offset);
}

static ssize_t
rdrand_dev_read (struct file *filep, char *buffer, size_t len,
		 loff_t * offset)
{
  return rd_dev_read (1, filep, buffer, len, offset);
}

module_init (clarona_init);
module_exit (clarona_exit);
