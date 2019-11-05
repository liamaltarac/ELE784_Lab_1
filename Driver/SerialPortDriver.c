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
#include <linux/spinlock.h>

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

	uid_t active_user;
	//int active_mode;

	bool mode_r, mode_w;

	wait_queue_head_t wait_rx, wait_tx;

	spinlock_t * tx_buf_lock, * rx_buf_lock; 
	circular * tx_buf, *rx_buf;

	dev_t dev; 


};

struct serial_driver_struct serial[NUM_DEVICES] = {{.active_user = NULL,
													.mode_r = 0,
													.mode_w = 0
												   },
												   {.active_user = NULL,													
												   	.mode_r 	 = 0,
													.mode_w 	 = 0
												   }};

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

		cdev_add(serial[i].cdev, serial[i].dev, 1);

		serial[i].tx_buf = circular_init(10);
		serial[i].rx_buf = circular_init(10);

		init_waitqueue_head(&serial[i].wait_rx);
		init_waitqueue_head(&serial[i].wait_tx);

		spin_lock_init(serial[i].rx_buf_lock);
		spin_lock_init(serial[i].tx_buf_lock);

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

		circular_destroy(serial[i].tx_buf);
		circular_destroy(serial[i].rx_buf);
		printk(KERN_WARNING"Removing Serial Driver %d: Goodbye !\n", i);

	}

	unregister_chrdev_region(dev, NUM_DEVICES);


	return;

}


/* 
"
l'ouverture du Pilote est a usager unique. C'est-a-dire que dès
qu'un usager ouvre le Pilote dans n'importe quel mode, lecture ou
ecriture ou les deux, le Pilote lui appartient et tous nouvel usager sera
refuse avec un code d'erreur (ENOTTY). "

-First operation performed on device file.
-Should perform:
	-Check for device specific errors (ex.: device not ready, harware probs etc.)
	-Initialize device if being opened for first time
	-Update f_op pointer, if necessary
	-Allocate and fill any data struct to be put in flip->private_data
*/
static int serial_driver_open(struct inode *inode, struct file *flip){
	
	int port_num = iminor(inode);
	int req_mode = flip->f_flags & O_ACCMODE; // requested mode

	/*"Toute nouvelle ouverture dans un 
	   mode deja ouvert sera refusee et un
	   code d'erreur (ENOTTY) sera retourne au demandeur."*/
	if((serial[port_num].mode_w & req_mode & O_WRONLY ) ||
	  (serial[port_num].mode_r & req_mode & O_RDONLY)) return -ENOTTY;	
	

	if(serial[port_num].active_user == NULL)	
		serial[port_num].active_user = current_cred()->uid.val;
	
	
	/*"L'ouverture du Pilote est a usager unique. C'est-a-dire que dès
	   qu'un usager ouvre le Pilote dans n'importe quel mode, lecture ou
	   ecriture ou les deux, le Pilote lui appartient et tous nouvel usager sera
	   refuse avec un code d'erreur (ENOTTY)" */
	if(serial[port_num].active_user != current_cred()->uid.val)
		return -ENOTTY;

	serial[port_num].mode_w |= (req_mode == O_WRONLY) || (req_mode == O_RDWR);
	serial[port_num].mode_r |= (req_mode == O_RDONLY) || (req_mode == O_RDWR);

	flip -> private_data = &serial[port_num];

	/*TODO : si le mode d'ouverture O_RDONLY ou O_RDWR est choisi par
			 l'usager, alors le Port Serie doit être place en mode Reception (active
  			 l'interruption de reception) afin de commencer a recevoir des donnees
   			 immediatement. */

	printk(KERN_WARNING"Opening Serial Driver %d ! Current mode_r = %d, mode_w= %d\n", port_num,serial[port_num].mode_r, serial[port_num].mode_w);
	printk(KERN_WARNING"SerialDriver Opened by %d", serial[port_num].active_user);
	return 0;

}

