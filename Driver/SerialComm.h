
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define NO_PARITY   -1
#define ODD_PARITY  0
#define EVEN_PARITY 1



#define R  1
#define W  2
#define RW R|W

#define RBR_ADDR 0x00
#define THR_ADDR 0x00
#define DLL_ADDR 0x00
#define DLM_ADDR 0x01
#define IER_ADDR 0x01
#define IIR_ADDR 0x02
#define FCR_ADDR 0x02
#define LCR_ADDR 0x03
#define MCR_ADDR 0x04
#define LSR_ADDR 0x05
#define MSR_ADDR 0x06
#define SCR_ADDR 0x07


//Reg_name   address,dlab,permission(r/w)
#define RBR RBR_ADDR,  0, R
#define THR THR_ADDR,  0 ,W
#define DLL DLL_ADDR,  1 ,RW
#define DLM DLM_ADDR,  1 ,RW
#define IER IER_ADDR,  0 ,RW
#define IIR IIR_ADDR, -1 ,R
#define FCR FCR_ADDR, -1 ,W
#define LCR LCR_ADDR, -1 ,RW
#define MCR MCR_ADDR, -1 ,RW
#define LSR LSR_ADDR, -1 ,R
#define MSR MSR_ADDR, -1 ,R
#define SCR SCR_ADDR, -1 ,RW

#include <linux/types.h>


typedef struct serialcomm_struct{

	int base_addr;
	int baud_rate;
	int data_size;
	int parity;
	int buf_size;
	int current_dlab;	//keep track of dlab to avoid setting/reset it as much as possible

}serialcomm;

serialcomm * serialcomm_init(int base_addr);

int serialcomm_set_baud(serialcomm * s, int baud_rate);

void serialcomm_set_word_len(serialcomm *s, int word_len);

void serialcomm_set_parity(serialcomm * s, int parity);

void serialcomm_write_reg(serialcomm * s, uint8_t adresse, int dlab, uint8_t access, uint8_t val);

uint8_t serialcomm_read_reg(serialcomm * s, uint8_t adresse, int dlab, uint8_t access);

void serialcomm_write_bit(serialcomm * s, uint8_t adresse, int dlab, uint8_t access, uint8_t val, uint8_t bit);

void serialcomm_set_bit(serialcomm * s, uint8_t adresse, int dlab, uint8_t access, uint8_t bit);

void serialcomm_rst_bit(serialcomm * s, uint8_t adresse, int dlab, uint8_t access, uint8_t bit);

uint8_t serialcomm_read_bit(serialcomm * s, uint8_t adresse, int dlab, uint8_t access, uint8_t bit);

void serialcomm_deinit(serialcomm * s);
