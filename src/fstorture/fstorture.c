/*
 * Name: fstorture.c
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
#include "cmpdir.h"
#include "fileemu.h"
#include "util.h"
#ifdef XILOG
#include <XILog/XILog.h>
#endif /* XILOG */

#define VERS "2.1-pfh"

// Global variables
uuid_t *uuid=NULL;
pthread_t thr[THREADS];
extern void *thr_start();
pthread_mutex_t mutex_start;
pthread_cond_t cond_start;

/* ------------------------------------------------------------------------- */
/* ------------------------ commandline parameters ------------------------- */
/* ------------------------------------------------------------------------- */
static btbool	bugFtruncateReadonly = 0;
static btbool	bugFtruncateSync = 0;
static btbool	noRenameOpenFiles = 0;
static btbool	noUnlinkOpenFiles = 0;
static btbool	noTestDirs = 0;
static btbool	WinVolume = 0;
static btbool	noStats = 0;
static btbool	noPerms = 0;
static btbool	softlinks = 1;
static btbool	hardlinks = 1;
static btbool	sleepy = 0;
static btbool	acl = 0;
static btbool	nocache = 0;
static char	*root1;	/* directories where to perform test */
static char	*root2;
static int	count = 0; // how many to do
static int	mycount = 0; // how many we've done so far
static int 	gStopTime = 0;
pid_t		parent_pid = 0;
int		trace = 0;

#ifdef XILOG
static XILogRef gLogRef;
char* gLogPath;
Boolean gXML = false;
Boolean gEcho = false;
#endif //XILOG

int parsetime(char *t) {
	int i = 0;
	int secs = 0;
	char b[128]; bzero(b, 128);
	
	for (i=0; i < strlen(t); i++) {
		switch (t[i]) {
			case 's':
				secs += atoi(b);
				bzero(b, 128);
				break;
			case 'm':
				secs += atoi(b) * 60;
				bzero(b, 128);
				break;
			case 'h':
				secs += atoi(b) * 60 * 60;
				bzero(b, 128);
				break;
			case 'd':
				secs += atoi(b) * 60 * 60 * 24;
				bzero(b, 128);
				break;
			case 'w':
				secs += atoi(b) * 60 * 60 * 24 * 7;
				bzero(b, 128);
				break;
			case 'y':
				secs += atoi(b) * 60 * 60 * 24 * 365;
				bzero(b, 128);
				break;
			default:
				sprintf(b, "%s%c", b, t[i]);
		} 
	}
	if (secs == 0) // maybe they just gave us a number?
		secs = atoi(t);
	return(secs);
}

/* ------------------------------------------------------------------------- */
/* ------------------- random number generator for tests ------------------- */
/* ------------------------------------------------------------------------- */

struct random_status{
	int		i;
	real_t	u[97];
	real_t	c;
};

struct random_status	random_status;

/* ------------------------------------------------------------------------- */

real_t	random_get(void)	/* 3.56 microseconds execution-time on 486/66 */
{
real_t	r;

	r = random_status.u[random_status.i]
						- random_status.u[(random_status.i + 33) % 97];
	if(r < 0)
		r += 1;
	random_status.u[random_status.i--] = r;
	if(random_status.i < 0)
		random_status.i = 96;
	random_status.c = random_status.c - 7654321./16777216.;
	if(random_status.c < 0)
		random_status.c += 16777213./16777216.;
	r -= random_status.c;
	if(r < 0)
		r += 1;
	return r;
}

/* ------------------------------------------------------------------------- */

void	random_init(int na1, int na2, int na3, int nb1)
{
int		i, j, mat;
real_t	s, t;

	random_status.i = 96;
	for(j = 0; j < 97; j++){
		s = 0;
		t = 0.5;
		for(i = 0; i < 24; i++){
			mat = (((na1 * na2) % 179) * na3) % 179;
			na1 = na2;
			na2 = na3;
			na3 = mat;
			nb1 = (53 * nb1 + 1) % 169;
			if((nb1 * mat % 64) >= 32)
				s += t;
			t /= 2;
		
		}
		random_status.u[j] = s;
	}
	random_status.c  = 362436./16777216.;
}

