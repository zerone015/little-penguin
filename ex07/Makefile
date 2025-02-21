obj-m += debugfs_module.o
debugfs_module-y = main.o id.o foo.o

K-DIR = /lib/modules/$(shell uname -r)/build

all :
	make -C $(K-DIR) M=$(CURDIR) modules

clean :
	make -C $(K-DIR) M=$(CURDIR) clean

.PHONY: all clean
