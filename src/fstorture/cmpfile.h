/*
 * Name: cmpfile.h
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
This class defines a "comparing file". The file class defined herein
implements the usual file operations. All these operations are performed
on a stored reference file (file_t object, type defined by fileemu.h) _and_
on a real file in the filesystem. The results are compared when possible
and errors are reported. The only return value is whether the operation
succeeded without errors (both files returned the same results). The return
value is 1 on success, 0 on failure.
*/

/* ------------------------------------------------------------------------- */

#include "basictypes.h"
#include <sys/acl.h>

#define THREADS 5

//char *acl_perm_t_file_n[] = { "read", "write", "execute", "delete", "append", "readattr", "writeattr", "readextattr", "writeextattr", "readsecurity", "writesecurity", "chown" };
//int acl_perm_t_file_r[] = { ACL_READ_DATA, ACL_WRITE_DATA, ACL_EXECUTE, ACL_DELETE, ACL_APPEND_DATA, ACL_READ_ATTRIBUTES, ACL_WRITE_ATTRIBUTES, ACL_READ_EXTATTRIBUTES, ACL_WRITE_EXTATTRIBUTES, ACL_READ_SECURITY, ACL_WRITE_SECURITY, ACL_CHANGE_OWNER };

//int acl_perm_t_ALL[] = { ACL_READ_DATA, ACL_LIST_DIRECTORY, ACL_WRITE_DATA, ACL_ADD_FILE, ACL_EXECUTE, ACL_SEARCH, ACL_DELETE, ACL_APPEND_DATA, ACL_ADD_SUBDIRECTORY, ACL_DELETE_CHILD, ACL_READ_ATTRIBUTES, ACL_WRITE_ATTRIBUTES, ACL_READ_EXTATTRIBUTES, ACL_WRITE_EXTATTRIBUTES, ACL_READ_SECURITY, ACL_WRITE_SECURITY, ACL_CHANGE_OWNER };


typedef struct cmpfile{
	struct file	*refFile;
	int			fd;
	char		*name;
	const char	*root;
	char		acl[100];
	btbool		isOpen;
}cmpfile_t;

/* ------------------------------------------------------------------------- */

cmpfile_t	*cmpfileNew(char *path, const char *root);
/* Creates a new object. This method also creates the embedded reference
 * file object and creates the file on disk.
 */
void	cmpfileFree(cmpfile_t *cmpfile, btbool hadErr, btbool unlinkme);
/* Frees the object and the embedded reference file. The file on disk is
 * unlinked if 'hadErr' is 0 or renamed to <filename>.error if 'hadErr' is 1.
 */

btbool cmpfileLink(cmpfile_t *cmpfile, char *linkName, int hard);

btbool	cmpfileRename(cmpfile_t *cmpfile, char *newName);
/* Renames the file on disk and stores the new name for further named
 * references to the file.
 */
btbool	cmpfileOpen(cmpfile_t *cmpfile, btbool writable, btbool nocache);
/* Opens the file read/write, if 'writable' is 1, read only if 'writable'
 * is 0.
 */
btbool	cmpfileClose(cmpfile_t *cmpfile);
/* Closes the file.
 */
btbool	cmpfileWrite(cmpfile_t *cmpfile, int offset, int len, void *data);
/* Writes the given data block to the file. The file must be open for write.
 */
btbool	cmpfileRead(cmpfile_t *cmpfile, int offset, int len);
/* Reads the data block at the given position from the file and verifies the
 * result.
 */
btbool	cmpfileTruncate(cmpfile_t *cmpfile, int len);
/* Sets the file size to the given size.
 */
btbool	cmpfileSetMode(cmpfile_t *cmpfile, btbool writable);
/* Changes the write protection mode of the file.
 */

btbool  cmpfileAcls_do();
btbool  cmpfileAcls(cmpfile_t *cmpfile);

btbool	cmpfileStat(cmpfile_t *cmpfile, int noPerms);
/* Compares write protection status and size of the "real" file and the
 * reference file.
 */
btbool	cmpfileUnlink(cmpfile_t *cmpfile);

/* ------------------------------------------------------------------------- */
