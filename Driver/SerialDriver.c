#include <linux/kernel.h>
#include <linux/kthread.h>

#include <linux/fs.h>
#include <linux/types.h>

#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>

#include <linux/module.h>
#include <linux/moduleparam.h>

//#include <asm_generic/bitops.h>

#define MAXSIZE 255

MODULE_AUTHOR("Liam");
MODULE_LICENSE("Dual BSD/GPL");


static int serial_driver_open(struct inode *inode, struct file *flip);
static int serial_driver_release(struct inode *inode, struct file *flip);
static ssize_t serial_driver_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t serial_driver_write(struct file * filp, const char __user *buf, size_t count,
							   loff_t *f_pos);

struct class * dev_class;

struct file_operations 	serial_fops = {
	.owner   = THIS_MODULE,
	.open    = serial_driver_open,
	.release = serial_driver_release,
	.read 	 = serial_driver_read,
	.write 	 = serial_driver_write,
	}; ;

struct serial_driver_struct{

	int size;
	struct cdev * cdev;

	char non_blocking;

	kuid_t user_id;

}serial;



spinlock_t * tp_spinlock;
spinlock_t * cu_spinlock;

uint8_t id_in, id_out = 0; 	//Premiere et derniere indice du tampon circulaire
uint8_t num_data = 0;
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
	device_create(dev_class, NULL, dev, NULL, "My_Serial_Driver");
	
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


/*  
-First operation performed on device file.
-Should perform:
	-Check for device specific errors (ex.: device not ready, harware probs etc.)
	-Initialize device if being opened for first time
	-Update f_op pointer, if necessary
	-Allocate and fill any data struct to be put in flip->private_data
*/
static int serial_driver_open(struct inode *inode, struct file *flip){
	

	printk(KERN_WARNING"Opening SerialDriver!\n");
	return 0;


	/*static kuid current_user = inode->i_uid;
	static int count = 0;
	struct serial_dev *dev;  // cette structure  contient ... ?
	


	if(inode->i_uid == current_user){
		count++;
		filp->private_data = &serial;
		//Identifie l'unite-materiel
		//Initilisation personnelle

		//if(flip->f_flags & O_ACCMODE){
			//Si flip->f_flags est en mode O_RDONLY our O_RDWR, le port Série doit être placé en mode Réception
		//} 
		return 0;

	}
	*/

}

static int serial_driver_release(struct inode *inode, struct file *flip){
	
	printk(KERN_WARNING"Releasing SerialDriver !\n");
	//clear_bit(serial.not_available);  
	return 0;

}


static ssize_t serial_driver_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos){
	printk(KERN_WARNING"Reading SerialDriver !\n");

	/*static int count = 0;
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
	*/

	return 0;
}


/*


returns:
	-ret val = count     if the requested # of bytes has been transfered
	-0 < ret val < count if only part of the data has been transfered
	-ret val = 0		 if nothing was written (not an error)
	-ret val < 0		 if an error has occurred (linux/errno.h)	
*/
static ssize_t serial_driver_write(struct file * filp, const char __user *buf, size_t count,
							   loff_t *f_pos){

	printk(KERN_WARNING"Writting in SerialDriver!\n");
	
	return count; 
}


module_init(serial_driver_init);
module_exit(serial_driver_exit);


