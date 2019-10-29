#include <stdio.h>
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
typedef struct circular_buffer_struct{

	void ** buffer;
	int head;
	int tail;
	int size;
	int num_data

}circular;

circular * circular_init(int buffer_size){
	circular * c = malloc(sizeof(circular *));	
	c->buffer = (void**) calloc(buffer_size, sizeof(void *));
	for(int i = 0; i < buffer_size; i++)
		c->buffer[i] = NULL;
	c->head = 0;
	c->tail = 0;
	c->num_data = 0;
	c->size = buffer_size;
	printf("Created buff\n");
	return c;
}

void circular_add(circular * c, void * data){

	if(c->head == c->tail && c->buffer[c->tail] != NULL)
		return;
	c->buffer[c->head] = data;
	if (c->num_data < c->size)
		c->num_data += 1;
	c->head = (c->head+1) % c->size;

}

void * circular_remove(circular * c){
	void * removed_data = c->buffer[c->tail];
	c->buffer[c->tail] = NULL;
	c->tail = (c->tail+1) % c->size;
	return removed_data;
}


void circular_display(circular * c){
	void * val;

	for(int i = 0; i<c->num_data; i++){
		val = c->buffer[i];
		if(val == NULL)
			printf(" , ");
		else
			printf("%c, ", *(char *)val);
	} 
	printf("\n");
	return;
}

void main(){


	circular * c = circular_init(6);
	char string[] = "HelloWorld";
	circular_add(c, &string[0]);	circular_display(c);
	circular_add(c, &string[1]);	circular_display(c);
	circular_remove(c);	circular_display(c);
	circular_add(c, &string[2]);	circular_display(c);
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




}