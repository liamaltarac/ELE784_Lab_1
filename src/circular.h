
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef struct circular_buffer_struct{

	char * buffer;
	int head;
	int tail;
	int size;
	int num_data

}circular;

circular * circular_init(int buffer_size);

void   circular_add(circular * c, char data);
void   circular_add_n(circular * c, char * data, int n);

char   circular_remove(circular * c);
void   circular_remove_n(circular * c, char * removed_data, int n);

void   circular_reset(circular * c);
void   circular_destroy(circular * c);
void circular_display(circular * c);
void circular_resize(circular * c, int new_size);
