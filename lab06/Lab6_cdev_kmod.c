/* Lab6_cdev_kmod.c
 * ECE4220/7220
 * Based on code from: https://github.com/blue119/kernel_user_space_interfaces_example/blob/master/cdev.c
 * Modified and commented by: Luis Alberto Rivera
 
 You can compile the module using the Makefile provided. Just add
 obj-m += Lab6_cdev_kmod.o

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
#define CDEV_NAME "Lab6"	// "YourDevName"

MODULE_LICENSE("GPL");
 
static int major; 
static char msg[MSG_SIZE];

char tone = 'A';

// Declare button GPIOs so IRQ Handler function can point at these
static int GPIO_Button1 = 16;
static int GPIO_Button2 = 17;
static int GPIO_Button3 = 18;
static int GPIO_Button4 = 19;
static int GPIO_Button5 = 20;

// Declare IRQs here so they can be freed on removal
static int IRQ_Button1;
static int IRQ_Button2;
static int IRQ_Button3;
static int IRQ_Button4;
static int IRQ_Button5;

static const struct gpio buttons[] = {
	{ 16 }, 
	{ 17 }, 
	{ 18 },
	{ 19 },
	{ 20 }
};

static struct task_struct *kthread1;

/* Interrupt handler for all five button GPIO pins. Will be called on a raising edge.
 * dev_id is an int pointer pointing at the GPIO pin # raised.
 */
static irqreturn_t gpio_irq_handler(int irq,void *dev_id) {
    // Switch based on dev_id to determine which button was pressed
	switch(*(int*) dev_id){
		case 16: // Button 1
			printk("Message from button: A\n");
			tone='A';
			break;
		case 17: // Button 2
			printk("Message from button: B\n");
			tone='B';
			break;

		case 18: // Button 3
			printk("Message from button: C\n");
			tone='C';
			break;
		case 19: // Button 4
			printk("Message from button: D\n");
			tone='D';
			break;
		case 20: // Button 5
			printk("Message from button: E\n");
			tone='E';
			break;
		default:
			break;
	}
  
	return IRQ_HANDLED;
}


// Function to be associated with the kthread; what the kthread executes.
int kthread_fn(void *ptr)
{
	printk("In kthread_fn\n");

	// The ktrhead does not need to run forever. It can execute something
	// and then leave.
	while(1)
	{

		// Write code to generate a square wave on BCM 6 (SPKR) (GPIO_OUT6)
		if(tone == 'A'){
			gpio_set_value(6, 1);
			udelay(1800);
			gpio_set_value(6, 0);
			udelay(1800);
		}

		else if(tone == 'B'){
			gpio_set_value(6, 1);
			udelay(1600);
			gpio_set_value(6, 0);
			udelay(1600);
		}

		else if(tone == 'C'){
			gpio_set_value(6, 1);
			udelay(1400);
			gpio_set_value(6, 0);
			udelay(1400);
		}

		else if(tone == 'D'){
			gpio_set_value(6, 1);
			udelay(1200);
			gpio_set_value(6, 0);
			udelay(1200);
		}

		else if(tone == 'E'){
			gpio_set_value(6, 1);
			udelay(1000);
			gpio_set_value(6, 0);
			udelay(1000);
		}
		



		//msleep(1000);	// good for > 10 ms
		//msleep_interruptible(1000); // good for > 10 ms
		//udelay(unsigned long usecs);	// good for a few us (micro s)
		//usleep_range(unsigned long min, unsigned long max); // good for 10us - 20 ms
		
		
		// In an infinite loop, you should check if the kthread_stop
		// function has been called (e.g. in clean up module). If so,
		// the kthread should exit. If this is not done, the thread
		// will persist even after removing the module.
		if(kthread_should_stop()) {
			do_exit(0);
		}
				
		// comment out if your loop is going "fast". You don't want to
		// printk too often. Sporadically or every second or so, it's okay.
		//printk("Count: %d\n", ++count);
	}
	
	return 0;
}














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

// Function called when the user space program writes to the Character Device.
// Some arguments not used here.
// buff: data that was written to the Character Device will be there, so it can be used
//       in Kernel space.
// In this example, the data is placed in the same global variable msg used above.
// That is not needed. You could place the data coming from user space in a different
// string, and use it as needed...
static ssize_t device_write(struct file *filp, const char __user *buff, size_t len, loff_t *off)
{
	ssize_t dummy;
	
	if(len > MSG_SIZE)
		return -EINVAL;
	
	// unsigned long copy_from_user(void *to, const void __user *from, unsigned long n);
	dummy = copy_from_user(msg, buff, len);	// Transfers the data from user space to kernel space
	if(len == MSG_SIZE)
		msg[len-1] = '\0';	// will ignore the last character received.
	else
		msg[len] = '\0';
	
	// You may want to remove the following printk in your final version.
	printk("Message from user space: %s\n", msg);
	tone = msg[0];
	
	return len;		// the number of bytes that were written to the Character Device.
}