/* ------------------------------------------------------------------------- */

int		random_int(int exclusive_range)
{
int	r = (int)(random_get() * exclusive_range);

	if(r < 0 || r >= exclusive_range){
		printf("random number %d illegal\n", r);
		exit(1);
	}
	return r;
}

/* ------------------------------------------------------------------------- */

/* This is not a real gaussian random number generator. Don't use it for
 * something where the probability distribution really counts! It is
 * shaped after what is mathematically easy and what the application
 * demands.
 */
real_t	random_gauss(real_t stdval, real_t maxRange)
{
real_t	val, x, f, m, a, b, h;

	a = 10; b = 2;
	m = M_PI/2 - 1 + a + a/b; /* mean value of distribution */
	f = stdval / m;
	for(;;){
		x = random_get();
		h = sqrt(1 - x*x);
		if(h == 0)
			continue;
		val = a * pow(x, 1/b) + 1/h - 1;
		val *= f;
		if(val < maxRange)
			return val;
	}
}

void	random_block(void *block, int len)
{
int		i;
btu8	*p = block;

	for(i=0;i<len;i++){
		*p++ = random_int(256);
	}
}

static char	*charTable = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-";

void	random_name(char *name, int len, int prefix)
{
	int		i,j;
#define howmanyt 5
	int rndnum=0;
	int tstrlen=0;
	char *dtypes[howmanyt];

	dtypes[0] = ".app";
	dtypes[1] = ".mp3";
	dtypes[2] = ".mov";
	dtypes[3] = ".rtf";
	dtypes[4] = ".html";

	/* add an one-digit prefix to avoid name duplication */
	if (prefix >= 10 || prefix < 0) {
		perr(0, "CMPDIR_MAX_FILES and prefix must be no larger than 10");
#if XILOG
		XILogEndTestCase(gLogRef, kXILogTestPassOnErrorLevel);
		XILogCloseLog(gLogRef);
#endif /* XILOG */
		exit(1);
	}
	name[0] = '0' + prefix;

	for(i=1;i<len;i++){
		name[i] = charTable[random_int(strlen(charTable))];
	}
	name[i] = 0;

	/* PFH - Now, let's add an extension */
	rndnum = random_int(howmanyt);
	tstrlen = strlen(dtypes[rndnum]);
	for (j = 1; j <= tstrlen; j++) {
		name[i-j] = dtypes[rndnum][tstrlen-j];
	}
}

const char * random_root()
{
	return (random_int(2) ? root1 : root2);
}

void	random_path(char *path, int id, int running, const char *root)
{
char	rname[16];

	random_name(rname, 8, 0);
	sprintf(path, "%s/%02d-%03d-%s", root, id, running, rname);
}

/* ------------------------------------------------------------------------- */
/* --------------------- the test's main functionality --------------------- */
/* ------------------------------------------------------------------------- */


void perr(int e, char *s, ...) { // e is errno, and s is the string
	int p;
	va_list argp;
	struct tm *newtime;
	time_t aclock;
	char msg[1024]; bzero(msg, 1024);
	char msg2[1024]; bzero(msg2, 1024);

	time( &aclock );                 /* Get time in seconds */
	newtime = localtime( &aclock );  /* Convert time to struct */

	if (e == 0)  // no error given
		sprintf(msg, "%s %s", asctime(newtime), s);
	else
		sprintf(msg, "%s %s: %s", asctime(newtime), s, strerror(e));

	// shoulda used asprintf, except for stripping \n's out
	va_start(argp, s); // Let's reimplement printf!
	for (p = 0; msg[p] != '\0'; p++) {
		if(msg[p] == '\n')
			continue;
		if(msg[p] != '%') {
			sprintf(msg2, "%s%c", msg2, msg[p]);
			continue;
		}
		switch(msg[++p]) {
			case 'c':
				sprintf(msg2, "%s%c", msg2, va_arg(argp, int));
				break;
			case 'd':
				sprintf(msg2, "%s%d", msg2, va_arg(argp, int));
				break;
			case 'o':
				sprintf(msg2, "%s%o", msg2, va_arg(argp, int));
				break;
			case 's':
				sprintf(msg2, "%s%s", msg2, va_arg(argp, char *));
				break;
			case 'x':
				sprintf(msg2, "%s%x", msg2, va_arg(argp, int));
				break;
			case '%':
				sprintf(msg2, "%s%%", msg2);
				break;
		}
	}
	va_end(argp);

	printf("%s\n", msg2);

#ifdef XILOG
	XILogErr(msg2);	
#endif /* XILOG */
}



