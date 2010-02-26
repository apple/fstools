/*
 * Name: util.h
 * Project: CIFS Client Automatic Test
 * Author: Christian Starkjohann <cs@obdev.at>
 * Creation Date: 1998-04-28
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

#include "basictypes.h"

/* ------------------------------------------------------------------------- */

typedef double		real_t;						/* real numbers are double */

void	random_init(int na1, int na2, int na3, int nb1);
real_t	random_get(void);
int		random_int(int exclusive_range);
real_t	random_gauss(real_t stdval, real_t maxRange);
void	random_block(void *block, int len);
void	random_name(char *name, int len, int prefix);
const char * random_root();
void	random_path(char *path, int id, int running, const char *root);

void	perr(int e, char *s, ...);
void	cleanup(int sig);
/* ------------------------------------------------------------------------- */

