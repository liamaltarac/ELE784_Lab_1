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
	circular * c = kmalloc(sizeof(circular), GFP_KERNEL);	
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


void circular_resize(circular * c, int new_size){

	
	if(new_size < c->num_data){		//Si il nỳ a pas assez de place dans le nouveau tampon, on retour.
		return;
	}

	char * t_buffer = (char*) kmalloc(new_size * sizeof(char), GFP_KERNEL);

	int count = 0;
	int j=0;
	int index=c->tail;
	int i = 0;
	while(i < c->num_data){
		if(c->buffer[index] != NULL){
			t_buffer[i] = c->buffer[index];
			i++;
		}
		index = (index+1) % c->size ;	
	}
	c->head = 0;
	c->tail = i;
	c->size = new_size;
	kfree(c->buffer);	
	c->buffer = t_buffer;

}


void circular_destroy(circular * c){
	kfree(c->buffer);
	kfree(c);
	return;
}

char b[10];

/*void circular_display(circular * c){
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
}  */



void circular_display(circular * c){
	void * val;
	int i = 0;
	for(i = 0; i<c->size; i++){
		val = c->buffer[i];
		if(val == NULL)
			printk(KERN_CONT" , ");
		else
			printk(KERN_CONT"%d, ", val);
	} 
	printk(KERN_CONT" head: %d, tail: %d\n", c->head, c->tail);
	printk(KERN_CONT"\n");
	return;
}


