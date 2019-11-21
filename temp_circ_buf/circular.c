#include "circular.h"


circular * circular_init(int buffer_size){
	circular * c = malloc(sizeof(circular *));	
	c->buffer = (char*) malloc(buffer_size * sizeof(char));
	for(int i = 0; i < buffer_size; i++)
		c->buffer[i] = NULL;
	c->head = 0;
	c->tail = 0;
	c->num_data = 0;
	c->size = buffer_size;
	printf("Created buff\n");
	return c;
}

void circular_add(circular * c, char data){
	if(c->buffer[c->head] != NULL){
		printf("here\n");
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


		if(c->buffer[c->head] == NULL){
			c->buffer[c->head] = data[i];
			if (c->num_data < c->size)
				c->num_data += 1;
		
			c->head = (c->head+1) % c->size;
		}
		
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

char * circular_remove_n(circular * c, int n){
	char * removed_data = (char*) malloc(n * sizeof(char));; // = c->buffer[c->tail];
	int i = c->head;
	while(i < n){
		if (c->buffer[c->tail] != NULL){
			removed_data[i] = c->buffer[c->tail] ;
			c->buffer[c->tail] = NULL;
			i++;
		}
		c->tail = (c->tail+1) % c->size;
	}	
	return removed_data;

}

void  circular_reset(circular * c){
	for(int i = 0; i < c->size; i++)
		c->buffer[i] = NULL;
	c->head = 0;
	c->tail = 0;
	return;
}


void delete_list(circular * c){
	for(int i = 0; i < c->size; i++)
		free(c->buffer[i]);
	free(c->buffer);
	free(c);
	return;
}


void circular_display(circular * c){
	void * val;

	for(int i = 0; i<c->size; i++){
		val = c->buffer[i];
		if(val == NULL)
			printf(" , ");
		else
			printf("%c, ", val);
	} 
	printf("head: %d, tail: %d", c->head, c->tail);
	printf("\n");
	return;
}

void main(){


	circular * c = circular_init(6);
	char string[] = "HelloWorld";
	circular_add(c, 'a');	circular_display(c);
	circular_add(c, 'b');	circular_display(c);
	circular_add(c, 'a');	circular_display(c);
	circular_add(c, 'b');	circular_display(c);
	circular_add(c, 'a');	circular_display(c);
	circular_add(c, 'b');	circular_display(c);
	circular_add(c, '9');	circular_display(c);
	circular_add(c, '8');	circular_display(c);
	circular_add(c, '7');	circular_display(c);
	circular_add(c, '6');	circular_display(c);
	circular_add(c, '5');	circular_display(c);
	circular_add(c, '4');	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, 'c');	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, 'd');	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, 'e');	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, 'f');	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, 'g');	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, 'h');	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, 'i');	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, 'j');	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, 'k');	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, 'l');	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, 'm');	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, 'n');	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, 'o');	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_remove(c);	circular_display(c);
	/*circular_add(c, &string[2]);	circular_display(c);
	circular_add(c, &string[3]);	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, &string[4]);	circular_display(c);
	circular_add(c, &string[5]);	circular_display(c);
	circular_add(c, &string[6]);	circular_display(c);
	circular_add(c, &string[7]);	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, &string[8]);	circular_display(c);
	circular_add(c, &string[9]);	circular_display(c);
	circular_add(c, &string[10]);	circular_display(c);
	//circular_add(c, &string[1]);	circular_display(c);

*/


} 
