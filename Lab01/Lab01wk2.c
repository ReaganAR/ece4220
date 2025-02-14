#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  
#include <linux/gpio.h>     
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");

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

static const struct gpio leds[] = {
		{ 2 }, 
		{ 3 }, 
		{ 4 },
		{ 5 }
	};

static const struct gpio buttons[] = {
		{ 16 }, 
		{ 17 }, 
		{ 18 },
		{ 19 },
		{ 20 }
	};

/* Interrupt handler for all five button GPIO pins. Will be called on a raising edge.
 * dev_id is an int pointer pointing at the GPIO pin # raised.
 */
static irqreturn_t gpio_irq_handler(int irq,void *dev_id) {
    // Switch based on dev_id to determine which button was pressed
	switch(*(int*) dev_id){
		case 16: // Button 1
			gpio_set_value(2, 1);
			break;
		case 17: // Button 2
			gpio_set_value(3, 1);
			break;
		case 18: // Button 3
			gpio_set_value(4, 1);
			break;
		case 19: // Button 4
			gpio_set_value(5, 1);
			break;
		case 20: // Button 5
			gpio_set_value(2, 0);
			gpio_set_value(3, 0);
			gpio_set_value(4, 0);
			gpio_set_value(5, 0);
			break;
		default:
			break;
	}
  
	return IRQ_HANDLED;
}

// Module Init Function
static int __init lab01_enter(void) {
	// Request Output Pins (Note: Should be moved to userspace i.e. WiringPi)
    if (gpio_request_array(leds, ARRAY_SIZE(leds)) != 0) {
        printk(KERN_NOTICE "Could not request LED GPIO pins");
        return -1;
    }

	// Request Input Pins
	if (gpio_request_array(buttons, ARRAY_SIZE(buttons)) != 0) {
        printk(KERN_NOTICE "Could not request button GPIO pins");
        return -1;
    }

	// Configure pin directions and initial behavior (Note: outputs should be moved to userspace i.e. WiringPi)
	gpio_direction_output(2, 0);
    gpio_direction_output(3, 0);
    gpio_direction_output(4, 0);
    gpio_direction_output(5, 0);

    gpio_direction_input(16);
	gpio_direction_input(17);
    gpio_direction_input(18);
	gpio_direction_input(19);

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
	
	printk(KERN_INFO "*************Module Inserted**********\n");
	return 0;
}

// Module Exit Function
static void __exit lab01_exit(void) {
	// Remove the interrupt Handlers
	free_irq(IRQ_Button1, &GPIO_Button1);
	free_irq(IRQ_Button2, &GPIO_Button2);
	free_irq(IRQ_Button3, &GPIO_Button3);
	free_irq(IRQ_Button4, &GPIO_Button4);
	free_irq(IRQ_Button5, &GPIO_Button5);
	
	// Free the GPIO pins
	gpio_free_array(leds, ARRAY_SIZE(leds));
	gpio_free_array(buttons, ARRAY_SIZE(buttons));

	printk(KERN_INFO "*************Module Removed**********\n");
}
 
module_init(lab01_enter);
module_exit(lab01_exit);
