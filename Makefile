PROJ_NAME = vgx

##########################################
# Toolchain Settings
##########################################
CC = g++
DBG = gdb

##########################################
# Files
##########################################
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o) $(SRCS_SERIAL:.cpp=.o)
INCLUDES = /vgx
LIBS =

##########################################
# Flag Settings
##########################################
CFLAGS = $(INCLUDES) -g -Wall -O0
LDFLAGS = -g $(LIBS) -o $(PROJ_NAME)

##########################################
# Targets
##########################################
all: $(PROJ_NAME)

$(PROJ_NAME): $(OBJS)
@$(CC) $(OBJS) $(LDFLAGS)
@echo $@

.cpp.o:
@$(CC) $(CFLAGS) -c -o $@ $<
@echo $@

clean:
rm -f $(OBJS)
rm -f $(PROJ_NAME)

debug:
$(DBG) $(PROJ_NAME)