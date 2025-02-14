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

static struct gpio leds[] = {
		{ 2 }, 
		{ 3 }, 
		{ 4 }
	};

MODULE_LICENSE("GPL");

static int __init lab01_enter(void) {
    printk(KERN_INFO "Module installed!\n");
    
    if (gpio_request_array(leds, ARRAY_SIZE(leds)) != 0) {
        printk(KERN_INFO "Could not request GPIO pins");
        return -1;
    }
        

    gpio_direction_output(2, 0);
    gpio_direction_output(3, 0);
    gpio_direction_output(4, 0);
    gpio_direction_output(5, 0);

    msleep(300);

    gpio_direction_output(2, 1);
    gpio_direction_output(3, 1);
    gpio_direction_output(4, 1);
    gpio_direction_output(5, 1);

    msleep(300);

    gpio_direction_output(2, 0);
    gpio_direction_output(3, 0);
    gpio_direction_output(4, 0);
    gpio_direction_output(5, 0);

    return 0;
}

static void __exit lab01_exit(void) {
    printk(KERN_INFO "Module removed!\n");

    gpio_direction_output(2, 1);
    gpio_direction_output(3, 1);
    gpio_direction_output(4, 1);
    gpio_direction_output(5, 1);

    gpio_free(2);
    gpio_free(3);
    gpio_free(4);
    gpio_free(5);
}

module_init(lab01_enter);
module_exit(lab01_exit);
