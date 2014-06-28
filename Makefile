PROJ = vgx-demo

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
INCLUDES = vgx
LIBS     =

##########################################
# Flag Settings
##########################################
CCFLAGS = -I$(INCLUDES) -g -Wall -O0 -std=gnu++0x
LDFLAGS = -g $(LIBS)

##########################################
# Targets
##########################################
all: $(PROJ)

$(PROJ): $(OBJS)
	@$(CC) $(OBJS) $(LDFLAGS) -o $(PROJ)
	@echo $@

%.o: %.cpp
	$(CC) -c $(CCFLAGS) $< -o $@

clean:
	rm -f $(OBJS)
	rm -f $(PROJ_NAME)

debug:
	$(DBG) $(PROJ_NAME)
