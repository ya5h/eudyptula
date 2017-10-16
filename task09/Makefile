obj-m += hello.o 
#obj-m += kobj-example.o 

CFLAGS_hello.o := -DDEBUG
#CFLAGS_kobj-example.o := -DDEBUG

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean 
