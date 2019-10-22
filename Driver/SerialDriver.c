#include <linux/kernel.h>
#include <linux/kthread.h>

#include <linux/fs.h>
#include <linux/types.h>

#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>

#include <linux/module.h>
#include <linux/moduleparam.h>

#include <asm_generic/bitops.h>

#define MAXSIZE = 255;

MODULE_AUTHOR("Liam");
MODULE_LICENSE("Dual BSD/GPL");

struct class * dev_class;

struct serial_driver_struct{

	int size;
	struct cdev *cdev;

	char non_blocking;

}serial = {
	.id_in = 0,
	.id_out = 0,
	.num_data = 0,
};

struct file_operations serial_fops = {
	.owner   = THIS_MODULE,
	.open    = serial_driver_open,
	.release = serial_driver_release,
};

spinlock_t * tp_spinlock;
spinlock_t * cu_spinlock;

uint8_t id_in, id_out; 	//Premiere et derniere indice du tampon circulaire
uint8_t num_data;
char buffer[MAXSIZE];

wait_queue_head_t wait_rx;


//kuid_t current_user = NULL;

dev_t dev;

int err;

static int __init serial_driver_init (void) {

	serial.cdev = cdev_alloc();
	serial.cdev->ops = &serial_fops;

	alloc_chrdev_region(&dev, 0, 1, "SerialDriver");		//Allocation dynamique d`un numero d`unite-materiel 
	dev_class = class_create(THIS_MODULE, "Serial Driver");
	device_create(dev_class, NULL, dev, NULL, "My Serial Driver");
	
	cdev_init(serial.cdev, &serial_fops);
	//serial.cdev->owner = THIS_MODULE;

	err = cdev_add(serial.cdev, dev, 1);

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

static int serial_driver_open(struct inode *inode, struct file *flip){
	
	static kuid current_user = inode->i_uid;
	static int count = 0;
	struct serial_dev *dev;  // cette structure  contient ... ?

	spin_lock(cu_spinlock);
	serial.user_id = inode->i_uid;
	spin_unlock(cu_spinlock);

	if(inode->i_uid == current_user){
		count++;
		filp->private_data = &serial;

		//Identifie l'unite-materiel
		//Initilisation personnelle

		/*if(flip->f_flags & O_ACCMODE){
			//Si flip->f_flags est en mode O_RDONLY our O_RDWR, le port Série doit être placé en mode Réception
		} */

	return 0;

	}

}

static int serial_driver_release(struct inode *inode, struct file *flip){

	clear_bit(serial.not_available);  
	return 0;

}


static void serial_driver_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos){
	static int count = 0;
	struct serial_driver_struct * p = (struct serial_driver_struct *) filp->private_data;
	spin_lock(p->tp_spinlock);
	while(p->num_data <= 0){		//est ce que j'ai <<count>> donnees a lui donnee ?
		spin_unlock(p->tp_spinlock);
		if(flip->f_flags & O_NONBLOCK)
			return -EAGAIN;
		wait_even_interruptible(p->wait_rx, p->num_data >= 0);
		spin_lock(p->tp_spinlock);
	}
	count = (p->num_data >= count)? count : p->num_data;

	copy_to_user(buf, p->tp[id_out], count); //bloquant ... a retravailler

	p->id_out = (p->id_out + count) % MAXSIZE;
	p->num_data -= count;
	spin_unlock(p->tp_spinlock);

	return count;
}

/*

static void serial_driver_write(...){
	return;
}
*/



module_init(serial_driver_init);
module_exit(serial_driver_exit);


