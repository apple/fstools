---
title: fstorture
---

`fstorture` is a filesystem test tool originally written by the Sharity folks. It was taken into the Apple internal test suites some time ago and has been hacked on by various people. It's quite difficult to obtain the original fstorture sources these days and it seems to have fallen out of use, but it's a handy test and find bugs quite regularly.

fstorture performs the following filesystem operations in a reasonable (but still random) order with reasonable parameters and verifies the results:

* `creat()`
* `mkdir()`
* `open()`
* `close()`
* `read()`
* `write()`
* `truncate()`
* `ftruncate()`
* `chmod()`
* `stat()`
* `rename()`
* `unlink()`

This is done in parallel from a given number of processes.

## Commandline Parameters

The basic usage of fstorture is:

    fstorture <root-dir1> <root-dir2> <number of processes> [<options>]

The <root-dir1> and <root-dir2> are the directories where the filesystem test will be performed. There are two root directories to better test cross-directory renames. The root directories must exist and must be empty before fstorture is started. <number of processes> is the number of parallel running processes that will be used for the test. Each process operates on only one file or subdirectory at a time. You should choose this number appropriate for your system. Higher numbers mean more testing but can put a heavy load on the operating system. Good numbers are in the range of 5 to 100, depending on the load your kernel can take. <options> are options that modify the way certain tests are performed or that switch off certain tests.

## Available Options

<dl>

<dt><code>fsync_before_ftruncate</code></dt>
<dd>Some operating systems may allow a truncate operation to "overtake" a write operation. The sequence <code>write(offset=1000, length=1000)</code> <code>truncate(500)</code> may in fact be executed as <code>truncate(500)</code> <code>write(offset=1000, length=1000)</code> which results in a completely different file, of course. To work around this operating system bug, you can define <code>fsync_before_ftruncate</code>. If this parameter is passed to fstorture, it executes an <code>fsync()</code> before every <code>truncate()</code> or <code>ftruncate()</code>.</dd>

<dt><code>no_rename_open_files</code></dt>
<dd>UNIX semantics allow to rename files while they are still open. Reads and writes will go to the same file even after renaming. In the SMB case this option is needed when running against systems that only support the older rename operation.</dd>

<dt><code>no_unlink_open_files</code></dt>
<dd>UNIX semantics allow to delete files while they are still open. In the SMB case this option is needed when running against systems that only support the older delete operation.</dd>

<dt><code>no_test_dirs</code></dt>
<dd>The fstorture test also creates, renames and destroys directories during the test. The directories are populated with open files, even during renames. This is a hard test for the filing system and there are operating systems that introduce structural errors on the hard drive if this test is performed (on the hard drive, of course). If you don't want to perform the directory test, add this option.</dd>

<dt><code>no_stats</code></dt>
<dd>Turn off file permissions checks (POSIX modes). Needed for servers that don't support SMB UNIX extensions.</dd>

<dt><code>windows_volume</code></dt>
<dd>Treat the test volume as a Windows mount volume; don't move items that are open.</dd>

<dt><code>windows98</code></dt>
<dd>Turn off any test that fails when mounting shares off a Windows 95/98/Me server.</dd>

<dt><code>nocache</code></dt>
<dd>Adds <code>fcntl(F_NOCACHE)</code> after open</dd>

<dt><code>acl</code></dt>
<dd>Performs ACL busting</dd>

<dt><code>nosoftlinks</code></dt>
<dd>Disables soft-links</dd>

<dt><code>nohardlinks</code></dt>
<dd>Disables hard-links</dd>

<dt><code>sleep</code></dt>
<dd>Adds sleep between operations</dd>

<dt><code>-v</code></dt>
<dd>Increases verbosity</dd>

<dt><code>-c 100</code></dt>
<dd>Each process will run this many tests</dd>

<dt><code>-t 5h35m18s</code></dt>
<dd>Runs for 5hours 35min 18sec</dd>

</dl>

**Note:** Both count & time may be given; whichever takes longer is used

**Note:** If you specify 4 processes instead of 1, `-c 100` will take 4x as long

## Examples

When running with Windows 95/98/Me use the follow:

    fstorture DIR1 DIR2 16 99999 windows98 nostats

When running with Windows NT/XP/2000/2003 use the follow:

    fstorture DIR1 DIR2 16 99999 windows_volume nostats

When running from a Apple SMB Client with a server that supports the UNIX extensions use the follow:

    fstorture DIR1 DIR2 16
