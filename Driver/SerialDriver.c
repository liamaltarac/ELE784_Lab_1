#include <linux/kernel.h>
#include <linux/kthread.h>

#include <linux/fs.h>
#include <linux/types.h>

#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>

#include <linux/module.h>
#include <linux/moduleparam.h>

MODULE_AUTHOR("Liam");
MODULE_LICENSE("Dual BSD/GPL");

int serial_major;
int serial_minor;

struct class * dev_class;

/*struct serial_dev{
	int size;
	struct cdev cdev;
}*/

struct file_operations serial_fops;

dev_t dev;

int err;

static int __init serial_driver_init (void) {

	struct cdev *my_cdev = cdev_alloc();
	my_cdev->ops = &serial_fops;

	alloc_chrdev_region(&dev, 0, 1, "SerialDriver");		//Allocation dynamique d`un numero d`unite-materiel 
	dev_class = class_create(THIS_MODULE, "Serial Driver");
	device_create(dev_class, NULL, dev, NULL, "My Serial Driver");
	
	cdev_init(my_cdev, &serial_fops);
	my_cdev->owner = THIS_MODULE;

	err = cdev_add(my_cdev, dev, 1);

	printk(KERN_WARNING"Adding SerialDriver : %d\n", err);

	return 0;

}

static void __exit serial_driver_exit (void) {

	device_destroy(dev_class, dev);
	class_destroy(dev_class);

	unregister_chrdev_region(dev, 1);

	printk(KERN_WARNING"Removing SerialDriver : Goodbye !\n");

	return;

}

module_init(serial_driver_init);
module_exit(serial_driver_exit);


