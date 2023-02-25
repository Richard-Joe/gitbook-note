obj-m := test.o
KERNEL_DIR := /lib/modules/$(shell uname -r)/build/
PWD := $(shell pwd)

modules:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.c .*.cmd .*.mk *.order *.symvers .tmp_versions

.PHONY: modules clean