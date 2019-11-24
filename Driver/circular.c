#include "circular.h"
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


MODULE_LICENSE("Dual BSD/GPL");


circular * circular_init(int buffer_size){
	circular * c = kmalloc(sizeof(circular *), GFP_KERNEL);	
	c->buffer = (char*) kmalloc(buffer_size * sizeof(char), GFP_KERNEL);
	int i = 0;
	while(i<buffer_size){
		c->buffer[i] = NULL;
		i++;
	}
	c->head = 0;
	c->tail = 0;
	c->num_data = 0;
	c->size = buffer_size;
	//printf("Created buff\n");
	return c;
}

void circular_add(circular * c, char data){
	if(c->buffer[c->head] != NULL){
		printk(KERN_WARNING"Not adding to Circ. buff");
		return;
	}
	c->buffer[c->head] = data;
	if (c->num_data < c->size)
		c->num_data += 1;
	c->head = (c->head+1) % c->size;

}

void circular_add_n(circular * c, char * data, int n){
	int i = 0;
	while(i < n){
		circular_add(c, data[i]);
		i++;

	}

}


char  circular_remove(circular * c){
	char removed_data = c->buffer[c->tail];
	if(removed_data != NULL){
		//printk("Tail : %d (%c)", c->tail, c->buffer[c->tail]);
		c->buffer[c->tail] = NULL;
		c->tail = (c->tail+1) % c->size;
		c->num_data -= 1;
	}
	return removed_data;
}

void circular_remove_n(circular * c, char * removed_data, int n){
	//char * removed_data = (char*) kmalloc(n * sizeof(char), GFP_KERNEL); // = c->buffer[c->tail];
	int i = 0;
	while(i < n){
			removed_data[i] = circular_remove(c);
			i++;
		}
}
void circular_resize(circular * c, int size){
	circular * nc = circular_init(size);
	int num_data =  c->num_data;
	for(int i = 0; i < num_data; i++){
		circular_add(nc,circular_remove(c));
	}
	circular_destroy(c);
	* c = * nc;
}
void  circular_reset(circular * c){
	int i = 0;
	while(i < c->size){
		c->buffer[i] = NULL;
		i++;
	}
	c->head = 0;
	c->tail = 0;
	c->num_data = 0;
	return;
}


void circular_destroy(circular * c){
	kfree(c->buffer);
	kfree(c);
	return;
}

char b[10];

void circular_display(circular * c){
	//char * val;
	int i = 0;

	printk(KERN_WARNING"(head, %d et tail, %d (tail),", c->head, c->tail);

	while(i < c->num_data+1){
		//val = 
		if(c->buffer[i] == NULL){
			if(i == c->tail)
				printk(KERN_WARNING"(%d) NULL (tail),", i);
			else if(i == c->head)
				printk(KERN_WARNING"(%d) NULL (head),", i);
			else if(i == c->head && i == c->tail)
				printk(KERN_WARNING"(%d) NULL (head), (tail)", i);
			else 
				printk(KERN_WARNING"(%d) NULL", i);
			//snprintf(b, 5, "NULL");
		}
		else{
			if(i == c->tail)
				printk(KERN_WARNING"(%d) %c (tail),", i, c->buffer[i]);
			else if(i == c->head)
				printk(KERN_WARNING"(%d) %c (head),", i, c->buffer[i]);
			else if(i == c->head && i == c->tail)
				printk(KERN_WARNING"(%d) %c (head), (tail)", i);
			else 
				printk(KERN_WARNING"(%d) %c,", i, c->buffer[i]);
		}

		i++;
	} 
	//printk(KERN_WARNING"head: %d, tail: %d\n", c->head, c->tail);
	return;
} 
