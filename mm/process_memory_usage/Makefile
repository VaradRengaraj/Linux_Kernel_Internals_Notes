obj-m += process_mem_usage.o

KERNEL_SRC := /home/varadarajan/learning/linux/kernel/lk/linux
PWD := $(shell pwd)

all:
	make -C $(KERNEL_SRC) M=$(PWD) modules

clean:
	make -C $(KERNEL_SRC) M=$(PWD) clean

