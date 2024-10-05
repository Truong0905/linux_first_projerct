PROJECT_NAME        := led


CC := arm-linux-gnueabihf-gcc
LD := arm-linux-gnueabihf-ld

CFLAGS := -Wall -Wextra

#PROJECT_DIR = $(shell pwd)

PROJECT_DIR =.

OUT_DIR :=$(PROJECT_DIR)/out

#Create an Variable to store all include directories
INCLUDE_DIRS =$(PROJECT_DIR)/inc

# Include files 
INCLUDE_FILES =$(foreach DIR, $(INCLUDE_DIRS), $(wildcard $(DIR)/*.h ))

#Create an Variable to store all source directories
SRC_DIRS = $(PROJECT_DIR)/src

# SOURCES files
SRC_FILES =$(foreach DIR, $(SRC_DIRS), $(wildcard $(DIR)/*.c))

# OBJECTS files
OBJECT_FILES =$(addprefix $(OUT_DIR)/, $(notdir $(patsubst %.c, %.o, $(SRC_FILES))))


INC_DIR_OPT :=$(foreach DIR,$(INCLUDE_DIRS),-I$(DIR))


CCOPT :=$(INC_DIR_OPT)

vpath %.c  $(SRC_DIRS)
vpath %.h  $(INCLUDE_DIRS)

$(OUT_DIR)/%.o: %.c $(INCLUDE_FILES)
	$(CC) $(CCOPT) $(CFLAGS) -c $< -o $@


TARGET := $(PROJECT_NAME)

$(TARGET): $(OBJECT_FILES)
	$(CC) $(CFLAGS)  $^ -o $(OUT_DIR)/$@

.PHONY:
build: create  $(TARGET)
	@echo Build successfully!

.PHONY:
create:
	 mkdir $(OUT_DIR)


clean:
	rm -r $(OUT_DIR)

.PHONY:
print-%:
	@echo $($(subst print-,,$@))

