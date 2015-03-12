#*********************************************************************
#Compiler
#*********************************************************************
CC = gcc
#*********************************************************************
#Directories
#*********************************************************************
ROOT = ./
SRC_MYSQL = mysql
SRC_COMMON = common
SRC_MAGIC = magicnet
APP_SRCDIR_MYSQL := $(ROOT)/$(SRC_MYSQL)
APP_SRCDIR_COMMON := $(ROOT)/$(SRC_COMMON)
APP_SRCDIR_MAGICNET := $(ROOT)/$(SRC_MAGIC)
SRCDIR += $(APP_SRCDIR_MYSQL)
SRCDIR += $(APP_SRCDIR_COMMON)
SRCDIR += $(APP_SRCDIR_MAGICNET)
VPATH = $(SRCDIR)
#*********************************************************************
#Compiler options
#*********************************************************************
CC_OPT = -c -fpic -lrt -Wall -g -I$(SRC_MAGIC) -I$(SRC_COMMON) -I$(SRC_MYSQL)
#*********************************************************************
#DEBUG OR RELASE
#*********************************************************************
DEBUG = YES
ifeq (YES, $(DEBUG))
CC_OPT += -D_DEBUG 
EXE_FILE = ./libD_SeNetWork.so.1.0.0
else
CC_OPT += -DNDEBUG
EXE_FILE = ./libR_SeNetWork.so.1.0.0
endif


#*********************************************************************
#Files to be compiled
#*********************************************************************
SRC_C := $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.c))
#*********************************************************************
#All link files for the object
#*********************************************************************
OBJ_C := $(notdir $(patsubst %.c, %.o, $(SRC_C)))
#*********************************************************************
#Rules for make
#*********************************************************************
$(EXE_FILE):$(OBJ_C)
	$(CC) -shared -lpthread -o $@ $^
	rm *.o

$(OBJ_C):%.o:%.c
	$(CC) $(CC_OPT) -o $@ $<
clean:
	rm *.o $(EXE_FILE)