static btbool	randomOpenOp(cmpfile_t *f, btbool w, int id, int running)
{
btbool	rval = 1;
int		offset, len;
void	*block;
char	path[1024];

	
	switch(random_int(11)){
	case 0:	/* read */
	case 1:	/* read */
		offset = random_int(30000) + random_gauss(10, 100000);
		len = 1 + random_gauss(20000, 100000);
		rval = cmpfileRead(f, offset, len);
		break;
	case 2:	/* write */
	case 3:	/* write */
		if(w){
			offset = random_int(30000) + random_gauss(10, 100000);
			len = 1 + random_gauss(30000, 500000);
			block = malloc(len);
			random_block(block, len);
			rval = cmpfileWrite(f, offset, len, block);
			free(block);
		}
		break;
	case 4:	/* truncate */
		if(bugFtruncateSync)
			fsync(f->fd);
		if(!random_int(10)){	/* do much less truncates than rest */
			if(!bugFtruncateReadonly || (w && fileIsWritable(f->refFile))){
				rval = cmpfileTruncate(f, random_int(50000));
			}
		}
		break;
	case 5:	/* chmod */
		rval = cmpfileSetMode(f, random_int(3) != 0);
		break;
	case 6:	/* stat */
		if (!noStats) { // if the noStats flag ISN'T set
			rval = cmpfileStat(f, noPerms);
		}
		break;
	case 7:	/* sleep */ 
		if (sleepy) {
			if(!random_int(4)){
				usleep(1000000 * random_gauss(1, 200));
			}
		}
		break;
	case 8:	/* rename */
		if(!noRenameOpenFiles) {
			random_path(path, id, running, (WinVolume) ? f->root : random_root());
			rval = cmpfileRename(f, path);
		}
		break;
	case 9: /* acls */
		if (acl) {
			if(!random_int(5)) {	// do less ACL busting
				rval = cmpfileAcls(f);
			}
		}
		break;
	case 10: /* link */
		if (!random_int(2)) {
			if (softlinks) {
				random_path(path, id, running, random_root());
				rval = cmpfileLink(f, path, 0); // Soft link
			}
		} else {
			if (hardlinks) {
				random_path(path, id, running, random_root());
				rval = cmpfileLink(f, path, 1); // Hard link
			}	
		}
		break;
	}
	return rval;
}

/* ------------------------------------------------------------------------- */

static btbool	randomClosedOp(cmpfile_t *f, int id, int running)
{
btbool	rval = 1;
char	path[1024];
const char *root;

	switch(random_int(7)){
		case 0:	/* truncate */
			if(!random_int(10)){	/* do much less truncates than rest */
				if(fileIsWritable(f->refFile)){
					rval = cmpfileTruncate(f, random_int(50000));
				}
			}
			break;
		case 1:	/* chmod */
			rval = cmpfileSetMode(f, random_int(3) != 0);
			break;
		case 2:	/* stat */
			if (!noStats) { // if the noStats flag ISN'T set
				rval = cmpfileStat(f, noPerms);
			}
			break;
		case 3:	/* sleep */
			if (sleepy) {
				if(!random_int(4)){
					usleep(1000000 * random_gauss(1, 200));
				}
			}
			break;
		case 4: /* rename */
			root = random_root();
			random_path(path, id, running, root);
			rval = cmpfileRename(f, path);
			if (rval)	/* Rename sucessed so reset the root */
				f->root = root;
			break;
		case 5: /* acls */
			if(!random_int(5)){   // do less ACL busting than the rest
				if (acl)
					rval = cmpfileAcls(f);
			}
			break;
		case 6: /* link */
			if (!random_int(2)) {
				if (softlinks) {
					random_path(path, id, running,  random_root());
					rval = cmpfileLink(f, path, 0); // Soft link
				}
			} else {
				if (hardlinks) {
					random_path(path, id, running, random_root());
					rval = cmpfileLink(f, path, 1); // Hard link
				}	
			}
			break;
	}
	return rval;
}

