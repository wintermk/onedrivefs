# Makefile for OneDriveFS
#
#

### Name
NAME = OneDriveFS
VERSION = 0.0.1

### Directories
PREFIX = /usr/local
CFGDIR = $(PREFIX)/etc
BINDIR = $(PREFIX)/bin

### Compiler flags:
CC       ?= gcc
CFLAGS   ?= -g -O3 -Wall
CDEFINES  = -D_GNU_SOURCE
DEFINES  += $(CDEFINES)
export CFLAGS

### PKG-Config Libs
DEPENDENCIES = json-c fuse libcurl

### Libraries and compiler options
LIBS += $(foreach dependence, $(DEPENDENCIES), $(shell pkg-config --libs $(dependence)) )
INCLUDES += $(foreach dependence, $(DEPENDENCIES), $(shell pkg-config --cflags $(dependence)) )

### Includes and Defines
DEFINES += -DPNAME=\"$(NAME)\" -DPVERSION=\"$(VERSION)\"
#INCLUDES +=

### Debug
DEFINES += -DDEBUG

### Binary
BINARY = mount.$(shell echo $(NAME) | tr '[:upper:]' '[:lower:]')

### The source files
SRCS = $(wildcard *.c)

### The object files (derived from source files)
OBJS = $(SRCS:%.c=%.o)

### The main target:
all: $(NAME)

### Implicit rules:

%.o: %.c
	$(CC) $(CFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

### Dependencies:

MAKEDEP = $(CC) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(CFLAGS) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.c) > $@

-include $(DEPFILE)

### Targets:

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $(BINARY)

clean:
	@-rm -f $(OBJS) $(DEPFILE) $(BINARY)

