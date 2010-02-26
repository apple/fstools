/*
 * Name: cmpdir.h
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

/* ------------------------------------------------------------------------- */

#include "basictypes.h"
#include <sys/acl.h>

//char *acl_perm_dir_n[] = { "list", "add_file", "search", "delete", "add_subdirectory", "delete_child", "readattr", "writeattr", "readextattr", "writeextattr", "readsecurity", "writesecurity", "chown" };
//int acl_perm_t_dir_r[] = { ACL_LIST_DIRECTORY, ACL_ADD_FILE, ACL_SEARCH, ACL_DELETE, ACL_ADD_SUBDIRECTORY, ACL_DELETE_CHILD, ACL_READ_ATTRIBUTES, ACL_WRITE_ATTRIBUTES, ACL_READ_EXTATTRIBUTES, ACL_WRITE_EXTATTRIBUTES, ACL_READ_SECURITY, ACL_WRITE_SECURITY, ACL_CHANGE_OWNER };

//int acl_perm_t_ALL[] = { ACL_READ_DATA, ACL_LIST_DIRECTORY, ACL_WRITE_DATA, ACL_ADD_FILE, ACL_EXECUTE, ACL_SEARCH, ACL_DELETE, ACL_APPEND_DATA, ACL_ADD_SUBDIRECTORY, ACL_DELETE_CHILD, ACL_READ_ATTRIBUTES, ACL_WRITE_ATTRIBUTES, ACL_READ_EXTATTRIBUTES, ACL_WRITE_EXTATTRIBUTES, ACL_READ_SECURITY, ACL_WRITE_SECURITY, ACL_CHANGE_OWNER };
//char *acl_d[] = { "delete", "readattr", "writeattr", "readextattr", "writeextattr", "readsecurity", "writesecurity", "chown", "search", "add_file", "add_subdirectory", "delete_child", "file_inherit", "directory_inherit", "limit_inherit", "only_inherit" };



#define	CMPDIR_MAX_FILES	10



typedef struct cmpdir{
	char	*name;
	struct{
		char	name[16];
		int		fd;
		int		filePattern;
	}		files[CMPDIR_MAX_FILES];
}cmpdir_t;

/* ------------------------------------------------------------------------- */

cmpdir_t	*cmpdirNew(char *path);
btbool	cmpdirFree(cmpdir_t *dir);
btbool	cmpdirRename(cmpdir_t *dir, char *newPath, int WinVolume);
btbool	cmpdirChangeFiles(cmpdir_t *dir);
btbool	cmpdirVerifyFiles(cmpdir_t *dir);

/* ------------------------------------------------------------------------- */
