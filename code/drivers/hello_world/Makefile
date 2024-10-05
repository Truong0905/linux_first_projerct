MODULE_NAME :=hello_world


ARC :=arm
CROSS_COMPILER :=arm-linux-gnueabihf-
KERN_DIR :=/home/truonglv/BBB/linux-5.10.168-ti-r79

PROJECT_DIR := $(shell pwd )

obj-m := $(MODULE_NAME).o

#.PHONY:pre_Build
#pre_Build:i
#	make ARCH=$(ARC) CROSS_COMPILE=$(CROSS_COMPILER) -C $(KERN_DIR) -j8 modules

.PHONY: prepare
prepare:
	make ARCH=$(ARC) CROSS_COMPILE=$(CROSS_COMPILER) -C $(KERN_DIR) -j8 prepare

.PHONY: build
build:
	make ARCH=$(ARC) CROSS_COMPILE=$(CROSS_COMPILER) -C $(KERN_DIR) M=$(PROJECT_DIR) -j8 modules

.PHONY:clean
clean:
	make ARCH=$(ARC) CROSS_COMPILE=$(CROSS_COMPILER) -C $(KERN_DIR) M=$(PROJECT_DIR) -j8 clean

.PHONY: help
help:
	make ARCH=$(ARC) CROSS_COMPILE=$(CROSS_COMPILER) -C $(KERN_DIR) M=$(PROJECT_DIR) help

.PHONY: print
print-%:
	@echo ---------------------------------------------
	@echo $($(subst print-,,$@))
	@echo ---------------------------------------------
