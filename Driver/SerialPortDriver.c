#include <linux/kernel.h>
#include <linux/kthread.h>

#include <linux/fs.h>
#include <linux/types.h>

#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>

#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/uaccess.h>
#include "circular.h"



//#include "circular.h"

//#include <asm_generic/bitops.h>

#define MAXSIZE 255
#define NUM_DEVICES 2  //SerialPort0 et SerialPort1

MODULE_AUTHOR("Liam et Maxime");
MODULE_LICENSE("Dual BSD/GPL");

static int serial_driver_open(struct inode *inode, struct file *flip);
static int serial_driver_release(struct inode *inode, struct file *flip);
static ssize_t serial_driver_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t serial_driver_write(struct file * filp, const char __user *buf, size_t count,
							   loff_t *f_pos);



struct file_operations 	serial_fops = {
	.owner   = THIS_MODULE,
	.open    = serial_driver_open,
	.release = serial_driver_release,
	.read 	 = serial_driver_read,
	.write 	 = serial_driver_write,
	}; 




spinlock_t * cu_spinlock;

uint8_t id_in, id_out = 0; 	//Premiere et derniere indice du tampon circulaire
uint8_t num_data = 0;
char buffer[MAXSIZE];


//kuid_t current_user = NULL;

dev_t dev;

int err;


struct serial_driver_struct{

	int size;
	struct cdev * cdev;

	struct class * dev_class;

	char non_blocking;

	kuid_t active_user;
	int active_mode;


	wait_queue_head_t wait_rx;

	spinlock_t * tp_spinlock; 
	circular * c_buf;

	dev_t dev; 


};

struct serial_driver_struct serial[NUM_DEVICES] = {{.active_mode = NULL,},
												   {.active_mode = NULL,}};

char t_buf[20];
static int __init serial_driver_init (void) {

	//TODO : Verifier si on peut prendre controlle des ports (premiere chose a faire)

	alloc_chrdev_region(&dev, 0, NUM_DEVICES, "SerialDriver");		//Allocation dynamique d`un numero d`unite-materiel 
	
	int i =0;
	for(i = 0; i < NUM_DEVICES; i++){

		serial[i].dev = MKDEV(MAJOR(dev), MINOR(dev)+i);

		serial[i].cdev = cdev_alloc();
		serial[i].cdev->ops = &serial_fops;

		snprintf(t_buf, 20, "Serial Driver %d", i);
		serial[i].dev_class = class_create(THIS_MODULE, t_buf);
		device_create(serial[i].dev_class, NULL, 
					  serial[i].dev, 
					  NULL, "My_Serial_Driver_%d", i);
		
		cdev_init(serial[i].cdev, &serial_fops);

		err = cdev_add(serial[i].cdev, serial[i].dev, 1);

		serial[i].c_buf = circular_init(10);

		printk(KERN_WARNING"Adding Serial Driver %d (err. %d)\n", i, err);

	}

	//spin_lock_init(serial.tp_spinlock);


	return 0;

}

static void __exit serial_driver_exit (void) {
	printk(KERN_WARNING"Exiting\n");
	int i = 0;
	for(i = 0; i < NUM_DEVICES; i++){

		device_destroy(serial[i].dev_class, serial[i].dev);
		class_destroy(serial[i].dev_class);

		circular_destroy(serial[i].c_buf);
		printk(KERN_WARNING"Removing Serial Driver %d: Goodbye !\n", i);

	}

	unregister_chrdev_region(dev, NUM_DEVICES);


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
	
	int port_num = iminor(inode);

	if(serial[port_num].active_mode == NULL){	//If file not yet opened

		serial[port_num].active_user = current_cred()->uid;
		serial[port_num].active_mode = flip->f_flags & O_ACCMODE;
		flip -> private_data = &serial[port_num];
		printk(KERN_WARNING"Opening Serial Driver %d in mode %d\n",port_num, serial[port_num].active_mode );
		printk(KERN_WARNING"SerialDriver Opened by %d", serial[port_num].active_user);
		return 0;

	}  
	

	return -ENOTTY;
	/*
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
	
	int port_num = iminor(inode);


	printk(KERN_WARNING"Releasing Serial Driver %d!\n", port_num);
	
	//serial.active_user = NULL;
	serial[port_num].active_mode = NULL;

	//clear_bit(serial.not_available);  
	return 0;

}


/*
returns:
	- ret val = count argument	if the requested number of bytes has been transferred. 
								This is the optimal case.

	- 0 < ret val < count		if only part of the data has been transferred. 

	- ret val = 0				if end-of-file was reached (and no data was read).

	-ret val < 0				if an error has occurred (linux/errno.h)	
*/
static ssize_t serial_driver_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos){
	
	//int port_num = iminor(inode);

	printk(KERN_WARNING"Reading SerialDriver !\n");
	struct serial_driver_struct * p = (struct serial_driver_struct *) filp->private_data;
	printk(KERN_WARNING"Data in buf : %s\n",p->c_buf->buffer);
	printk(KERN_WARNING"Data requested : %d\n",count);


	char * data_buf = (char*) kmalloc(count * sizeof(char), GFP_KERNEL);	
	circular_remove_n(p->c_buf, data_buf, count);
	//memcpy(data_buf, , 1);
	printk(KERN_WARNING"Data to send  : %c\n", data_buf);

	copy_to_user(buf, (void *)data_buf, count);

	
	//circular_display(p->c_buf);

	//circular_remove_n(p->buf)
	//static int count = 0;
	/*
	struct serial_driver_struct * p = (struct serial_driver_struct *) filp->private_data;
	spin_lock(p->tp_spinlock);
		printk(KERN_WARNING"Started spinlock SerialDriver !\n");

	while(p->c_buf->num_data <= 0){		//est ce que j'ai <<count>> donnees a lui donnee ?
		printk(KERN_WARNING"NO DATA !!\n");
		spin_unlock(p->tp_spinlock);
		if(filp->f_flags & O_NONBLOCK){
			printk(KERN_WARNING"ERROR IN READING !!\n");
			return -EAGAIN;
		}
		wait_event_interruptible(p->wait_rx, p->c_buf->num_data >= 0);
		spin_lock(p->tp_spinlock);
	}
	count = (p->c_buf->num_data >= count)? count : p->c_buf->num_data;

	copy_to_user(buf, circular_remove_n(p->c_buf, count), count); //bloquant ... a retravailler

	spin_unlock(p->tp_spinlock);  */
	

	return count; 
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

	//int port_num = iminor(inode);

	printk(KERN_WARNING"Writing Serial Driver!\n");
	struct serial_driver_struct * p = (struct serial_driver_struct *) filp->private_data;
	printk(KERN_WARNING"Data in buf : %d\n",p->c_buf->num_data);

	char * user_buf = (char*) kmalloc(count * sizeof(char), GFP_KERNEL);	
	copy_from_user((void*)user_buf, buf, count);
	printk(KERN_WARNING"Data  : %s\n", user_buf);

	int i = 0;
	circular_add_n(p->c_buf, user_buf, count);
	
	//circular_display(p->c_buf);
	//kfree(user_buf);
	return count; 
}



module_init(serial_driver_init);
module_exit(serial_driver_exit);


