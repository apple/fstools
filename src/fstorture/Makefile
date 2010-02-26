# Name: Makefile
# Project: CIFS Client Automatic Test
# Author: Christian Starkjohann <cs@obdev.at>
# Creation Date: 1998-04-20
# Tabsize: 4
# Copyright: (c) 1998 by Christian Starkjohann. This program is distributed
#     under the terms of the Gnu General Public License (GPL).
# 
# Copyright © 2009 Apple Inc.  
# This program is free software; you can redistribute it and/or modify it 
# under the terms of the GNU General Public License version 2 only.  This 
# program is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied WARRANTY OF MERCHANTABILITY OR 
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
# version 2 for more details.  A copy of the GNU General Public License 
# version 2 is included along with this program.
# 

#
# The testsoftware is currently not ported to all platforms Sharity
# runs on. It is a naive quick hack to find bugs quickly. Please modify
# the CFLAGS and LIBS lines appropriately for your platform.
#

# USE XILOG
#CFLAGS= -Wall -g -O0 -DXILOG -F/AppleInternal/Library/Frameworks
ARCHES= -arch ppc -arch i386 -arch x86_64
CFLAGS= $(ARCHES) -Wall -O3 -DXILOG -F/AppleInternal/Library/Frameworks
LDFLAGS= -framework XILog

# DON'T USE XILOG
#CFLAGS= -Wall -O3 -fomit-frame-pointer

# For Linux add:
# LIBS = -lm

OBJ =	fileemu.o cmpfile.o cmpdir.o fstorture.o

XNAME =	fstorture

all: $(XNAME)

Project = $(XNAME)

#include $(MAKEFILEPATH)/CoreOS/ReleaseControl/Common.make


.c.o:
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean::
	rm -f $(OBJ) $(XNAME)

$(XNAME):$(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)




# More stuff


XintInstall = $(DSTROOT)/$(HOME)/TestSuite/Tools/$(XNAME)

install::       $(XintInstall)/$(XNAME)

$(XintInstall)/$(XNAME):$(XNAME)
	$(CP) $(SRCROOT)/$(XNAME) $(SYMROOT)
	$(STRIP) $(SRCROOT)/$(XNAME)
	$(MKDIR) $(XintInstall)
	$(MV) $(SRCROOT)/$(XNAME) $(XintInstall)

