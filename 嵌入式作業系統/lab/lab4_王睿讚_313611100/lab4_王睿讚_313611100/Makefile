CROSS_COMPILE = aarch64-linux-gnu-
ARCH = arm64
obj-m += lab4_driver.o
KDIR = /home/ericwang/linux
PWD  = $(shell pwd)

driver:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

reader: reader.c
	aarch64-linux-gnu-gcc reader.c -o lab4_reader_arm64
	
writer: lab4_writer.c
	aarch64-linux-gnu-gcc lab4_writer.c -o lab4_writer_arm64
		
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
	rm -f lab4_reader_arm64
	rm -f lab4_writer_arm64
