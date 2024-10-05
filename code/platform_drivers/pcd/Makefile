MODULE_NAME := pcd_device_setup pcd_platform_driver


ARC :=arm
CROSS_COMPILER :=arm-linux-gnueabihf-
KERN_DIR :=/home/truonglv/BBB/linux-5.10.168-ti-r79

HOST_KER_DIR := /lib/modules/$(shell uname -r)/build/

PROJECT_DIR := $(shell pwd )

obj-m := $(MODULE_NAME).o

#.PHONY:pre_Build
#pre_Build:i
#	make ARCH=$(ARC) CROSS_COMPILE=$(CROSS_COMPILER) -C $(KERN_DIR) -j4 modules

#.PHONY: prepare
#prepare:
#	make ARCH=$(ARC) CROSS_COMPILE=$(CROSS_COMPILER) -C $(KERN_DIR) -j4 prepare


.PHONY: build
build:
	make ARCH=$(ARC) CROSS_COMPILE=$(CROSS_COMPILER) -C $(KERN_DIR) M=$(PROJECT_DIR) -j4 modules


.PHONY:host_build
host_build:
	make -C $(HOST_KER_DIR) M=$(PROJECT_DIR) -j4 modules

.PHONY:clean
clean:
	make ARCH=$(ARC) CROSS_COMPILE=$(CROSS_COMPILER) -C $(KERN_DIR) M=$(PROJECT_DIR) -j4 clean

.PHONY: help
help:
	make ARCH=$(ARC) CROSS_COMPILE=$(CROSS_COMPILER) -C $(KERN_DIR) M=$(PROJECT_DIR) help

.PHONY: print
print-%:
	@echo ---------------------------------------------
	@echo $($(subst print-,,$@))
	@echo ---------------------------------------------
