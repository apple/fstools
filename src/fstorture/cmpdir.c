/*
 * Name: cmpdir.c
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

#include "stdheaders.h"
#include "cmpdir.h"
#include "util.h"

extern int trace;
char *acl_perm_dir_n[] = { "list", "add_file", "search", "delete", "add_subdirectory", "delete_child", "readattr", "writeattr", "readextattr", "writeextattr", "readsecurity", "writesecurity", "chown" };
int acl_perm_t_dir_r[] = { ACL_LIST_DIRECTORY, ACL_ADD_FILE, ACL_SEARCH, ACL_DELETE, ACL_ADD_SUBDIRECTORY, ACL_DELETE_CHILD, ACL_READ_ATTRIBUTES, ACL_WRITE_ATTRIBUTES, ACL_READ_EXTATTRIBUTES, ACL_WRITE_EXTATTRIBUTES, ACL_READ_SECURITY, ACL_WRITE_SECURITY, ACL_CHANGE_OWNER };
char errstr[1024];

/* ------------------------------------------------------------------------- */

const int BUF_SIZE=32768;

static unsigned char	*blockWithPattern(int pattern, unsigned char block[BUF_SIZE])
{
int		i;
	srand(pattern);	/* make reproducible random sequence */
	for(i=0;i<BUF_SIZE;i++)
		block[i] = rand() >> 8;
	return block;
}

static btbool	verifyFile(cmpdir_t *dir, int i)
{
int		fileLen;
unsigned char block[BUF_SIZE], buffer[BUF_SIZE];

	if(lseek(dir->files[i].fd, 0, SEEK_SET) == -1){
		perr(errno, "error seeking (read) in file %s in dir %s", dir->files[i].name, dir->name);
		return 0;
	}
	fileLen = read(dir->files[i].fd, buffer, BUF_SIZE);
	if(fileLen < 0){
		perr(errno, "error reading file %s in dir %s", dir->files[i].name, dir->name);
		return 0;
	}
	if(fileLen != BUF_SIZE){
		perr(errno, "read size error reading file %s in dir %s: %d", dir->files[i].name, dir->name, fileLen);
		return 0;
	}
	blockWithPattern(dir->files[i].filePattern, block);
	if(memcmp(block, buffer, BUF_SIZE) != 0){
		perr(errno, "verify error in file %s in dir %s filepattern:%d", dir->files[i].name, dir->name, dir->files[i].filePattern);
		return 0;
	}
	return 1;
}

/* ------------------------------------------------------------------------- */

cmpdir_t	*cmpdirNew(char *path)
{
cmpdir_t	*dir = calloc(1, sizeof(cmpdir_t));

	if(trace)
		printf("creating directory %s\n", path);
	dir->name = malloc(strlen(path) + 1);
	strcpy(dir->name, path);
	if(mkdir(path, 0700) != 0){
		perr(errno, "error making dir %s", path);
		free(dir->name);
		free(dir);
		return NULL;
	}
	return dir;
}

/* ------------------------------------------------------------------------- */

btbool	cmpdirFree(cmpdir_t *dir)
{
int		i;
btbool	rval = 1;
char	path[1024];

	if(trace)
		printf("destroying directory %s\n", dir->name);
	for(i=0;i<CMPDIR_MAX_FILES;i++){
		if(dir->files[i].name[0] != 0){
			if(!verifyFile(dir, i))
				rval = 0;
			if(close(dir->files[i].fd)){
				perr(errno, "error closing file %s in dir %s", dir->files[i].name, dir->name);
				rval = 0;
			}
			sprintf(path, "%s/%s", dir->name, dir->files[i].name);
			if(unlink(path)){
				perr(errno, "error unlinking file %s", path);
				rval = 0;
			}
			dir->files[i].fd = 0;
			dir->files[i].name[0] = 0;
		}
	}
	if(rmdir(dir->name)){
		perr(errno, "error removing dir %s", dir->name);
		rval = 0;
	}
	if(dir->name != NULL)
		free(dir->name);
	free(dir);
	return rval;
}

/* ------------------------------------------------------------------------- */

btbool	cmpdirRename(cmpdir_t *dir, char *newPath, int WinVolume)
{
int ii;
	
	if(trace)
		printf("renaming directory %s to %s\n", dir->name, newPath);
	if(rename(dir->name, newPath) < 0) {
		/* 
		 * Remember that Windows will not let you rename a directory, if
		 * there is an open file in the directory. So if we get an EACCES
		 * error, and there are any open files in the directory just ignore
		 * the error.
		 */
		if (WinVolume && (errno == EACCES)) {
			for (ii = 0; ii < CMPDIR_MAX_FILES; ii++)
				if (dir->files[ii].fd)
					return 1;
		}
		perr(errno, "error renaming dir %s to %s", dir->name, newPath);
		return 0;
	}
	free(dir->name);
	dir->name = malloc(strlen(newPath) + 1);
	strcpy(dir->name, newPath);
	return 1;
}

/* ------------------------------------------------------------------------- */

btbool	cmpdirChangeFiles(cmpdir_t *dir)
{
btbool	rval = 1;
int		i, len;
char	path[1024];
unsigned char block[BUF_SIZE];

	if(trace)
		printf("changing file in %s\n", dir->name);
	i = random_int(CMPDIR_MAX_FILES);
	if(dir->files[i].name[0]){	/* file exists */
		if(!verifyFile(dir, i))
			rval = 0;
		if(close(dir->files[i].fd)){
			perr(errno, "error closing file %s in dir %s", dir->files[i].name, dir->name);
			rval = 0;
		}
		sprintf(path, "%s/%s", dir->name, dir->files[i].name);
		if(unlink(path)){
			perr(errno, "error unlinking file %s", path);
			rval = 0;
		}
		dir->files[i].fd = 0;
		dir->files[i].name[0] = 0;
	}else{	/* file does not exist */
		random_name(dir->files[i].name, 8, i);
		dir->files[i].filePattern = random_int(65536);
		sprintf(path, "%s/%s", dir->name, dir->files[i].name);
		if((dir->files[i].fd = open(path, O_RDWR|O_CREAT|O_TRUNC, 0600)) < 0){
			perr(errno, "error creating file %s in dir %s", dir->files[i].name, dir->name);
			rval = 0;
			dir->files[i].fd = 0;
			dir->files[i].name[0] = 0;
		}else{
			if((len = write(dir->files[i].fd, blockWithPattern(dir->files[i].filePattern, block), BUF_SIZE)) != BUF_SIZE){
				perr(errno, "error writing file %s, wlen = %d", path, len);
				rval = 0;
			}
		}
	}
	return rval;
}

/* ------------------------------------------------------------------------- */

btbool	cmpdirVerifyFiles(cmpdir_t *dir)
{
btbool	rval = 1;
int		i;

	if(trace)
		printf("verifying all in %s\n", dir->name);
	for(i=0;i<CMPDIR_MAX_FILES;i++){
		if(dir->files[i].name[0]){	/* file exists */
			if(!verifyFile(dir, i))
				rval = 0;
		}
	}
	return rval;
}

/* ------------------------------------------------------------------------- */
