/*
 * Name: basictypes.h
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

#ifndef __BASICTYPES_H_INCLUDED__
#define __BASICTYPES_H_INCLUDED__

/*
General Description:
This header defines an abstraction for the primitive C datatypes. It is not
necessary to have this in the simple test program, but I became used to
the bt* names...
*/

#define	bti8		char
#define	bti16		short
#define	bti32		int
#define	bti64		long long
#define	btu8		unsigned char
#define	btu16		unsigned short
#define	btu32		unsigned
#define	btu64		unsigned long long
#define	btoffset	btu32	/* offset in files (currently 32 bit) */
#define	btuni		unsigned short	/* unicode character */
#define	btbool		char

#define	btsize		int		/* for data block sizes */
#define	btint		int		/* fast integer, min 32 bit long */


#endif	/* __BASICTYPES_H_INCLUDED__ */
