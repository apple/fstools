/*
 * Name: stdheaders.h
 * Project: CIFS Client Automatic Test
 * Author: Christian Starkjohann <cs@obdev.at>
 * Creation Date: 1998-04-20
 * Tabsize: 4
 * Copyright: (c) 1998 by Christian Starkjohann. This program is distributed
 *     under the terms of the Gnu General Public License (GPL).
 *
 * Copyright © 2009 Apple Inc.  
 * This program is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU General Public License version 2 only.  This 
 * program is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied WARRANTY OF MERCHANTABILITY OR 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 * version 2 for more details.  A copy of the GNU General Public License 
 * version 2 is included along with this program. 
 */

/*
General Description:
This header includes all system headers that are needed. 
*/

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <membership.h>
#include <pthread.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef NeXT
#	include <libc.h>
#endif