static int serial_driver_release(struct inode *inode, struct file *flip){
	
	int port_num = iminor(inode);

	int req_mode = flip->f_flags & O_ACCMODE; // requested mode

	
	serial[port_num].active_user = NULL;
	//serial[port_num].active_mode = NULL;
	serial[port_num].mode_w &= ~((req_mode == O_WRONLY) || (req_mode == O_RDWR));
	serial[port_num].mode_r &= ~((req_mode == O_RDONLY) || (req_mode == O_RDWR));

	printk(KERN_WARNING"Releasing Serial Driver %d! Current mode_r = %d, mode_w= %d (%d)\n", port_num,serial[port_num].mode_r, serial[port_num].mode_w, req_mode);

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
	printk(KERN_WARNING"Data in rx_buf : %s\n",p->rx_buf->buffer);
	printk(KERN_WARNING"Data requested : %d\n",count);


	char * buf_r = (char*) kmalloc(count * sizeof(char), GFP_KERNEL);	//tampon local

	spin_lock_irq(p->rx_buf_lock);

	int i = 0;

waitForDataInBuf:	
	while(p->rx_buf->num_data == 0){
		spin_unlock_irq(p->rx_buf_lock);

		/*
		O_NONBLOCK:
		S'il n'y a aucune donnee disponible, le Pilote 
		retourne immediatement en indiquant que (0) 
		données ont ete fournies a l'usager.
		*/
		if(filp->f_flags & O_NONBLOCK) {
			return 0;
		}

		printk(KERN_WARNING"No data in read buffer, going to sleep \n");
		wait_event_interruptible(p->wait_rx, p->rx_buf->num_data > 0);
		spin_lock_irq(p->rx_buf_lock);
	}
	
	printk(KERN_WARNING"New data in read buffer, Waking up\n");

	while(i < count && p->rx_buf->num_data > 0){
		printk(KERN_WARNING"buf size: %d\n",p->rx_buf->num_data);
		buf_r[i] = circular_remove(p->rx_buf);
		i++;
	} 

	if((i < count) && !(filp->f_flags & O_NONBLOCK))
		goto waitForDataInBuf;

	//memcpy(data_buf, , 1);
	spin_unlock_irq(p->rx_buf_lock);

	printk(KERN_WARNING"Data to send  : %s\n", buf_r);
	copy_to_user(buf, (void *)buf_r, count);

	kfree(buf_r);

	return i; 
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
	printk(KERN_WARNING"Data in tx_buf : %d\n",p->tx_buf->num_data);

	char * buf_w = (char*) kmalloc(count * sizeof(char), GFP_KERNEL);	
	copy_from_user((void*)buf_w, buf, count);

	spin_lock_irq(p->tx_buf_lock);
	int i = 0;
waitForRoomInBuf:
	while(p->tx_buf->num_data == p->tx_buf->size){
		spin_unlock_irq(p->tx_buf_lock);
		if(filp->f_flags & O_NONBLOCK)
			return 0;		//0 donnees ont ete tranferees.
		printk(KERN_WARNING"No room in tx buffer, going to sleep \n");
		wait_event_interruptible(p->wait_tx, p->tx_buf->num_data < p->tx_buf->size);
		spin_lock_irq(p->tx_buf_lock);
	}

	printk(KERN_WARNING"Space available in tx buffer, Waking up\n");

	while(i < count && p->tx_buf->num_data < p->tx_buf->size){
		printk(KERN_WARNING"buf size: %d\n",p->tx_buf->num_data);
		circular_add(p->tx_buf, buf_w[i]);
		i++;
	} 

	if((i < count) && !(filp->f_flags & O_NONBLOCK))
		goto waitForRoomInBuf;

	//memcpy(data_buf, , 1);
	spin_unlock_irq(p->tx_buf_lock);
	
	//circular_display(p->c_buf);
	kfree(buf_w);
	return i; 
}



module_init(serial_driver_init);
module_exit(serial_driver_exit);


