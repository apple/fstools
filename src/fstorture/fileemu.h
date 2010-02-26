/*
 * Name: fileemu.h
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
The file class defined in this module emulates the behaviour of files on
disk media. It is used as a reference when the results of file operations
must be verified.
*/

/* ------------------------------------------------------------------------- */

#include "basictypes.h"

typedef struct file{
	btbool	isOpen;
	btbool	isWritable;
	btu8	*touchedBits;
	int		touchedSize;
	char	*data;
	int		allocSize;
	int		size;
}file_t;

/* ------------------------------------------------------------------------- */

file_t	*fileNew(void);
/* Creates a new file_t object
 */
void	fileFree(file_t *file);
/* Frees an existing file_t object
 */
btbool	fileOpen(file_t *file, btbool writable);
/* Sets a file_t object into the "opened" state. The flag "writable" defines
 * whether writes to the file will be possible.
 */
void	fileClose(file_t *file);
/* Sets the file_t object into the closed state.
 */
void	fileWrite(file_t *file, int offset, int len, void *data);
/* Writes to the file_t object. The data written to the file is stored in
 * memory and the range of already written-to bytes is also stored.
 */
int	fileReadSize(file_t *file, int offset, int len);
/* Returns the number of bytes that would be read from the file_t object
 * if a read with the parameters 'offset' and 'len' would be performed.
 */
int	fileRead(file_t *file, int offset, int len, void *dest);
/* Reads data from the file. This method is currently not used.
 */
btbool	fileCompareRead(file_t *file, int offset, int len, void *data);
/* This method verifies whether the data read from the "real" file is correct.
 * It takes into account that only those parts of the block should be verified
 * that have already been written to. Unwritten parts that are not entire
 * filesystem blocks or pages may contain old data of previous contents.
 * The function returns 1, if no compare error occured.
 */
void	fileTruncate(file_t *file, int len);
/* Sets the file size to the given value and invalidates all data behind the
 * end of file.
 */
void	fileSetMode(file_t *file, btbool writable);
/* Changes the write protection mode of the file. The write protection is only
 * checked by fileOpen(). Whether writes are possible depends on the open mode
 * used.
 */
btbool	fileIsWritable(file_t *file);
/* Returns the write protection status of the file.
 */

static inline int	fileSize(file_t *file)
{
	return file->size;
}

/* ------------------------------------------------------------------------- */
