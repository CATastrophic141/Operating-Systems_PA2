/**
 * File:	lkmasg1.c
 * Adapted for Linux 5.15 by: John Aedo
 * Class:	COP4600-SP23
 * Modified for PA2 Assignment by: Joshua Glaspey
 */

#include <linux/module.h>	  // Core header for modules.
#include <linux/device.h>	  // Supports driver model.
#include <linux/kernel.h>	  // Kernel header for convenient functions.
#include <linux/fs.h>		  // File-system support.
#include <linux/uaccess.h>	  // User access copy function support.
#define DEVICE_NAME "lkmasg1" // Device name.
#define CLASS_NAME "char"	  ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");						 ///< The license type -- this affects available functionality
MODULE_AUTHOR("John Aedo");					 ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("lkmasg1 Kernel Module"); ///< The description -- see modinfo
MODULE_VERSION("0.1");						 ///< A version number to inform users

/**
 * Important variables that store data and keep track of relevant information.
 */
static int major_number;

static struct class *lkmasg1Class = NULL;	///< The device-driver class struct pointer
static struct device *lkmasg1Device = NULL; ///< The device-driver device struct pointer

#define KERNEL_BUFFER_SIZE 1024
static char kernel_buffer[KERNEL_BUFFER_SIZE];
static int buffer_size = 0;  // Current size of the buffer

/**
 * Prototype functions for file operations.
 */
static int open(struct inode *, struct file *);
static int close(struct inode *, struct file *);
static ssize_t read(struct file *filep, char *buffer, size_t len, loff_t *offset);
static ssize_t write(struct file *filep, const char *user_buffer, size_t len, loff_t *offset);

/**
 * File operations structure and the functions it points to.
 */
static struct file_operations fops =
	{
		.owner = THIS_MODULE,
		.open = open,
		.release = close,
		.read = read,
		.write = write,
};

/**
 * Initializes module at installation
 */
int init_module(void)
{
	printk(KERN_INFO "lkmasg1: installing module.\n");

	// Allocate a major number for the device.
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0)
	{
		printk(KERN_ALERT "lkmasg1 could not register number.\n");
		return major_number;
	}
	printk(KERN_INFO "lkmasg1: registered correctly with major number %d\n", major_number);

	// Register the device class
	lkmasg1Class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(lkmasg1Class))
	{ // Check for error and clean up if there is
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(lkmasg1Class); // Correct way to return an error on a pointer
	}
	printk(KERN_INFO "lkmasg1: device class registered correctly\n");

	// Register the device driver
	lkmasg1Device = device_create(lkmasg1Class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(lkmasg1Device))
	{								 // Clean up if there is an error
		class_destroy(lkmasg1Class); // Repeated code but the alternative is goto statements
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(lkmasg1Device);
	}
	printk(KERN_INFO "lkmasg1: device class created correctly\n"); // Made it! device was initialized

	return 0;
}

/*
 * Removes module, sends appropriate message to kernel
 */
void cleanup_module(void)
{
	printk(KERN_INFO "lkmasg1: removing module.\n");
	device_destroy(lkmasg1Class, MKDEV(major_number, 0)); // remove the device
	class_unregister(lkmasg1Class);						  // unregister the device class
	class_destroy(lkmasg1Class);						  // remove the device class
	unregister_chrdev(major_number, DEVICE_NAME);		  // unregister the major number
	printk(KERN_INFO "lkmasg1: Goodbye from the LKM!\n");
	unregister_chrdev(major_number, DEVICE_NAME);
	return;
}

/*
 * Opens device module, sends appropriate message to kernel
 */
static int open(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "lkmasg1: device opened.\n");
	return 0;
}

/*
 * Closes device module, sends appropriate message to kernel
 */
static int close(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "lkmasg1: device closed.\n");
	return 0;
}

static ssize_t read(struct file *filep, char *user_buffer, size_t len, loff_t *offset)
{
	// Report when a character device is read
	printk(KERN_INFO "read stub");

    int bytes_to_read;

    // Determine the number of bytes to read
    bytes_to_read = len < buffer_size ? len : buffer_size;

	// If there is no data available, return 0 to indicate end of file
    if (bytes_to_read == 0)
    {
        return 0;
    }

    // Copy data from kernel space to user space
    if (copy_to_user(user_buffer, kernel_buffer, bytes_to_read) != 0)
    {
        return -EFAULT; // Error occurred while copying data
    }

	// Set the last character of the kernel buffer to \0
	if (copy_to_user(user_buffer + bytes_to_read, "\0", 1) != 0)
	{
		return -EFAULT; // Error occurred while copying data
	}

    // Shift the remaining data in the buffer to the beginning
    for (int i = bytes_to_read; i < buffer_size; ++i)
    {
        kernel_buffer[i - bytes_to_read] = kernel_buffer[i];
    }

    buffer_size -= bytes_to_read;
    return bytes_to_read;
}


/*
 * Writes to the device
 */
static ssize_t write(struct file *filep, const char *user_buffer, size_t len, loff_t *offset)
{
	// Report when a character device is written
	printk(KERN_INFO "write stub");

    int space_available = KERNEL_BUFFER_SIZE - buffer_size;
    int bytes_to_write = len < space_available ? len : space_available;

    // Copy data from user space to kernel space
    if (copy_from_user(kernel_buffer + buffer_size, user_buffer, bytes_to_write) != 0)
    {
        return -EFAULT; // Error occurred while copying data
    }

    buffer_size += bytes_to_write;
    return bytes_to_write;
}
