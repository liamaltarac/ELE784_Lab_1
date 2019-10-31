obj-m := SerialDriver.o

SerialDriver-objs := circular.o SerialPortDriver.o


all:
	make -C /usr/src/linux-headers-4.15.18+/ M=`pwd` modules
	
clean:
	rm -f *.o *~ core