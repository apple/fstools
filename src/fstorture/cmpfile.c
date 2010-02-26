/*
 * Name: cmpfile.c
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
#include "cmpfile.h"
#include "fileemu.h"
#include "util.h"

extern int trace;
char *acl_perm_t_file_n[] = { "read", "write", "execute", "delete", "append", "readattr", "writeattr", "readextattr", "writeextattr", "readsecurity", "writesecurity", "chown" };
int acl_perm_t_file_r[] = { ACL_READ_DATA, ACL_WRITE_DATA, ACL_EXECUTE, ACL_DELETE, ACL_APPEND_DATA, ACL_READ_ATTRIBUTES, ACL_WRITE_ATTRIBUTES, ACL_READ_EXTATTRIBUTES, ACL_WRITE_EXTATTRIBUTES, ACL_READ_SECURITY, ACL_WRITE_SECURITY, ACL_CHANGE_OWNER };
extern uuid_t *uuid;
extern pthread_t thr[THREADS];
extern pthread_mutex_t mutex_start;
extern pthread_cond_t cond_start;
char filename_shared[1024];
int howmany = 0;
int acl_err = 1;


/* ------------------------------------------------------------------------- */

cmpfile_t	*cmpfileNew(char *path, const char *root)
{
cmpfile_t	*cmpfile = calloc(1, sizeof(cmpfile_t));

	if(trace)
		printf("creating file %s\n", path);
	cmpfile->refFile = fileNew();
	cmpfile->fd = creat(path, 0600);
	if(cmpfile->fd < 0){
		perr(errno, "error creating file %s", path);
	}
	if(close(cmpfile->fd) < 0){
		perr(errno, "error closing file %s", path);
	}
	cmpfile->name = malloc(strlen(path) + 1);
	strcpy(cmpfile->name, path);
	cmpfile->isOpen = 0;
	cmpfile->root = root;
	return cmpfile;
}

/* ------------------------------------------------------------------------- */

btbool cmpfileLink(cmpfile_t *cmpfile, char *linkName, int hard)
{
	if (!strcasecmp(cmpfile->name, linkName)) {
//		printf("CASE MATCHES: %s %s\n", cmpfile->name, linkName);
		return 1;
	}

	if (trace)
		printf("%s linking file %s to %s\n", hard ? "hard" : "soft", cmpfile->name, linkName);

	if (hard) { // Hard link request
		if (link(cmpfile->name, linkName) < 0) {
			perr(errno, "error hard linking file %s to %s", cmpfile->name, linkName);
			return 0;
		}
	} else { // Soft link request
		if (symlink(cmpfile->name, linkName) < 0) {
			perr(errno, "error soft linking file %s to %s", cmpfile->name, linkName);
			return 0;
		}	
	}
	unlink(linkName);
	return 1;
}


/* ------------------------------------------------------------------------- */

btbool	cmpfileRename(cmpfile_t *cmpfile, char *newName)
{
	filename_shared[0] = '\0';
	if (trace)
		printf("Renaming file %s to %s\n", cmpfile->name, newName);
	if(rename(cmpfile->name, newName) < 0){
		perr(errno, "error renaming file %s to %s", cmpfile->name, newName);
		return 0;
	}
	free(cmpfile->name);
	cmpfile->name = malloc(strlen(newName) + 1);
	strcpy(cmpfile->name, newName);
	return 1;
}

/* ------------------------------------------------------------------------- */

void	cmpfileFree(cmpfile_t *cmpfile, btbool hadErr, btbool unlinkme)
{
char	errName[1024];

	if(trace)
		printf("destroying file %s\n", cmpfile->name);
	if(cmpfile->isOpen){
		if(close(cmpfile->fd) < 0){
			perr(errno, "error closing file %s", cmpfile->name);
		}
		cmpfile->isOpen = 0;
	}
	if(hadErr){
		sprintf(errName, "%s.error", cmpfile->name);
		if(rename(cmpfile->name, errName) < 0){
			perr(errno, "error renaming file %s to %s", cmpfile->name, errName);
		}
	}else if (unlinkme) {
		if(unlink(cmpfile->name) < 0){
			perr(errno, "error unlinking file %s", cmpfile->name);
		}
	}
	fileFree(cmpfile->refFile);
	cmpfile->refFile = NULL;
	free(cmpfile->name);
	cmpfile->name = NULL;
	free(cmpfile);
}

/* ------------------------------------------------------------------------- */

btbool	cmpfileOpen(cmpfile_t *cmpfile, btbool writable, btbool nocache)
{
	if(trace)
		printf("opening file %s %s\n", cmpfile->name, writable ? "writable" : "");
	if(!fileOpen(cmpfile->refFile, writable))
		return 0;
	cmpfile->fd = open(cmpfile->name, writable ? O_RDWR : O_RDONLY, 0);
	if(cmpfile->fd < 0){
		perr(errno, "error opening file %s %s", cmpfile->name, writable ? "RDWR" : "RDONLY");
		fileClose(cmpfile->refFile);
		return 0;
	}

	if (nocache) 
		fcntl(cmpfile->fd, F_NOCACHE, 1);
	cmpfile->isOpen = 1;
	return 1;
}

