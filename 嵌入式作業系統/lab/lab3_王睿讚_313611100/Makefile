CROSS_COMPILE = aarch64-linux-gnu-
ARCH = arm64
obj-m += lab3_driver.o
KDIR = /home/ericwang/linux
PWD  = $(shell pwd)

driver:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
	
writer: lab3_writter.c
	aarch64-linux-gnu-gcc lab3_writter.c -o writter_arm64

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
	rm -f writter