/* ------------------------------------------------------------------------- */

static btbool   randomUnlinkOp(cmpfile_t *f, btbool w, int id, int running)
{
btbool  rval = 1;
int             offset, len;
void    *block;


        switch(random_int(5)){
        case 0: /* read */
                offset = random_int(30000) + random_gauss(10, 100000);
                len = 1 + random_gauss(20000, 100000);
                rval = cmpfileRead(f, offset, len);
                break;
        case 1: /* write */
                if(w){
                        offset = random_int(30000) + random_gauss(10, 100000);
                        len = 1 + random_gauss(30000, 500000);
                        block = malloc(len);
                        random_block(block, len);
                        rval = cmpfileWrite(f, offset, len, block);
						free(block);
                }
                break;

        case 2: /* ftruncate */
                if(bugFtruncateSync)
                        fsync(f->fd);
                if(!bugFtruncateReadonly || (w && fileIsWritable(f->refFile))){
                	rval = cmpfileTruncate(f, random_int(50000));
                }
                break;
        case 3: /* fchmod */
                rval = cmpfileSetMode(f, random_int(3) != 0);
                break;
        case 4: /* fstat */
                if (!noStats) { // if the noStats flag ISN'T set
                        rval = cmpfileStat(f, noPerms);
				}
                break;
        }
        return rval;
}

/* ------------------------------------------------------------------------- */


static void	testOneFile(int id, int running)
{
char		path[1024];
cmpfile_t	*f;
int			w, i, l, i1, l1;
btbool		hadErr = 1;
const char		*root = random_root();

	random_path(path, id, running, root);
	f = cmpfileNew(path, root);
	l1 = random_gauss(50,5000);
	for(i1=0;i1<l1;i1++){

	// Perform open file operations
		w = random_int(3);
		if(!fileIsWritable(f->refFile))
			w = 0;
		w = w != 0;
		if(!cmpfileOpen(f, w, nocache))
			goto errorOccured;
		l = random_gauss(20, 5000);
		for(i=0;i<l;i++){
			if(!randomOpenOp(f, w, id, running))
				goto errorOccured;
		}
		if(!cmpfileClose(f))
			goto errorOccured;

	// Perform closed file operations
		if(random_int(2)){
			l = random_gauss(20, 500);
		}else{
			l = random_int(5);
		}
		for(i=0;i<l;i++){
			if(!randomClosedOp(f, id, running))
				goto errorOccured;
		}
	}

	hadErr = 0;
errorOccured:
	cmpfileFree(f, hadErr, 1);
}


static void     testOneUnlinkedFile(int id, int running)
{
char            path[1024];
cmpfile_t       *f;
int                     w, i, l, i1, l1;
btbool          hadErr = 1;
const char		*root = random_root();

		if (noUnlinkOpenFiles)	/* Not supported */
			return;
        random_path(path, id, running, root);
        f = cmpfileNew(path, root);
        l1 = random_gauss(50,5000);

        // Perform unlinked file operations
        w = random_int(3);
        if(!fileIsWritable(f->refFile))
                w = 0;
        w = w != 0;
        if(!cmpfileOpen(f, w, nocache))
                goto errorOccured;
        if(!cmpfileUnlink(f))
                goto errorOccured;
        for(i1=0;i1<l1;i1++){
                l = random_gauss(20, 5000);
                for(i=0;i<l;i++){
					if(!randomUnlinkOp(f, w, id, running)) 
						goto errorOccured;
                }
        }

        hadErr = 0;
errorOccured:
        cmpfileFree(f, hadErr, 0);
}