/* ------------------------------------------------------------------------- */

btbool	cmpfileClose(cmpfile_t *cmpfile)
{
	if(trace)
		printf("closing file %s\n", cmpfile->name);
	fileClose(cmpfile->refFile);
	if(close(cmpfile->fd) < 0){
		perr(errno, "error closing file %s", cmpfile->name);
	}
	cmpfile->isOpen = 0;
	return 1;
}

/* ------------------------------------------------------------------------- */

btbool	cmpfileWrite(cmpfile_t *cmpfile, int offset, int len, void *data)
{
int	wlen;

	if(trace)
		printf("writing file %s offset=%d, len=%d\n", cmpfile->name, offset, len);
	fileWrite(cmpfile->refFile, offset, len, data);
	if(lseek(cmpfile->fd, offset, SEEK_SET) == -1){
		perr(errno, "error seeking (write) in file %s", cmpfile->name);
		return 0;
	}
	wlen = write(cmpfile->fd, data, len);
	if(wlen != len){
		perr(errno, "error writing file %s, wlen = %d", cmpfile->name, wlen);
		return 0;
	}
	return 1;
}

/* ------------------------------------------------------------------------- */

btbool	cmpfileRead(cmpfile_t *cmpfile, int offset, int len)
{
char	*fileBuf;
int		refLen, fileLen;
btbool	rval = 1;

	if(trace)
		printf("reading file %s offset=%d, len=%d\n", cmpfile->name, offset, len);
	fileBuf = malloc(len);
	refLen = fileReadSize(cmpfile->refFile, offset, len);
	if(lseek(cmpfile->fd, offset, SEEK_SET) == -1){
		perr(errno, "error seeking (read) in file %s", cmpfile->name);
		rval = 0;
	}else{
		fileLen = read(cmpfile->fd, fileBuf, len);
		if(fileLen < 0){
			perr(errno, "error reading file %s", cmpfile->name);
			rval = 0;
		}else{
			if(refLen != fileLen){
				perr(0, "read size error reading file %s: %d instead of %d", cmpfile->name, fileLen, refLen);
				rval = 0;
			}else{
				if(!fileCompareRead(cmpfile->refFile, offset, refLen, fileBuf)){
					perr(0, "read error file %s: content error offset=%d len=%d size=%d", cmpfile->name, offset, len, fileSize(cmpfile->refFile));
					rval = 0;
				}
			}
		}
	}
	free(fileBuf);
	return rval;
}

/* ------------------------------------------------------------------------- */

btbool	cmpfileTruncate(cmpfile_t *cmpfile, int len)
{
int	rval;

	fileTruncate(cmpfile->refFile, len);
	if(cmpfile->isOpen){
		if(trace)
			printf("ftruncating file %s to %d\n", cmpfile->name, len);
		rval = ftruncate(cmpfile->fd, len);
	}else{
		if(trace)
			printf("truncating file %s to %d\n", cmpfile->name, len);
		rval = truncate(cmpfile->name, len);
	}
	if(rval < 0){
		perr(errno, "error (f)truncating file %s", cmpfile->name);
		return 0;
	}
	return 1;
}

/* ------------------------------------------------------------------------- */

btbool	cmpfileSetMode(cmpfile_t *cmpfile, btbool writable)
{
int rval = 0;
	if(trace)
		printf("setting mode of file %s to %s\n", cmpfile->name, writable ? "writable" : "not writable");
	fileSetMode(cmpfile->refFile, writable);
	if (cmpfile->isOpen) {
		rval = fchmod(cmpfile->fd, writable ? 0600 : 0400);
	} else {
		rval = chmod(cmpfile->name, writable ? 0600 : 0400);
	}
	if(rval < 0){
		perr(errno, "error chmoding file %s", cmpfile->name);
		return 0;
	}
	return 1;
}

/* ------------------------------------------------------------------------- */

void *thr_start() { // here, we just wait for work == when howmany != 0
	while (1) {
		pthread_cond_wait(&cond_start, &mutex_start);
		pthread_mutex_unlock(&mutex_start);
		while (howmany > 0) {
			acl_err = cmpfileAcls_do();
			howmany--;
			usleep(100);
		}
	}
	return(0);
}
		
