/**
 * File:	lkmasg1.c
 * Adapted for Linux 5.15 by: John Aedo
 * Class:	COP4600-SP23
 */

#include <linux/module.h>	  // Core header for modules.
#include <linux/device.h>	  // Supports driver model.
#include <linux/kernel.h>	  // Kernel header for convenient functions.
#include <linux/fs.h>		  // File-system support.
#include <linux/uaccess.h>	  // User access copy function support.
#include <linux/slab.h>  // For kmalloc and kfree

#define DEVICE_NAME "lkmasg1" // Device name.
#define CLASS_NAME "char"	  ///< The device class -- this is a character device driver

// Define a maximum buffer size as a constant
#define MAX_BUFFER_SIZE 1024

MODULE_LICENSE("GPL");						 ///< The license type -- this affects available functionality
MODULE_AUTHOR("John Aedo");					 ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("lkmasg1 Kernel Module"); ///< The description -- see modinfo
MODULE_VERSION("0.1");						 ///< A version number to inform users

/**
 * Important variables that store data and keep track of relevant information.
 */
static int major_number;

// Declare a dynamically allocated buffer
static char *module_buffer = NULL;

static struct class *lkmasg1Class = NULL;	///< The device-driver class struct pointer
static struct device *lkmasg1Device = NULL; ///< The device-driver device struct pointer

/**
 * Prototype functions for file operations.
 */
static int open(struct inode *, struct file *);
static int close(struct inode *, struct file *);
static ssize_t read(struct file *, char *, size_t, loff_t *);
static ssize_t write(struct file *, const char *, size_t, loff_t *);

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

/*
 * Reads from device, displays in userspace, and deletes the read data
 */
static ssize_t read(struct file *filep, char *user_buffer, size_t len, loff_t *offset) {
    int bytes_read = 0;
    
    if (!module_buffer) {
        printk(KERN_INFO "lkmasg1: Nothing to read, buffer is empty.\n");
        return 0;
    }

    // Calculate the available data to read from the buffer
    int data_available = bytes_written;

    // Check if there's enough data available to service the read request
    if (len > data_available) {
        len = data_available;
    }

    // Use copy_to_user to safely send data from the module's buffer to user space
    if (copy_to_user(user_buffer, module_buffer, len)) {
        return -EFAULT;
    }

    // Remove the read data from the module's buffer
    memmove(module_buffer, module_buffer + len, data_available - len);
    bytes_written -= len;

    printk(KERN_INFO "lkmasg1: %zu bytes read from the buffer\n", len);

    return len;
}


/*
 * Writes to the device
 */
static ssize_t write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    int bytes_written = 0;
    
    // If the module_buffer is not allocated, allocate it now.
    if (module_buffer == NULL) {
        module_buffer = kmalloc(MAX_BUFFER_SIZE, GFP_KERNEL);
        if (!module_buffer) {
            printk(KERN_ALERT "lkmasg1: Memory allocation failed\n");
            return -ENOMEM;
        }
    }
    
    // Calculate the available space in the buffer
    int space_available = MAX_BUFFER_SIZE - bytes_written;

    // Check if there's enough space to store the write request
    if (len > space_available) {
        len = space_available;
    }

    // Use copy_from_user to safely copy data from the user space to the module's buffer
    if (copy_from_user(module_buffer + bytes_written, buffer, len)) {
        return -EFAULT;
    }

    // Update bytes_written to keep track of the data in the buffer
    bytes_written += len;

    printk(KERN_INFO "lkmasg1: %zu bytes written to the buffer\n", len);
    
    return len;
}