#include "SerialComm.h"
#include <linux/kernel.h>
#include <linux/kthread.h>

#include <linux/fs.h>

#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>

#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/uaccess.h>

#define DEFAULT_BAUD   	    9600
#define DEFAULT_PARITY  	NO_PARITY
#define DEFAULT_DATA_SIZE   8

#define FCLK 	1843200		//Hz

MODULE_LICENSE("Dual BSD/GPL");


serialcomm * serialcomm_init(int base_addr){
	serialcomm * s = kmalloc(sizeof(serialcomm *), GFP_KERNEL);	


	s->current_dlab = -1;
	s->base_addr = base_addr;
	return s;
}

//returns baud rate that was set
int serialcomm_set_baud(serialcomm * s, int baud_rate){
	uint16_t div = FCLK / (16 * baud_rate);
	serialcomm_write_reg(s, DLM, div > 8);		//set dlm to upper 8 bits of div
	serialcomm_write_reg(s, DLL, div & 0x0F);	//set dlm to lower 8 bits of div 
	return  FCLK / (16 * div);

}
void serialcomm_set_word_len(serialcomm *s, int word_len){

	if(word_len >= 5 && word_len <= 8){
		uint8_t val = serialcomm_read_reg(s, LCR);
		serialcomm_write_reg(s, LCR, (val & ~0x02) | (word_len - 5));
	}
	return;
}


void serialcomm_set_parity(serialcomm * s, int parity){

	if(parity == NO_PARITY){
		serialcomm_rst_bit(s, LCR, 3);
		return;
	}	

	serialcomm_set_bit(s, LCR, 3);
	serialcomm_write_bit(s, LCR, parity, 4);

}


void serialcomm_write_reg(serialcomm * s, uint8_t adresse, int dlab, uint8_t access, uint8_t val){

	uint8_t ret;

	if(!(access & W)){
		printk(KERN_WARNING"Cant write this reg\n");
		return;	//Cant write this register
	}

	//If current dlab already is set properly or dlab is "dont care", 
	//then we dont need to change it.
	//If current dlab is not properly set, and dlab isnt "dont care",
	//then we need to toggle the dblab bit.
	if(s->current_dlab != dlab && dlab != -1){
		s->current_dlab = dlab;	
		ret = inb(s->base_addr + LCR_ADDR);
		rmb();
		outb((ret & ~(1 << 7)) | (dlab << 7), s->base_addr + LCR_ADDR); //toggle DLAB to bit
	}
	outb(val, s->base_addr + adresse);
	wmb();

	return;
}

uint8_t serialcomm_read_reg(serialcomm * s, uint8_t adresse, int dlab, uint8_t access){
	uint8_t ret;

	if(!(access & R))
		return;	//Cant read this register

	if(s->current_dlab != dlab && dlab != -1){
		ret = inb(s->base_addr + LCR_ADDR);
		rmb();
		outb((ret & ~(1 << 7)) | (dlab << 7), s->base_addr + LCR_ADDR); //toggle DLAB to 1
	}
	printk(KERN_WARNING"Base ADDR : %d ", s->base_addr);
	ret = inb(s->base_addr + adresse);
	rmb();

	return ret;
}


void serialcomm_write_bit(serialcomm * s, uint8_t adresse, int dlab, uint8_t access, uint8_t val, uint8_t bit){
	uint8_t ret;

	if(!(access & W))
		return;	//Cant write this register

	//If current dlab already is set properly or dlab is "dont care", 
	//then we dont need to change it.
	//If current dlab is not properly set, and dlab isnt "dont care",
	//then we need to toggle the dblab bit.
	if(s->current_dlab != dlab && dlab != -1){
		s->current_dlab = dlab;	
		ret = inb(s->base_addr + LCR_ADDR);
		rmb();
		outb((ret & ~(1 << 7)) | (dlab << 7), s->base_addr + LCR_ADDR); //toggle DLAB to bit
	}
	ret = inb(s->base_addr + adresse); 
	rmb();
	outb((ret & ~(1 << bit)) | (val << bit) , s->base_addr + adresse);	//set bit
	wmb();

	return;
}


void serialcomm_set_bit(serialcomm * s, uint8_t adresse, int dlab, uint8_t access, uint8_t bit){
	uint8_t ret = 0 ;

	if(!(access == RW)){
		printk(KERN_WARNING"Cant do set %u, %u, %u\n", RW, access, access & RW);
		return;	//Cant write this register
	}

	//If current dlab already is set properly or dlab is "dont care", 
	//then we dont need to change it.
	//If current dlab is not properly set, and dlab isnt "dont care",
	//then we need to toggle the dblab bit.
	if(s->current_dlab != dlab && dlab != -1){
		s->current_dlab = dlab;	
		ret = inb(s->base_addr + LCR_ADDR);
		rmb();
		outb((ret & ~(1 << 7)) | (dlab << 7), s->base_addr + LCR_ADDR); //toggle DLAB to bit
		wmb();
	}
	ret = inb(s->base_addr + adresse);
	rmb();
	outb(ret | (1 << bit) , s->base_addr + adresse);	//set bit
	wmb();

	return;
}

void serialcomm_rst_bit(serialcomm * s, uint8_t adresse, int dlab, uint8_t access, uint8_t bit){
	uint8_t ret = 0;

	if(!(access == RW))
		return;	//Cant write this register

	//If current dlab already is set properly or dlab is "dont care", 
	//then we dont need to change it.
	//If current dlab is not properly set, and dlab isnt "dont care",
	//then we need to toggle the dblab bit.
	if(s->current_dlab != dlab && dlab != -1){
		s->current_dlab = dlab;	
		ret = inb(s->base_addr + LCR_ADDR);
		rmb();
		outb((ret & ~(1 << 7)) | (dlab << 7), s->base_addr + LCR_ADDR); //toggle DLAB to bit
		wmb();
	}
	ret = inb(s->base_addr + adresse);
	rmb();
//printk(KERN_WARNING"RET val %u\n", ret) ;
//	printk(KERN_WARNING"TEST val %u\n", (ret & ~(1 << bit))) ;

	outb(ret & ~(1 << bit) , s->base_addr + adresse);	//clear bit
	wmb();

	return;
}

uint8_t serialcomm_read_bit(serialcomm * s, uint8_t adresse, int dlab, uint8_t access, uint8_t bit){
	uint8_t ret;

	if(!(access & R))
		return;	//Cant read this register

	if(s->current_dlab != dlab && dlab != -1){
		ret = inb(s->base_addr + LCR_ADDR);
		rmb();
		outb((ret & ~(1 << 7)) | (dlab << 7), s->base_addr + LCR_ADDR); //toggle DLAB to 1
	}
	ret = inb( s->base_addr + adresse);
	rmb();

	return (ret >> (bit+1)) & (0x01);
}


void serialcomm_deinit(serialcomm * s){

	kfree(s);

}
