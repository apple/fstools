/*
 * Name: fileemu.c
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

#include "stdheaders.h"
#include "fileemu.h"

/* ------------------------------------------------------------------------- */

static void	reallocTouched(file_t *file)
{
int		size = file->allocSize / 8 + 1;

	file->touchedBits = realloc(file->touchedBits, size);
	memset(file->touchedBits + file->touchedSize, 0, size - file->touchedSize);
	file->touchedSize = size;
}

/* ------------------------------------------------------------------------- */

static void	bitarrSetRange(btu8 *arr, btsize startBitIndex, btsize numBits, btbool value)
{
btsize	fillStart, startRemainder, fillEnd, endRemainder;
btu8	startMask, endMask;
   
    fillStart = startBitIndex / 8 + 1;
    startRemainder = startBitIndex % 8;
    fillEnd = (startBitIndex + numBits) / 8;
    endRemainder = (startBitIndex + numBits) % 8;
    if(fillEnd > fillStart)
    	memset(arr + fillStart, value ? 0xff : 0, fillEnd - fillStart);
	startMask = ~((1<<startRemainder) - 1);
	endMask = (1<<endRemainder) - 1;
	if(value){
		if(fillStart - 1 == fillEnd){
			arr[fillEnd] |= startMask & endMask;
		}else{
			arr[fillStart - 1] |= startMask;
			arr[fillEnd] |= endMask;
		}
	}else{
		if(fillStart - 1 == fillEnd){
			arr[fillEnd] &= ~(startMask & endMask);
		}else{
			arr[fillStart - 1] &= ~startMask;
			arr[fillEnd] &= ~endMask;
		}
	}
}

/* ------------------------------------------------------------------------- */

static inline btbool	bitarrBitIsSet(btu8 *arr, btsize bitIndex)
{
	return (arr[bitIndex/8] & (1<<(bitIndex % 8))) != 0;
}

/* ------------------------------------------------------------------------- */

file_t	*fileNew(void)
{
file_t	*file;

	file = calloc(1, sizeof(file_t));
	file->isOpen = 0;
	file->isWritable = 1;
	file->allocSize = 32768;
	file->data = calloc(1, file->allocSize);
	file->size = 0;
	reallocTouched(file);
	return file;
}

/* ------------------------------------------------------------------------- */

void	fileFree(file_t *file)
{
	free(file->data);
	if(file->touchedBits != NULL)
		free(file->touchedBits);
	free(file);
}

/* ------------------------------------------------------------------------- */

btbool	fileOpen(file_t *file, btbool writable)
{
	if(writable && !file->isWritable)
		return 0;
	file->isOpen = 1;
	return 1;
}

/* ------------------------------------------------------------------------- */

void	fileClose(file_t *file)
{
	file->isOpen = 0;
}

/* ------------------------------------------------------------------------- */

void	fileWrite(file_t *file, int offset, int len, void *data)
{
int	maxlen = offset + len, newSize;

	if(!file->isOpen){
		fprintf(stderr, "fileemu: write to closed file\n");
		return;
	}
	if(maxlen > file->allocSize){
		newSize = file->allocSize;
		while(maxlen > newSize)
			newSize *= 2;
		file->data = realloc(file->data, newSize);
		memset(file->data + file->allocSize, 0, newSize - file->allocSize);
		file->allocSize = newSize;
		reallocTouched(file);
	}
	memcpy(file->data + offset, data, len);
	bitarrSetRange(file->touchedBits, offset, len, 1);
	if(file->size < maxlen)
		file->size = maxlen;
}

/* ------------------------------------------------------------------------- */

int	fileReadSize(file_t *file, int offset, int len)
{
int	maxlen = offset + len;

	if(!file->isOpen){
		fprintf(stderr, "fileemu: read from closed file\n");
		return 0;
	}
	if(maxlen > file->size){
		len = file->size - offset;
	}
	if(len <= 0){
		return 0;
	}
	return len;
}

/* ------------------------------------------------------------------------- */

int	fileRead(file_t *file, int offset, int len, void *dest)
{
	len = fileReadSize(file, offset, len);
	memcpy(dest, file->data + offset, len);
	return len;
}

/* ------------------------------------------------------------------------- */

btbool	fileCompareRead(file_t *file, int offset, int len, void *data)
{
int		i;
char	*refData = data;
int		errors = 0, firstErrorIndex=0, lastErrorIndex=0;
int		originalOffset = offset;

	if(fileReadSize(file, offset, len) != len){
		fprintf(stderr, "file compare failed due to size error\n");
		return 0;
	}
	for(i=0;i<len;i++){
		if(bitarrBitIsSet(file->touchedBits, offset)){	/* is touched */
			if(file->data[offset] != refData[i]){
				if(errors == 0)
					firstErrorIndex = offset;
				lastErrorIndex = offset;
				if(errors < 20)
					fprintf(stderr, "data in file[%d] is 0x%02x instead of 0x%02x\n", offset, (btu8)refData[i], (btu8)file->data[offset]);
				errors++;
			}
		}
		offset++;
	}
	if(errors != 0){
		fprintf(stderr, "fileCompareRead: %d errors in data block: first=%d last=%d, blockstart=%d, blocklen=%d\n", errors, firstErrorIndex, lastErrorIndex, originalOffset, len);
	}
	return errors == 0;
}

/* ------------------------------------------------------------------------- */

void	fileTruncate(file_t *file, int len)
{
int	maxlen = len, newSize;

	if(maxlen > file->allocSize){
		newSize = file->allocSize;
		while(maxlen > newSize)
			newSize *= 2;
		file->data = realloc(file->data, newSize);
		memset(file->data + file->allocSize, 0, newSize - file->allocSize);
		file->allocSize = newSize;
		reallocTouched(file);
	}
	if(maxlen < file->size){	/* file shrinks */
		bitarrSetRange(file->touchedBits, maxlen, file->size - maxlen, 0);
	}
	file->size = maxlen;
}

/* ------------------------------------------------------------------------- */

void	fileSetMode(file_t *file, btbool writable)
{
	file->isWritable = writable;
}

/* ------------------------------------------------------------------------- */

btbool	fileIsWritable(file_t *file)
{
	return file->isWritable;
}

/* ------------------------------------------------------------------------- */
