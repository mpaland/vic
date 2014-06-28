PROJ_NAME = vgx

##########################################
# Toolchain Settings
##########################################
CC  = g++
DBG = gdb

##########################################
# Files
##########################################
DIRS := $(wildcard */)
SRCS := $(wildcard $(addsuffix *.cpp,$(DIRS)))
OBJS := $(patsubst %.cpp,%.o,$(SRCS))

##########################################
# Includes
##########################################
INCLUDES = ./vgx
LIBS     =

##########################################
# Flag Settings
##########################################
CFLAGS  = -I$(INCLUDES) -g -Wall -O0 -std=gnu++0x
LDFLAGS = -g $(LIBS) -o $(PROJ_NAME)

##########################################
# Targets
##########################################
all: $(PROJ_NAME)

$(PROJ_NAME): $(OBJS)
	@$(CC) $(OBJS) $(LDFLAGS)
	@echo $@

clean:
	rm -f $(OBJS)
	rm -f $(PROJ_NAME)

debug:
	$(DBG) $(PROJ_NAME)
