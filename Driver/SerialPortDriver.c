#include <linux/kernel.h>
#include <linux/kthread.h>

#include <linux/fs.h>
#include <linux/types.h>
#include <linux/stat.h>

#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>

#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/interrupt.h>

#include <linux/ioport.h>

#include <linux/irq.h>

#include <asm/barrier.h>
#include <asm/io.h>

#include "circular.h"
#include "SerialComm.h"


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
irqreturn_t serial_driver_irq(int irq, void *dev_id);


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

static int Port0Addr = 0;
static int Port0IRQ = 0;
static int Port1Addr= 0;
static int Port1IRQ = 0;
module_param(Port0Addr, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);   
module_param(Port0IRQ , int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); 
module_param(Port1Addr, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); 
module_param(Port1IRQ , int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); 

MODULE_PARM_DESC(Port0Addr , "Serial Port 0 Address");
MODULE_PARM_DESC(Port1Addr , "Serial Port 1 Address");
MODULE_PARM_DESC(Port0IRQ  , "Serial Port 0 IRQ Number");
MODULE_PARM_DESC(Port1IRQ  , "Serial Port 1 IRQ Number");

struct serial_driver_struct{

	int size;
	struct cdev * cdev;

	struct class * dev_class;

	char non_blocking;

	uid_t active_user;
	//int active_mode;

	int PortAddr;
	int PortIRQ;

	int IRQStatus;

	bool mode_r, mode_w;

	wait_queue_head_t wait_rx, wait_tx;

	spinlock_t  tx_buf_lock,  rx_buf_lock; 
	circular * tx_buf, *rx_buf;

	dev_t dev; 

	serialcomm * comm;


};

