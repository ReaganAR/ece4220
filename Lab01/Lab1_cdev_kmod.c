/* Lab6_cdev_kmod.c
 * ECE4220/7220
 * Based on code from: https://github.com/blue119/kernel_user_space_interfaces_example/blob/master/cdev.c
 * Modified and commented by: Luis Alberto Rivera
 
 You can compile the module using the Makefile provided. Just add
 obj-m += Lab1_cdev_kmod.o

 This Kernel module prints its "MajorNumber" to the system log. The "MinorNumber"
 can be chosen to be 0.
 
 -----------------------------------------------------------------------------------
 Broadly speaking: The Major Number refers to a type of device/driver, and the
 Minor Number specifies a particular device of that type or sometimes the operation
 mode of that device type. On a terminal, try:
    ls -l /dev/
 You'll see a list of devices, with two numbers (among other pieces of info). For
 example, you'll see tty0, tty1, etc., which represent the terminals. All those have
 the same Major number, but they will have different Minor numbers: 0, 1, etc.
 -----------------------------------------------------------------------------------
 
 After installing the module, 
 
 1) Check the MajorNumber using dmesg
 
 2) You can then create a Character device using the MajorNumber returned:
	  sudo mknod /dev/YourDevName c MajorNumber 0
    You need to create the device every time the Pi is rebooted.
	
 3) Change writing permissions, so that everybody can write to the Character Device:
	  sudo chmod go+w /dev/YourDevName
    Reading permissions should be enabled by default. You can check using
      ls -l /dev/YourDevName
    You should see: crw-rw-rw-

 After the steps above, you will be able to use the Character Device.
 If you uninstall your module, you won't be able to access your Character Device.
 If you install it again (without having shutdown the Pi), you don't need to
 create the device again --steps 2 and 3--, unless you manually delete it.
 
 Note: In this implementation, there is no buffer associated to writing to the
 Character Device. Every new string written to it will overwrite the previous one.
*/
  

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kthread.h>	// for kthreads
#include <linux/timer.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/fs.h>

#include <linux/uaccess.h>
#include <linux/gpio.h>




#define MSG_SIZE 40
#define CDEV_NAME "Lab1"	// "YourDevName"

MODULE_LICENSE("GPL");
 
static int major; 
static char msg[MSG_SIZE];

// Function called when the user space program reads the character device.
// Some arguments not used here.
// buffer: data will be placed there, so it can go to user space
// The global variable msg is used. Whatever is in that string will be sent to userspace.
// Notice that the variable may be changed anywhere in the module...
static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)
{
	// Whatever is in msg will be placed into buffer, which will be copied into user space
	ssize_t dummy = copy_to_user(buffer, msg, length);	// dummy will be 0 if successful

	// msg should be protected (e.g. semaphore). Not implemented here, but you can add it.
	msg[0] = '\0';	// "Clear" the message, in case the device is read again.
					// This way, the same message will not be read twice.
					// Also convenient for checking if there is nothing new, in user space.
	
	return length;
}



// structure needed when registering the Character Device. Members are the callback
// functions when the device is read from or written to.
static struct file_operations fops = {
	.read = device_read, 
};

int cdev_module_init(void)
{
	// register the Characted Device and obtain the major (assigned by the system)
	major = register_chrdev(0, CDEV_NAME, &fops);
	if (major < 0) {
     		printk("Registering the character device failed with %d\n", major);
	     	return major;
	}
	printk("Lab1_cdev_kmod example, assigned major: %d\n", major);
	printk("Create Char Device (node) with: sudo mknod /dev/%s c %d 0\n", CDEV_NAME, major);
 	return 0;
}

void cdev_module_exit(void)
{
	// Once unregistered, the Character Device won't be able to be accessed,
	// even if the file /dev/YourDevName still exists. Give that a try...
	unregister_chrdev(major, CDEV_NAME);
	printk("Char Device /dev/%s unregistered.\n", CDEV_NAME);
}  

module_init(cdev_module_init);
module_exit(cdev_module_exit);
