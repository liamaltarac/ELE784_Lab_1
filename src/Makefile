obj-m := SerialDriver.o

SerialDriver-objs := circular.o SerialComm.o SerialPortDriver.o 


all:
	make -C /usr/src/linux-source-4.15.18/ M=`pwd` modules
	
clean:
	rm -f *.o *~ core