unsigned long read_spin_flags;
unsigned long write_spin_flags;

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
	serial[0].PortAddr = Port0Addr;
	serial[0].PortIRQ = Port0IRQ;
	serial[1].PortAddr = Port1Addr;
	serial[1].PortIRQ = Port1IRQ;
				
	printk(KERN_WARNING"Starting \n");
	alloc_chrdev_region(&dev, 0, NUM_DEVICES, "SerialDriver");		//Allocation dynamique d`un numero d`unite-materiel 
	
	int i =0;
	for(i = 0; i < NUM_DEVICES; i++){
		
		printk(KERN_WARNING"PORT %d ADDR : %d \n", i, serial[i].PortAddr);
		printk(KERN_WARNING"PORT %d IRQ : %d \n", i,  serial[i].PortIRQ);
		

		snprintf(t_buf, 20, "Serial%d", i);
		if(request_region(serial[i].PortAddr, 8, t_buf) == NULL){
			return -EAGAIN;
		}

		serial[i].dev = MKDEV(MAJOR(dev), MINOR(dev)+i);

		serial[i].cdev = cdev_alloc();
		serial[i].cdev->ops = &serial_fops;

		snprintf(t_buf, 20, "Serial Driver %d", i);
		serial[i].dev_class = class_create(THIS_MODULE, t_buf);
		device_create(serial[i].dev_class, NULL, 
					  serial[i].dev, 
					  NULL, "SerialDev%d", i);
		
		cdev_init(serial[i].cdev, &serial_fops);

		cdev_add(serial[i].cdev, serial[i].dev, 1);

		serial[i].tx_buf = circular_init(10);
		serial[i].rx_buf = circular_init(10);

		printk(KERN_WARNING"Requesting IRQ (%d) \n", serial[i].PortIRQ);
		snprintf(t_buf, 20, "serial_irq_%d", i);
		serial[i].IRQStatus = -1;
		serial[i].IRQStatus = request_irq(serial[i].PortIRQ, serial_driver_irq, IRQF_SHARED, t_buf, &serial[i]);  // == IRQ_NONE){
		printk(KERN_WARNING"Request IRQ Status (%d) \n", serial[i].IRQStatus);

		init_waitqueue_head(&(serial[i].wait_rx));
		init_waitqueue_head(&(serial[i].wait_tx));

		spin_lock_init(&(serial[i].rx_buf_lock));
		spin_lock_init(&(serial[i].tx_buf_lock));
		


		printk(KERN_WARNING"Adding Serial Driver %d (err. %d)\n", i, err);

		/*unsigned char LCR = inb(serial[i].PortAddr + 3);
		rmb();
		printk(KERN_WARNING"LCR %d\n", LCR);
		outb(LCR | 0x80, serial[i].PortAddr + 3); //Set DLAB to 1
		wmb();
		LCR = inb(serial[i].PortAddr + 3);
		rmb();
		printk(KERN_WARNING"LCR After %d\n", LCR);


		unsigned char DLL = inb(serial[i].PortAddr + 0);
		rmb();
		printk(KERN_WARNING"Divisor %d\n", DLL);
		outb(15, serial[i].PortAddr + 0); //Set DLL to 15
		wmb();
		DLL = inb(serial[i].PortAddr + 0);
		rmb();
		printk(KERN_WARNING"Divisor After %d\n", DLL);
		*/

		//Set IER to Read and Write interruption
		//unsigned char IER = inb(serial[i].PortAddr + 1);
		//outb(IER | 0x03, serial[i].PortAddr + 3); 
		//wmb();

		/*	printk(KERN_WARNING"request IRQ not working (%d) \n", serial[i].PortIRQ);
			
			release_region(serial[i].PortAddr, 8);
			printk(KERN_WARNING"1 \n");

			device_destroy(serial[i].dev_class, serial[i].dev);
			printk(KERN_WARNING"2 \n");
			class_destroy(serial[i].dev_class);
			printk(KERN_WARNING"3\n");
			circular_destroy(serial[i].tx_buf);
			circular_destroy(serial[i].rx_buf);
			printk(KERN_WARNING"4 \n");

			serialcomm_deinit(serial[i].comm);
			printk(KERN_WARNING"5 \n");
			return 0;
		} */

		serial[i].comm = serialcomm_init(serial[i].PortAddr);
		serialcomm_set_baud(serial[i].comm, 9600);
		serialcomm_set_word_len(serial[i].comm, 8);
		serialcomm_set_parity(serial[i].comm, NO_PARITY);


		serialcomm_write_reg(serial[i].comm, IER, 0);

		//Set RCVR trigger level to 1 byte and enable FIFOEN
		//uint8_t val = serialcomm_read_reg(serial[i].comm, FCR);
		//printk(KERN_WARNING"Before FCR val %u\n", val);

		//Set RCVR trigger level at 1 Byte (bit[7:8] = 0b00)
		serialcomm_write_reg(serial[i].comm, FCR, 0x03);

		//val = serialcomm_read_reg(serial[i].comm, FCR);
		//rmb();

		//printk(KERN_WARNING"After FCR val %u\n", val);


		//serialcomm_set_bit(serial[i].comm, IER, 0); 	//Set ETBEI (enable read int.)
		//serialcomm_set_bit(serial[i].comm, IER, 0); 	//Set ERBFI (enable write int.)
		serialcomm_write_reg(serial[i].comm, SCR, 0);
		uint8_t test = serialcomm_read_reg(serial[i].comm, SCR);
		printk(KERN_WARNING"Before SCR %u\n", SCR);

		serialcomm_set_bit(serial[i].comm, SCR, 7);
		test = serialcomm_read_reg(serial[i].comm, SCR);
		printk(KERN_WARNING"After SCR %u\n", SCR);

		//Disable Parity
		serialcomm_set_bit(serial[i].comm, LCR, 3);
		// Parity even
		serialcomm_set_bit(serial[i].comm, LCR, 4);
		// 2 stop bit 
		serialcomm_set_bit(serial[i].comm, LCR, 2);
		//serialcomm_set_bit(serial[i].comm, MCR, 4);

	}

	//spin_lock_init(serial.tp_spinlock);


	return 0;

}

