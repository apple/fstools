---
title: fsx
---

fsx is a filesystem exerciser that is used to test both local and network filesystems.

Dave Jones has written a useful [history of fsx](http://www.codemonkey.org.uk/projects/fsx/) and gathered a collection of fsx variants. This version of fsx is the original fsx source that was released in 2001 with additional enhancements from various Apple folks.

This release adds the following options:

<dl class="dl-horizontal">

<dt><code>-d duration</code></dt>
<dd>number of hours for the tool to run</dd>

<dt><code>-e</code></dt>
<dd>tests using an extended attribute rather than a file</dd>

<dt><code>-f forkname</code></dt>
<dd>test the named fork of fname</dd>

<dt><code>-g logpath</code></dt>
<dd>path for .fsxlog file</dd>

<dt><code>-h</code></dt>
<dd>write 0s instead of creating holes (i.e. sparse file)</dd>

<dt><code>-i</code></dt>
<dd>interactive mode, hit return before performing the current operation</dd>

<dt><code>-l logpath</code></dt>
<dd>path for XILog file</dd>

<dt><code>-v</code></dt>
<dd>debug output for all operations</dd>

<dt><code>-x</code></dt>
<dd>write output in XML (XILOG)</dd>

<dt><code>-y</code></dt>
<dd>call fsync before closing the file</dd>

<dt><code>-C</code></dt>
<dd>mix cached and un-cached read/write ops</dd>

<dt><code>-G logsize</code></dt>
<dd>#entries in oplog (default 1024)</dd>

<dt><code>-F flen</code></dt>
<dd>the upper bound on file size (default 262144)</dd>

<dt><code>-I</code></dt>
<dd>start interactive mode since operation opnum</dd>

<dt><code>-M</code></dt>
<dd>slow motion mode: wait 1 second before each op</dd>

<dt><code>-R</code></dt>
<dd>mapped read operations DISabled</dd>

<dt><code>-T datasize</code></dt>
<dd>size of atomic data elements written to file [1,2,4] (default 4)</dd>

</dl>