btbool  cmpfileAcls(cmpfile_t *cmpfile)
{
	howmany = random_int(200); // how many iterations of acl bustin' we should do
	if (trace) {
		printf("acl busting: %i times\n", howmany);
	}
	strcpy(filename_shared, cmpfile->name);
	pthread_cond_broadcast(&cond_start);
	while (howmany > 0) { 
		if (acl_err == 0) {
			return 0;
		}
		usleep(1);
	}
	return 1;
}
btbool  cmpfileAcls_do()
{                               // don't forget: sudo /usr/sbin/fsaclctl -p / -e
	acl_t   acl;
	acl_entry_t ace;
	acl_permset_t perms;

	if (trace) 
		printf("%p: acl'ing file %s\n", pthread_self(), filename_shared);

	if (NULL == (acl = acl_init(32))) { 
		perr(errno, "acl_init()"); 
		return 0;
	}

	if (0 != acl_create_entry(&acl, &ace)) {
		perr(errno, "acl_create_entry()");
		return 0;
	}

	/* allow or deny */
	if (0 != acl_set_tag_type(ace, (random_int(2) == 1) ? ACL_EXTENDED_ALLOW : ACL_EXTENDED_DENY)) {
		perr(errno, "acl_set_tag_type()");
		return 0;
	}

	/* associate this with our uuid */
	if (0 != acl_set_qualifier(ace, uuid)) {
		perr(errno, "acl_set_qualifier()");
		return 0;
	}

	if (0 != acl_get_permset(ace, &perms)) {
		perr(errno, "acl_get_permset()");
		return 0;
	}

	switch(random_int(3)) {
		case 0: /* ADD */
			if (0 != acl_add_perm(perms, acl_perm_t_file_r[random_int(12)])) {
				perr(errno, "acl_add_perm()"); 
				return 0;
			}
			break;
		case 1: /* DELETE */
			if (0 != acl_delete_perm(perms, acl_perm_t_file_r[random_int(12)])) {
				perr(errno, "acl_delete_perms()");
				return 0;
			}
			break;
		case 2: /* CLEAR */
			if (0 != acl_clear_perms(perms)) {
				perr(errno, "acl_clear_perms()");
				return 0;
			}
			break;
	}

	if (0 != acl_set_permset(ace, perms)) { 
		perr(errno, "acl_set_permset()");
		return 0;
	}

	if (filename_shared[0] != '\0') {
		if (0 != acl_set_file(filename_shared, ACL_TYPE_EXTENDED, acl)) {
			perr(errno, "acl_set_file()");
			printf("f:%s\n", filename_shared);
			return 0;
		}
	}

	acl_free(acl);

	return 1;
}



// This is unused and was the first quick attempt
int cmpfileAcls2(cmpfile_t *cmpfile)
{				// don't forget: sudo /usr/sbin/fsaclctl -p / -e
	char dothis[100];
	sprintf(dothis, "www");

	switch(random_int(3)) {
		case 0: /* ADD */
			sprintf(dothis, "%s %s %s", dothis, 
					(random_int(2) == 1) ? "allow" : "deny", 
					acl_perm_t_file_n[random_int(12)]);

			strcpy(cmpfile->acl, dothis);
			sprintf(dothis, "exec chmod +a \"%s\" %s", cmpfile->acl, cmpfile->name);
			break;

		case 1: /* REMOVE */
			if (cmpfile->acl[0] != '\0') {
				sprintf(dothis, "exec chmod -a \"%s\" %s", cmpfile->acl, cmpfile->name);
				cmpfile->acl[0] = '\0';
			} else { dothis[0] = '\0'; }
			break;

		case 2: /* CHANGE */
			if (cmpfile->acl[0] != 'z') {
				sprintf(dothis, "%s %s %s", dothis, 
						(random_int(2) == 1) ? "allow" : "deny", 
						acl_perm_t_file_n[random_int(12)]);
				strcpy(cmpfile->acl, dothis);
				sprintf(dothis, "exec chmod =a# 0 \"%s\" %s", cmpfile->acl, cmpfile->name);
			} else { dothis[0] = '\0'; }
			break;
	}

	if (dothis[0] != '\0') {
		//		printf("dothis=%s\n", dothis);
		system(dothis);
	}

	return 1;
}

/* ------------------------------------------------------------------------- */

btbool	cmpfileStat(cmpfile_t *cmpfile, int noPerms)
{
	btbool	isW;
	int			refSize;
	struct stat buf;
	int sret = 0;

	if(trace)
		printf("stating file %s\n", cmpfile->name);
	isW = fileIsWritable(cmpfile->refFile);
	refSize = fileSize(cmpfile->refFile);
	if (cmpfile->isOpen) {
		sret = fstat(cmpfile->fd, &buf);
	} else {
		sret = stat(cmpfile->name, &buf);
	}	
	if(sret < 0){
		perr(errno, "error stating file %s\n", cmpfile->name);
		return 0;
	}else{
		if(buf.st_size != refSize){
			perr(0, "file %s %sstats with wrong size (%d instead of %d)\n", cmpfile->name, cmpfile->isOpen ? "f" : "",(int)buf.st_size, refSize);
			return 0;
		}
		if(((buf.st_mode & 0200) != 0) != isW){
			if (!noPerms) {
				perr(0, "file %s %sstats with wrong write protection status (0%o)\n", cmpfile->name, cmpfile->isOpen ? "f" : "", buf.st_mode);
				return 0;
			}
		}
	}
	return 1;
}

/* ------------------------------------------------------------------------- */

btbool  cmpfileUnlink(cmpfile_t *cmpfile) {
        int rval = 0;
        if (trace)
                printf("Unlinking %s\n", cmpfile->name);
        rval = unlink(cmpfile->name);
        if (rval < 0) {
                perr(errno, "error unlinking file %s\n", cmpfile->name);
                return 0;
        }
        return 1;
}