static void __exit serial_driver_exit (void) {


	printk(KERN_WARNING"Exiting\n");
	int i = 0;
	for(i = 0; i < NUM_DEVICES; i++){

		serialcomm_write_reg(serial[i].comm, IER, 0);


		if(!serial[i].IRQStatus){
			free_irq(serial[i].PortIRQ, &serial[i]);
		}

		release_region(serial[i].PortAddr, 8);

		device_destroy(serial[i].dev_class, serial[i].dev);
		class_destroy(serial[i].dev_class);

		circular_destroy(serial[i].tx_buf);
		circular_destroy(serial[i].rx_buf);

		serialcomm_deinit(serial[i].comm);

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



	if(serial[port_num].active_user == NULL)	
		serial[port_num].active_user = current_cred()->uid.val;
	
	
	/*"L'ouverture du Pilote est a usager unique. C'est-a-dire que dès
	   qu'un usager ouvre le Pilote dans n'importe quel mode, lecture ou
	   ecriture ou les deux, le Pilote lui appartient et tous nouvel usager sera
	   refuse avec un code d'erreur (ENOTTY)" */
	if(serial[port_num].active_user != current_cred()->uid.val)
		return -ENOTTY;


	/*"Toute nouvelle ouverture dans un 
	   mode deja ouvert sera refusee et un
	   code d'erreur (ENOTTY) sera retourne au demandeur."*/
	if((serial[port_num].mode_w & req_mode & O_WRONLY ) ||
	  (serial[port_num].mode_r & req_mode & O_RDONLY)) return -ENOTTY;	
	


	serial[port_num].mode_w |= (req_mode == O_WRONLY) || (req_mode == O_RDWR);
	serial[port_num].mode_r |= (req_mode == O_RDONLY) || (req_mode == O_RDWR);

	flip -> private_data = &serial[port_num];


	/*
	si le mode d'ouverture O_RDONLY ou O_RDWR est choisi par
	l'usager, alors le Port Serie doit être place en mode Reception (active
  	l'interruption de reception) afin de commencer a recevoir des donnees
   	immediatement. */
	if (serial[port_num].mode_r)
		serialcomm_set_bit(serial[port_num].comm, IER ,0);

	printk(KERN_WARNING"Opening Serial Driver %d ! Current mode_r = %d, mode_w= %d\n", port_num,serial[port_num].mode_r, serial[port_num].mode_w);
	printk(KERN_WARNING"SerialDriver Opened by %d\n", serial[port_num].active_user);
	

	return 0;

}

static int serial_driver_release(struct inode *inode, struct file *flip){
	
	int port_num = iminor(inode);

	int req_mode = flip->f_flags & O_ACCMODE; // requested mode
		
	serial[port_num].active_user = NULL;
	//serial[port_num].active_mode = NULL;
	serial[port_num].mode_w &= ~((req_mode == O_WRONLY) || (req_mode == O_RDWR));
	serial[port_num].mode_r &= ~((req_mode == O_RDONLY) || (req_mode == O_RDWR));

	//if(!serial[port_num].mode_w)
	//	serialcomm_rst_bit(serial[port_num].comm, IER, 1);
	if(!serial[port_num].mode_r)
		serialcomm_rst_bit(serial[port_num].comm, IER, 0);

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

	//printk(KERN_ALERT"Reading SerialDriver !\n");
	struct serial_driver_struct * p = (struct serial_driver_struct *) filp->private_data;
	//printk(KERN_WARNING"Data in rx_buf : %s\n",p->rx_buf->buffer);
	//printk(KERN_WARNING"Data requested : %d\n",count);
	printk(KERN_ALERT"Entering Reading %x\n", p->PortAddr);


	char * buf_r = (char*) kmalloc((count+1) * sizeof(char), GFP_KERNEL);	//tampon local
	buf_r[count] = '\0';

	int i = 0;

	spin_lock_irq(&p->rx_buf_lock );

waitForDataInBuf:	
	

	while(p->rx_buf->num_data == 0){
		spin_unlock_irq(&p->rx_buf_lock );

		/*
		O_NONBLOCK:
		S'il n'y a aucune donnee disponible, le Pilote 
		retourne immediatement en indiquant que (0) 
		données ont ete fournies a l'usager.
		*/
		if(filp->f_flags & O_NONBLOCK){
			return 0;
		}

		printk(KERN_WARNING"No data in read buffer, going to sleep (%d) \n", p->rx_buf->num_data );
		
		//serialcomm_set_bit(p->comm, IER, 0);
		if(p->rx_buf->num_data == 0)
			wait_event_interruptible(p->wait_rx, p->rx_buf->num_data > 0);
		
		spin_lock_irq(&p->rx_buf_lock );
	}
	
	printk(KERN_WARNING"New data in read buffer\n");

	while(i < count && p->rx_buf->num_data > 0){
		//printk(KERN_WARNING"buf size: %d\n",p->rx_buf->num_data);
		buf_r[i] = circular_remove(p->rx_buf);
		//circular_display(p->rx_buf);

		i++;
	} 		
	printk(KERN_WARNING"i %d, count %d, flags %x, block %x\n", i ,count, filp->f_flags, filp->f_flags & O_NONBLOCK );
	if((i < count) && !(filp->f_flags & O_NONBLOCK))
		goto waitForDataInBuf;

	//memcpy(data_buf, , 1);
	spin_unlock_irq(&p->rx_buf_lock);

	//printk(KERN_WARNING"Data to send : %s\n", buf_r);
	copy_to_user(buf, (void *)buf_r, count);
	
	circular_display(p->rx_buf);

	kfree(buf_r);
	printk(KERN_ALERT"Leaving Reading %x\n", p->PortAddr);

	return i; 
}


/*

returns:
	-ret val = count     if the requested # of bytes has been transfered
	-0 < ret val < count if only part of the data has been transfered
	-ret val = 0		 if nothing was written (not an error)
	-ret val < 0		 if an error has occurred (linux/errno.h)	
*/
static ssize_t  serial_driver_write(struct file * filp, const char __user *buf, size_t count,
							   loff_t *f_pos){

	//int port_num = iminor(inode);

	//printk(KERN_WARNING"Writing Serial Driver!\n");
	struct serial_driver_struct * p = (struct serial_driver_struct *) filp->private_data;
	//printk(KERN_WARNING"Data in tx_buf : %d\n",p->tx_buf->num_data);
	printk(KERN_ALERT"Entering Writing %x\n", p->PortAddr);
	char * buf_w = (char*) kmalloc(count * sizeof(char), GFP_KERNEL);	
	copy_from_user((void*)buf_w, buf, count);
	
	uint8_t val;
	

	

	int i = 0;

spin_lock_irq(&p->tx_buf_lock );

waitForRoomInBuf:
	
	val = serialcomm_read_reg(p->comm, IER);
	//printk(KERN_WARNING"Write , Before IER %u\n", val);


	serialcomm_set_bit(p->comm, IER, 1); 
	
	val = serialcomm_read_reg(p->comm, IER);
	//rintk(KERN_WARNING"Write , After IER %u\n", val);

	while(p->tx_buf->num_data == p->tx_buf->size){

		//serialcomm_set_bit(p->comm, IER, 1);   //Enable transmitter holding register empty 

		spin_unlock_irq(&p->tx_buf_lock );
		if(filp->f_flags & O_NONBLOCK)
			return 0;		//0 donnees ont ete tranferees.
		printk(KERN_WARNING"No room in tx buffer, going to sleep \n");
 
		wait_event_interruptible(p->wait_tx, p->tx_buf->num_data < p->tx_buf->size);
		spin_lock_irq(&p->tx_buf_lock );
	
	}

	printk(KERN_WARNING"Space available in tx buffer\n");

	char v;
	uint8_t ret = 0 ;
	while(i < count && p->tx_buf->num_data < p->tx_buf->size){
		//printk(KERN_WARNING"buf size: %d\n",p->tx_buf->num_data);
		circular_add(p->tx_buf, buf_w[i]);
		

		i++;
	} 
	
	if(i < count){ // && !(filp->f_flags & O_NONBLOCK))
		printk(KERN_WARNING"Write, going back to WAITFORROOMINBUF\n");
		goto waitForRoomInBuf;
	}

	//memcpy(data_buf, , 1);
	spin_unlock_irq(&p->tx_buf_lock );

	circular_display(p->tx_buf);
	kfree(buf_w);
	printk(KERN_ALERT"Leaving Writing %x\n", p->PortAddr);

	return i; 
}

irqreturn_t serial_driver_irq(int irq, void *dev_id){


	struct serial_driver_struct * p = (struct serial_driver_struct *) dev_id; //structure perso
	char data;
	printk(KERN_WARNING"Enter IRQ");

	uint8_t LSR_val = serialcomm_read_reg(p->comm, LSR);
	printk(KERN_WARNING"LSR VAL %u\n", LSR_val);
	uint8_t IER_val = serialcomm_read_reg(p->comm, IER);
	//printk(KERN_WARNING"IER VAL %u\n", IER_val);

	//If Transmitter empty  and is ready to send data
	if((LSR_val & 0x20) && (IER_val & 0x02) ){
		printk(KERN_ALERT"Entering IRQ (send). Called by Port %x\n", p->PortAddr);
		//printk(KERN_WARNING"IRQ ready to send\n");


		spin_lock(&p->tx_buf_lock);
		
		data = circular_remove(p->tx_buf);
		printk(KERN_WARNING"TX circ Buff Data %c\n", data);
		//circular_display(p->tx_buf);
		serialcomm_write_reg(p->comm, THR, data);
		
		//Si on a plus de donnees a envoyer, on a plus besoin de generer des interruptions TEMT
		if(p->tx_buf->num_data == 0){	
			printk(KERN_ALERT"NO DATA");
			serialcomm_rst_bit(p->comm, IER, 1);
			circular_display(p->tx_buf);
		}
		else{
			serialcomm_set_bit(p->comm, IER, 1);
		}

		IER_val = serialcomm_read_reg(p->comm, IER);
		printk(KERN_ALERT"IER VAL %u\n", IER_val);
		
		spin_unlock(&p->tx_buf_lock);
		wake_up_interruptible(&p->wait_tx);

		printk(KERN_ALERT"Leaving IRQ");
		return IRQ_HANDLED;
	}
	//If a data has arrived
	if((LSR_val & 0x01) && (IER_val & 0x01)){
		printk(KERN_ALERT"Entering IRQ (recieve). Called by Port %x\n", p->PortAddr);

		//printk(KERN_WARNING"IRQ ready to recieve\n");

		spin_lock(&p->rx_buf_lock);
		if(p->rx_buf->num_data < p->rx_buf->size){
			//si il y a de la place dans le tampon circulaire 
			data = serialcomm_read_reg(p->comm, RBR);

			printk(KERN_WARNING"RBR Buff Data %c\n", data);

			circular_add(p->rx_buf, data);
			//serialcomm_rst_bit(p->comm, FCR, 1); //FIFO de reception Reset<

		}
		//serialcomm_rst_bit(p->comm, IER, 0);
		spin_unlock(&(p->rx_buf_lock));
		wake_up_interruptible(&p->wait_rx);

		printk(KERN_ALERT"Leaving IRQ");
		return IRQ_HANDLED;

	}
	if(IER_val >= 0x4){
		serialcomm_rst_bit(p->comm, IER ,2);
		serialcomm_rst_bit(p->comm, IER ,3);
	}

	
	return IRQ_HANDLED;
}


module_init(serial_driver_init);
module_exit(serial_driver_exit);