/* ------------------------------------------------------------------------- */

static btbool	randomDirOp(cmpdir_t *d, int id, int running)
{
btbool	rval = 1;
char	path[1024];

	switch(random_int(4)){
	case 0:	/* rename */
			random_path(path, id, running, random_root());
			rval = cmpdirRename(d, path, WinVolume);
		break;
	case 1:	/* change files */
		rval = cmpdirChangeFiles(d);
		break;
	case 2:	/* verify files */
		rval = cmpdirVerifyFiles(d);
		break;
	case 3:	/* sleep */
		if (sleepy) {
			if(!random_int(8)){
				usleep(1000000 * random_gauss(1, 200));
		}
			}
	}
	return rval;
}

/* ------------------------------------------------------------------------- */

static void	testOneDirectory(int id, int running)
{
char		path[1024];
cmpdir_t	*d;
int			i, l;
btbool		hadErr = 1;

	random_path(path, id, running, random_root());
	d = cmpdirNew(path);
	if(d == NULL){
		printf("Sleeping 1sec\n");
		sleep(1);
		return;
	}
	l = random_gauss(150, 5000);
	for(i=0;i<l;i++){
		if(!randomDirOp(d, id, running))
			goto errorOccured;
	}
	hadErr = 0;
errorOccured:
	cmpdirFree(d);
}

/* ------------------------------------------------------------------------- */

void    testproc(int id, int isParent, int n)
{
	int             i = 0;

	if (isParent)
		parent_pid = getpid();
	random_init(11 + id, 23, 234 + id, 78);

	if ((gStopTime != 0) || (count != 0)) { // We are timing/counting this run
		if (gStopTime != 0) 
			gStopTime += (isParent ? 5 : 0); // parent quits 5sec after children
		while((time(NULL) < gStopTime) || (count > 0)) { // whichever is LONGER is run
			if(noTestDirs || random_int(7)) {
				testOneFile(id, i);
				testOneUnlinkedFile(id, i);
			} else
				testOneDirectory(id, i);
			i++;
			count -= n;
			mycount += n;
#ifdef XILOG
			if (isParent)
				if ((i*n % 100) == 0)
					XIPing(i);
#endif /* XILOG */
		}

	} else { // Just run indefinitely
		for(i=0;;i++){
#ifdef XILOG
			if (isParent)
				if ((i*n % 100) == 0)
					XIPing(mycount);
#endif /* XILOG */
			if(noTestDirs || random_int(7)){
				testOneFile(id, i);
				testOneUnlinkedFile(id, i);
			}else{
				testOneDirectory(id, i);
			}
			mycount += n;
		}

	}
	if (isParent) {
		printf("Test completed [fstorture %s]: ", VERS); fflush(NULL);
		system("date");
	}
}


/* ------------------------------------------------------------------------- */
void usage(char *argv) {
	fprintf(stderr, "fstorture %s\n", VERS);
	fprintf(stderr, "usage: %s <root dir1> <root dir2> <number of processes> [options]\n", argv);
	fprintf(stderr, "available options are:\n");
	fprintf(stderr, "   fsync_before_ftruncate\n");
	fprintf(stderr, "   no_rename_open_files\n");
	fprintf(stderr, "   no_unlink_open_files\n");
	fprintf(stderr, "   no_test_dirs\n");
	fprintf(stderr, "   no_stats\n");
	fprintf(stderr, "   no_perms\n");
	fprintf(stderr, "   windows_volume\n");
	fprintf(stderr, "   windows98\n");
	fprintf(stderr, "   nocache     Adds fcntl(F_NOCACHE) after open\n");
	fprintf(stderr, "   acl         Performs ACL busting\n");
	fprintf(stderr, "   nosoftlinks Disables soft-links\n");
	fprintf(stderr, "   nohardlinks Disables hard-links\n");
	fprintf(stderr, "   sleep       Adds sleep between operations\n");
	fprintf(stderr, "   -v          Increases verbosity\n");
	fprintf(stderr, "   -c 100      Each process will run this many tests\n");
	fprintf(stderr, "   -t 5h35m18s Runs for 5hours 35min 18sec\n");
	fprintf(stderr, "   NOTE: Both count & time may be given; whichever takes longer is used\n");
	fprintf(stderr, "   NOTE: If you specify 4 processes instead of 1, -c 100 will take 4x as long\n");
	exit(1);
}

