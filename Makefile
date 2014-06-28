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
INCLUDES = /vgx;/demo/windows
LIBS     =

##########################################
# Flag Settings
##########################################
CFLAGS  = -I$(INCLUDES) -g -Wall -O0
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