// structure needed when registering the Character Device. Members are the callback
// functions when the device is read from or written to.
static struct file_operations fops = {
	.read = device_read, 
	.write = device_write,
};

int cdev_module_init(void)
{
	// register the Characted Device and obtain the major (assigned by the system)
	major = register_chrdev(0, CDEV_NAME, &fops);
	if (major < 0) {
     		printk("Registering the character device failed with %d\n", major);
	     	return major;
	}
	printk("Lab6_cdev_kmod example, assigned major: %d\n", major);
	printk("Create Char Device (node) with: sudo mknod /dev/%s c %d 0\n", CDEV_NAME, major);

	// register the kernel module and ISRs required for button presses
	// Request Input Pins
	if (gpio_request_array(buttons, ARRAY_SIZE(buttons)) != 0) {
        printk(KERN_NOTICE "Could not request button GPIO pins");
        return -1;
    }

	gpio_request(6,NULL);
	gpio_direction_output(6, 0);
	
	gpio_direction_input(16);
	gpio_direction_input(17);
    gpio_direction_input(18);
	gpio_direction_input(19);
	gpio_direction_input(20);



	// Assign IRQs from button GPIOs
	IRQ_Button1 = gpio_to_irq(16);
	IRQ_Button2 = gpio_to_irq(17);
	IRQ_Button3 = gpio_to_irq(18);
	IRQ_Button4 = gpio_to_irq(19);
	IRQ_Button5 = gpio_to_irq(20);

	 	/* Request the interrupt / attach handlers
 	 * Enable (Async) Rising Edge detection
	 * The Fourth argument string can be different (you give the name)
	 * The last argument is a variable needed to identify the handler, but can be set to NULL
	 */
	if (
	request_irq(IRQ_Button1,(void *)gpio_irq_handler, IRQF_TRIGGER_RISING,"Button Interrupt 1", &GPIO_Button1) ||     //device id for shared IRQ
	request_irq(IRQ_Button2,(void *)gpio_irq_handler, IRQF_TRIGGER_RISING,"Button Interrupt 2", &GPIO_Button2) ||
	request_irq(IRQ_Button3,(void *)gpio_irq_handler, IRQF_TRIGGER_RISING,"Button Interrupt 3", &GPIO_Button3) ||
	request_irq(IRQ_Button4,(void *)gpio_irq_handler, IRQF_TRIGGER_RISING,"Button Interrupt 4", &GPIO_Button4) ||
	request_irq(IRQ_Button5,(void *)gpio_irq_handler, IRQF_TRIGGER_RISING,"Button Interrupt 5", &GPIO_Button5) != 0) {
		printk(KERN_NOTICE "Could not request IRQs");
		return -1;
	}

	// Create kernel module which plays the note specified
	char kthread_name[]="SoundGeneration_kthread";	// try running  ps -ef | grep SoundGeneration
	kthread1 = kthread_create(kthread_fn, NULL, kthread_name);
	
    if((kthread1))	// true if kthread creation is successful
    {
        printk("Inside if\n");
		// kthread is dormant after creation. Needs to be woken up
        wake_up_process(kthread1);
    }





	printk(KERN_INFO "*************Module Inserted**********\n");
 	return 0;
}

void cdev_module_exit(void)
{
	// Remove the interrupt Handlers
	free_irq(IRQ_Button1, &GPIO_Button1);
	free_irq(IRQ_Button2, &GPIO_Button2);
	free_irq(IRQ_Button3, &GPIO_Button3);
	free_irq(IRQ_Button4, &GPIO_Button4);
	free_irq(IRQ_Button5, &GPIO_Button5);

	gpio_free_array(buttons, ARRAY_SIZE(buttons));
	gpio_free(6);

	// Once unregistered, the Character Device won't be able to be accessed,
	// even if the file /dev/YourDevName still exists. Give that a try...
	unregister_chrdev(major, CDEV_NAME);
	printk("Char Device /dev/%s unregistered.\n", CDEV_NAME);

	
	int ret;
	// the following doesn't actually stop the thread, but signals that
	// the thread should stop itself (with do_exit above).
	// kthread should not be called if the thread has already stopped.
	ret = kthread_stop(kthread1);
								
	if(!ret)
		printk("Kthread stopped\n");

}
module_init(cdev_module_init);
module_exit(cdev_module_exit);
