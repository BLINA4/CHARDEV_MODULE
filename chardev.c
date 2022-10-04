/* 
 * chardev.c: read-only device driver that implements a method which returns how many times 
 * you've read from the dev file.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "chardev.h"

static int Major;           /* Major id of our device */          
static int Device_Open = 0; /* Is our device opened?! */
static char msg[BUF_LEN];   /* The message which device returns */
static char *msg_Ptr;       /* Pointer to message */

static struct file_operations fops = 
{
  .read = device_read, 
  .write = device_write,
  .open = device_open,
  .release = device_release
};

/*
 * Function called on creating module.
 */
int init_module( void )
{
  Major = register_chrdev(0, DEVICE_NAME, &fops);

  /* Error assigning device major identifier */
  if (Major < 0)
  {
    printk(KERN_ALERT "Registering char device failed with %d\n", Major);
    return Major;
  }

  printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
  printk(KERN_INFO "the driver, create a dev file with\n");
  printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
  printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
  printk(KERN_INFO "the device file.\n");
  printk(KERN_INFO "Remove the device file and module when done.\n");

  return SUCCESS;
}

/*
 * Function called on deleting module.
 */
void cleanup_module( void )
{
  unregister_chrdev(Major, DEVICE_NAME);
}

/*
 * Methods
 */
static int device_open( struct inode *inode, struct file *file )
{
  static int counter = 0;

  if (Device_Open)
    return -EBUSY;

  Device_Open++;

  sprintf(msg, "I already told you %d times Hello world!\n", counter++);
  msg_Ptr = msg;
  try_module_get(THIS_MODULE);

  return SUCCESS;
}

static int device_release( struct inode *inode, struct file *file )
{
  Device_Open--;

  module_put(THIS_MODULE);
  
  return 0;
}

static ssize_t device_read( struct file *filp, char *buffer, size_t length, loff_t * offset )
{
  int bytes_read = 0;

  if (*msg_Ptr == 0)
    return 0;

  while (length && *msg_Ptr)
  {
    put_user(*(msg_Ptr++), buffer++);
    length--;
    bytes_read++;
  }

  return bytes_read;
}

static ssize_t device_write( struct file *filp, const char *buff, size_t len, loff_t * offset )
{
  printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
  return -EINVAL;
}

MODULE_LICENSE("GPL");

