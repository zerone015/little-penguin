obj-m := ft_debugfs.o
ft_debugfs-objs := main.o id.o jiffies.o foo.o

PWD := $(CURDIR)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