int	main(int argc, char **argv)
{
	int			n, i;
	struct stat sbuf;
	struct volcapbuf {
		u_long buffer_size;
		vol_capabilities_attr_t caps;
	};
	struct attrlist alist;
	struct volcapbuf buf;
	struct statfs st;
	int dirs = 0;

	if(argc < 4)
		usage(argv[0]);
	root1 = argv[1];
	root2 = argv[2];
	if(stat(root1, &sbuf) != 0){
		fprintf(stderr, "root1 = %s does not exist\n", root1);
		exit(1);
	}
	if(stat(root2, &sbuf) != 0){
		fprintf(stderr, "root2 = %s does not exist\n", root2);
		exit(1);
	}
	if(!isdigit(argv[3][0])){
		fprintf(stderr, "<number of processes> must be a number, not %s!\n", argv[3]);
		usage(argv[0]);
	}
	n = atoi(argv[3]); // number of processes to spawn
	bugFtruncateReadonly = 1;	/* always on, documentation bug */
	for(i=4;i<argc;i++){
		if(strcmp(argv[i], "fsync_before_ftruncate") == 0){
			bugFtruncateSync = 1;
		}else if(strcmp(argv[i], "no_rename_open_files") == 0){
			noRenameOpenFiles = 1;
		}else if(strcmp(argv[i], "no_unlink_open_files") == 0){
			noUnlinkOpenFiles = 1;			
		}else if(strcmp(argv[i], "no_test_dirs") == 0){
			noTestDirs = 1;
		}else if(strcmp(argv[i], "windows_volume") == 0){
			/* 
			 * If Leopard or greater then this flag is not 
			 * required.
			 */
			WinVolume = 1;
			noPerms = 1;
		}else if(strcmp(argv[i], "windows98") == 0){
			noUnlinkOpenFiles = 1;
			noRenameOpenFiles = 1;
			WinVolume = 1;
			noPerms = 1;
		}else if((strcmp(argv[i],"no_stats")==0)||strcmp(argv[i],"nostats")==0) {
			noStats = 1;
		}else if((strcmp(argv[i],"no_perms")==0)||strcmp(argv[i],"noperms")==0) {
			noPerms = 1;
		} else if(strcmp(argv[i], "nocache") == 0) {
			nocache = 1;
		} else if(strcmp(argv[i], "acl") == 0) {
			acl = 1;
		} else if(strcmp(argv[i], "nosoftlinks") == 0) {
			softlinks = 0;
		} else if(strcmp(argv[i], "nohardlinks") == 0) {
			hardlinks = 0;
		} else if(strcmp(argv[i], "-v") == 0) {
			trace = 1;
		} else if((strcmp(argv[i],"count")==0)||(strcmp(argv[i],"-c")==0)) {
			if (argv[++i]) 
				count = atoi(argv[i]);
                } else if((strcmp(argv[i],"time")==0)||(strcmp(argv[i],"-t")==0)) {
			if (argv[i+1]) {
				gStopTime = parsetime(argv[++i]);
				if (gStopTime == 0) {
					printf("-t needs a proper time argument!\n");
					exit(-1);
				}
				gStopTime += time(NULL);
			} else {
				printf("-t needs a time argument!\n");
				exit(-1);
			}
		}else if(strcmp(argv[i], "sleep") == 0) {
			sleepy = 1;
		}
#ifdef XILOG
		else {
			if(strcmp(argv[i], "-x") == 0) {
				gXML = true;
			}
			else if(strcmp(argv[i], "-e") == 0) {
				gEcho = true;
			}
			else if(strcmp(argv[i], "-l") == 0) {
				gLogPath = argv[++i];
			}
#endif /* XILOG */
			else {
				if ((i == 4) && (isdigit(argv[4][0]))) { // for legacy pid
					continue;
				} else {
					fprintf(stderr, "option ->%s<- not known\n", argv[i]);
					exit(1);
				}
			}
#ifdef XILOG
		}
#endif /* XILOG */
	}


	// Check if root1 and root2 are empty directories
	DIR* dirp;
	struct dirent *dp;
	dirs = 0;
	dirp = opendir(root1);
	while ((dp = readdir(dirp)) != NULL)
		dirs++;
	(void)closedir(dirp);
	if (dirs != 2)
		printf("--> WARNING: %s should be an EMPTY DIRECTORY <-- \n", root1);

	dirs = 0;
	dirp = opendir(root2);
	while ((dp = readdir(dirp)) != NULL)
		dirs++;
	(void)closedir(dirp);
	if (dirs != 2)
		printf("--> WARNING: %s should be an EMPTY DIRECTORY <--\n", root2);


	// Check if soft/hard links & ACLs are supported
	alist.bitmapcount = 5;
	alist.reserved = 0;
	alist.commonattr = 0;
	alist.volattr = ATTR_VOL_INFO | ATTR_VOL_CAPABILITIES;
	alist.dirattr = 0;
	alist.fileattr = 0;
	alist.forkattr = 0;

	if (statfs(root1, &st) < 0) {
		perr(errno, "statfs");
		exit(-1);
	}
	if (getattrlist(st.f_mntonname, &alist, &buf, sizeof buf, 0) < 0) {
		perr(errno, "getattrlist");
		exit(-1);
	}
	printf("Capabilities of %s (on %s): softlinks?", root1, st.f_mntonname);
	if (buf.caps.capabilities[VOL_CAPABILITIES_FORMAT] & VOL_CAP_FMT_SYMBOLICLINKS) {
		printf("Y");
	} else {
		printf("N");
		softlinks = 0;
	}

	printf(" hardlinks?");
	if (buf.caps.capabilities[VOL_CAPABILITIES_FORMAT] & VOL_CAP_FMT_HARDLINKS) {
		printf("Y");
	} else {
		printf("N");
		hardlinks = 0;
	}
        printf(" ACLs?");
        if(buf.caps.capabilities[VOL_CAPABILITIES_INTERFACES]&VOL_CAP_INT_EXTENDED_SECURITY){
                printf("Y");
        } else {
                printf("N");
                acl = 0;
        }
	printf("\n");

	if (statfs(root1, &st) < 0) {
		perr(errno, "statfs");
		exit(-1);
	}
	if (getattrlist(st.f_mntonname, &alist, &buf, sizeof buf, 0) < 0) {
		perr(errno, "getattrlist");
		exit(-1);
	}
	printf("Capabilities of %s (on %s): softlinks?", root2, st.f_mntonname);
	if (buf.caps.capabilities[VOL_CAPABILITIES_FORMAT] & VOL_CAP_FMT_SYMBOLICLINKS) {
		printf("Y");
	} else {
		printf("N");
		softlinks = 0;
	}
	printf(" hardlinks?");

	if (buf.caps.capabilities[VOL_CAPABILITIES_FORMAT] & VOL_CAP_FMT_HARDLINKS) {
		printf("Y");
	} else {
		printf("N");
		hardlinks = 0;
	}
	printf(" ACLs?");
        if(buf.caps.capabilities[VOL_CAPABILITIES_INTERFACES]&VOL_CAP_INT_EXTENDED_SECURITY){
                printf("Y");
        } else {
                printf("N");
                acl = 0;
        }
	printf("\n");



	if (acl) {
		// Now, put user "www"'s UUID into the global variable "uuid"
		char *u = "www";
		struct passwd *tpass = NULL;
		//uuid_t *uuid=NULL; // this was defined globally
		if (NULL == (uuid = (uuid_t *)calloc(1,sizeof(uuid_t))))
			perr(errno, "unable to allocate a uuid");
		tpass = getpwnam(u);
		if (tpass) {
			if (0 != mbr_uid_to_uuid(tpass->pw_uid, *uuid)) {
				perr(errno, "mbr_uid_to_uuid(): Unable to translate");
			}
		}

		// Next, make our threads
		pthread_mutex_init(&mutex_start, NULL);
		pthread_cond_init (&cond_start,  NULL);

		for (i=0;i<THREADS;i++){
			pthread_create(&thr[i], NULL, thr_start, NULL);
		}
	}

#ifdef XILOG
	char argline[1024]; bzero(argline, 1024);
	strcpy(argline, "Arguments: ");
	int j;
	for (j=1; j<argc; j++) {
		sprintf(argline, "%s %s", argline, argv[j]);
	}
	gLogRef = XILogOpenLogExtended(gLogPath, "fstorture", "com.apple.xintegration", NULL, gXML,gEcho, NULL,"ResultOwner","com.apple.fstorture",NULL);
	if(gLogRef == NULL) {
		fprintf(stderr, "Couldn't create log for path: %s\n", gLogPath);
		exit(1);
	}
	XILogEnableMultithreaded(gLogRef);
	XILogBeginTestCase(gLogRef, argline, "Test random file operations");
	XILogMsg("Test started [fstorture %s]", VERS);
#endif /* XILOG */


	printf("Test started   [fstorture %s]: ", VERS); fflush(NULL);
	system("date");
	if (bugFtruncateSync)
		printf("Forcing fsync before ftruncate\n");
	if (noRenameOpenFiles)
		printf("Disabling rename of open files\n");
	if (noTestDirs)
		printf("Not using test directories\n");
	if (WinVolume)
		printf("Running test on a Windows Volume\n");
	if (noStats)
		printf("Disabling stats (and file permissions checks)\n");
	if (noPerms)
		printf("Disabling file permissions checks\n");
	if (nocache)
		printf("Setting fcntl(F_NOCACHE) after open\n");
	if (acl)
		printf("Running ACL busting\n");
	if (softlinks==0)
		printf("Disabling soft links\n");
	if (hardlinks==0)
		printf("Disabling hard links\n");
	if (count != 0)
		printf("Running for %d cycle%s\n", count, (count >= 2 ) ? "s" : "");
	if (gStopTime != 0)
		printf("Running for %d seconds\n", gStopTime - (int)time(NULL));
	if (sleepy)
		printf("Throwing in random sleeps\n");

	signal(SIGHUP,  cleanup);
	signal(SIGINT,  cleanup);
	signal(SIGPIPE, cleanup);
	signal(SIGALRM, cleanup);
	signal(SIGTERM, cleanup);
	signal(SIGXCPU, cleanup);
	signal(SIGXFSZ, cleanup);
	signal(SIGVTALRM,cleanup);
	signal(SIGUSR1, cleanup);
	signal(SIGUSR2, cleanup);

#ifdef XILOG
	XIPing(0);
#endif /* XILOG */

	for(i=0;i<n-1;i++){ // n is the number of processes (argv[3])
		if(fork() == 0){        /* we are the child */
			testproc(i, 0, n);
			exit(0);
		}
	}
	testproc(i, 1, n);            /* the last process is the parent */

#ifdef XILOG
	XILogEndTestCase(gLogRef, kXILogTestPassOnErrorLevel);
	XILogCloseLog(gLogRef);
#endif /* XILOG */

	return 0;
}

/* ------------------------------------------------------------------------- */

void cleanup(int sig) {
#ifdef XILOG
	XILogEndTestCase(gLogRef, kXILogTestPassOnErrorLevel);
	XILogCloseLog(gLogRef);
#endif /* XILOG */
	if (getpid() == parent_pid) {
		if (sig)
			printf("<- Got signal %d\n", sig);
		printf("testcall cycles = %d\n", mycount);
/* Test case is closed
#ifdef XILOG
		if (sig)
			XILogMsg("signal %d\n", sig);
		XILogMsg("testcall cycles = %d\n", mycount);
#endif
*/
	}
	exit(0);
}
