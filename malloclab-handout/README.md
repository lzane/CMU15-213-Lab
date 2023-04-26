#############################
# CS:APP Malloc Lab
# Handout files for students
#############################

*!important*: `mm-explicit.c` and `mm-segregate.c` are my solution.
## 最终得分

```
Results for mm malloc:
  valid  util   ops    secs     Kops  trace
   yes    86%  100000  0.007480 13369 ./traces/alaska.rep
 * yes    98%    4805  0.000472 10184 ./traces/amptjp.rep
 * yes    85%    4162  0.000263 15843 ./traces/bash.rep
 * yes    77%   57716  0.003613 15973 ./traces/boat.rep
 * yes    97%    5032  0.000489 10286 ./traces/cccp.rep
 * yes    76%   11991  0.000791 15155 ./traces/chrome.rep
 * yes    99%   20000  0.001373 14569 ./traces/coalesce-big.rep
   yes    98%   14400  0.000875 16464 ./traces/coalescing-bal.rep
   yes   100%      15  0.000002  7528 ./traces/corners.rep
 * yes    98%    5683  0.000649  8754 ./traces/cp-decl.rep
 u yes    59%      --        --    -- ./traces/exhaust.rep
 * yes    79%    8000  0.000496 16130 ./traces/firefox.rep
   yes    84%   99804  0.006966 14327 ./traces/firefox-reddit.rep
   yes    87%     118  0.000011 10938 ./traces/hostname.rep
 * yes    93%   19405  0.001456 13324 ./traces/login.rep
 * yes    88%     200  0.000012 17289 ./traces/lrucd.rep
   yes    91%     372  0.000030 12364 ./traces/ls.rep
   yes    84%      10  0.000002  5981 ./traces/malloc.rep
   yes    81%      17  0.000002 10139 ./traces/malloc-free.rep
 u yes   100%      --        --    -- ./traces/needle.rep
 * yes    97%     200  0.000017 12030 ./traces/nlydf.rep
   yes    89%    1494  0.000120 12455 ./traces/perl.rep
 * yes    89%     200  0.000012 16848 ./traces/qyqyc.rep
 * yes    93%    4800  0.000598  8021 ./traces/random.rep
 * yes    90%    4800  0.000623  7706 ./traces/random2.rep
 * yes    94%     147  0.000014 10562 ./traces/rm.rep
 * yes    96%     200  0.000014 14767 ./traces/rulsr.rep
 p yes     --    6495  0.000633 10261 ./traces/seglist.rep
   yes    98%      12  0.000002  5109 ./traces/short2.rep
18 17     89%  153836  0.011524 13349

Comparison with libc malloc: mm/libc = 13349 Kops / 17773 Kops = 0.75
Perf index = 56 (util) & 39 (thru) = 95/100
{"scores": {"Autograded Score": 95}, "scoreboard": [95, 95, 13349, 89]}
```

## 完成explicit list和segregated list

以书中给出的implicit list为基准完成explicit list：
```
Results for mm malloc:
  valid  util   ops    secs     Kops  trace
   yes    77%  100000  0.002237 44704 ./traces/alaska.rep
 * yes    89%    4805  0.000304 15799 ./traces/amptjp.rep
 * yes    71%    4162  0.000066 62801 ./traces/bash.rep
 * yes    56%   57716  0.000846 68217 ./traces/boat.rep
 * yes    92%    5032  0.000210 23988 ./traces/cccp.rep
 * yes    72%   11991  0.000191 62763 ./traces/chrome.rep
 * yes    99%   20000  0.000206 96948 ./traces/coalesce-big.rep
   yes    66%   14400  0.000142101198 ./traces/coalescing-bal.rep
   yes   100%      15  0.000001 18108 ./traces/corners.rep
 * yes    94%    5683  0.000439 12954 ./traces/cp-decl.rep
 u yes    71%      --        --    -- ./traces/exhaust.rep
 * yes    72%    8000  0.000122 65822 ./traces/firefox.rep
   yes    78%   99804  0.007249 13768 ./traces/firefox-reddit.rep
   yes    75%     118  0.000002 66316 ./traces/hostname.rep
 * yes    89%   19405  0.000285 68126 ./traces/login.rep
 * yes    63%     200  0.000002 84063 ./traces/lrucd.rep
   yes    88%     372  0.000005 75383 ./traces/ls.rep
   yes    34%      10  0.000001 17918 ./traces/malloc.rep
   yes    28%      17  0.000001 28962 ./traces/malloc-free.rep
 u yes    99%      --        --    -- ./traces/needle.rep
 * yes    76%     200  0.000004 52244 ./traces/nlydf.rep
   yes    81%    1494  0.000022 67883 ./traces/perl.rep
 * yes    57%     200  0.000003 74877 ./traces/qyqyc.rep
 * yes    88%    4800  0.000575  8345 ./traces/random.rep
 * yes    85%    4800  0.000694  6912 ./traces/random2.rep
 * yes    79%     147  0.000003 52228 ./traces/rm.rep
 * yes    68%     200  0.000002 81005 ./traces/rulsr.rep
 p yes     --    6495  0.000880  7379 ./traces/seglist.rep
   yes    89%      12  0.000002  7802 ./traces/short2.rep
18 17     79%  153836  0.004833 31832

Comparison with libc malloc: mm/libc = 31832 Kops / 17818 Kops = 1.79
Perf index = 36 (util) & 39 (thru) = 75/100
```

完成segregated list：
```
Results for mm malloc:
  valid  util   ops    secs     Kops  trace
   yes    85%  100000  0.007438 13445 ./traces/alaska.rep
 * yes    97%    4805  0.000461 10434 ./traces/amptjp.rep
 * yes    76%    4162  0.000275 15119 ./traces/bash.rep
 * yes    56%   57716  0.002618 22049 ./traces/boat.rep
 * yes    97%    5032  0.000450 11173 ./traces/cccp.rep
 * yes    72%   11991  0.000750 15998 ./traces/chrome.rep
 * yes    99%   20000  0.001345 14865 ./traces/coalesce-big.rep
   yes    65%   14400  0.000880 16356 ./traces/coalescing-bal.rep
   yes   100%      15  0.000002  8581 ./traces/corners.rep
 * yes    98%    5683  0.000571  9960 ./traces/cp-decl.rep
 u yes    71%      --        --    -- ./traces/exhaust.rep
 * yes    75%    8000  0.000495 16164 ./traces/firefox.rep
   yes    80%   99804  0.006434 15512 ./traces/firefox-reddit.rep
   yes    73%     118  0.000010 11632 ./traces/hostname.rep
 * yes    89%   19405  0.001387 13994 ./traces/login.rep
 * yes    61%     200  0.000013 15474 ./traces/lrucd.rep
   yes    88%     372  0.000026 14213 ./traces/ls.rep
   yes    33%      10  0.000002  6536 ./traces/malloc.rep
   yes    27%      17  0.000002  8786 ./traces/malloc-free.rep
 u yes    99%      --        --    -- ./traces/needle.rep
 * yes    88%     200  0.000012 16400 ./traces/nlydf.rep
   yes    81%    1494  0.000109 13676 ./traces/perl.rep
 * yes    84%     200  0.000011 18423 ./traces/qyqyc.rep
 * yes    88%    4800  0.000662  7248 ./traces/random.rep
 * yes    86%    4800  0.000704  6822 ./traces/random2.rep
 * yes    78%     147  0.000013 11284 ./traces/rm.rep
 * yes    67%     200  0.000011 19010 ./traces/rulsr.rep
 p yes     --    6495  0.000522 12443 ./traces/seglist.rep
   yes    89%      12  0.000002  5271 ./traces/short2.rep
18 17     82%  153836  0.010298 14938

Comparison with libc malloc: mm/libc = 14938 Kops / 17096 Kops = 0.87
Perf index = 42 (util) & 39 (thru) = 81/100
```

## 空间优化
### 1. 空闲块不需要footer

```
Results for mm malloc:
  valid  util   ops    secs     Kops  trace
   yes    85%  100000  0.007659 13056 ./traces/alaska.rep
 * yes    97%    4805  0.000435 11040 ./traces/amptjp.rep
 * yes    76%    4162  0.000235 17683 ./traces/bash.rep
 * yes    65%   57716  0.003138 18392 ./traces/boat.rep
 * yes    97%    5032  0.000458 10983 ./traces/cccp.rep
 * yes    74%   11991  0.000718 16709 ./traces/chrome.rep
 * yes    99%   20000  0.001346 14864 ./traces/coalesce-big.rep
   yes    65%   14400  0.000935 15404 ./traces/coalescing-bal.rep
   yes   100%      15  0.000002  8377 ./traces/corners.rep
 * yes    98%    5683  0.000578  9825 ./traces/cp-decl.rep
 u yes    71%      --        --    -- ./traces/exhaust.rep
 * yes    75%    8000  0.000478 16719 ./traces/firefox.rep
   yes    82%   99804  0.006377 15650 ./traces/firefox-reddit.rep
   yes    73%     118  0.000010 11976 ./traces/hostname.rep
 * yes    90%   19405  0.001427 13601 ./traces/login.rep
 * yes    61%     200  0.000011 18548 ./traces/lrucd.rep
   yes    88%     372  0.000026 14371 ./traces/ls.rep
   yes    33%      10  0.000001  7873 ./traces/malloc.rep
   yes    27%      17  0.000002 11241 ./traces/malloc-free.rep
 u yes   100%      --        --    -- ./traces/needle.rep
 * yes    88%     200  0.000012 16216 ./traces/nlydf.rep
   yes    86%    1494  0.000111 13443 ./traces/perl.rep
 * yes    84%     200  0.000011 18046 ./traces/qyqyc.rep
 * yes    88%    4800  0.000664  7229 ./traces/random.rep
 * yes    86%    4800  0.000670  7164 ./traces/random2.rep
 * yes    78%     147  0.000013 11553 ./traces/rm.rep
 * yes    67%     200  0.000011 18501 ./traces/rulsr.rep
 p yes     --    6495  0.000483 13458 ./traces/seglist.rep
   yes    89%      12  0.000002  5267 ./traces/short2.rep
18 17     83%  153836  0.010688 14393

Comparison with libc malloc: mm/libc = 14393 Kops / 18129 Kops = 0.79
Perf index = 44 (util) & 39 (thru) = 83/100
{"scores": {"Autograded Score": 83}, "scoreboard": [83, 83, 14393, 83]}
```

###	2. free list 存offset，不存整个地址
因为heap不会超过2^32，所以free list里可以用4B来存一个offset，而不是8B存一个指针，这样minimal block size可以从6B 降到 4B。
一开始使用判断指针是否为NULL的写法导致throughput下降，后用heap_listp来替代NULL指针

```
Results for mm malloc:
  valid  util   ops    secs     Kops  trace
   yes    86%  100000  0.007673 13033 ./traces/alaska.rep
 * yes    98%    4805  0.000520  9237 ./traces/amptjp.rep
 * yes    83%    4162  0.000258 16114 ./traces/bash.rep
 * yes    77%   57716  0.003554 16239 ./traces/boat.rep
 * yes    96%    5032  0.000451 11154 ./traces/cccp.rep
 * yes    76%   11991  0.000752 15956 ./traces/chrome.rep
 * yes    99%   20000  0.001308 15291 ./traces/coalesce-big.rep
   yes    66%   14400  0.000865 16653 ./traces/coalescing-bal.rep
   yes   100%      15  0.000002  8292 ./traces/corners.rep
 * yes    97%    5683  0.000683  8315 ./traces/cp-decl.rep
 u yes    71%      --        --    -- ./traces/exhaust.rep
 * yes    79%    8000  0.000475 16826 ./traces/firefox.rep
   yes    85%   99804  0.006121 16304 ./traces/firefox-reddit.rep
   yes    74%     118  0.000010 12165 ./traces/hostname.rep
 * yes    93%   19405  0.001327 14624 ./traces/login.rep
 * yes    62%     200 -0.000039 -5193 ./traces/lrucd.rep
   yes    88%     372  0.000029 12941 ./traces/ls.rep
   yes    33%      10  0.000001  7323 ./traces/malloc.rep
   yes    27%      17  0.000001 11435 ./traces/malloc-free.rep
 u yes   100%      --        --    -- ./traces/needle.rep
 * yes    88%     200  0.000015 13201 ./traces/nlydf.rep
   yes    86%    1494  0.000109 13669 ./traces/perl.rep
 * yes    85%     200  0.000010 19699 ./traces/qyqyc.rep
 * yes    88%    4800  0.000660  7269 ./traces/random.rep
 * yes    86%    4800  0.000786  6106 ./traces/random2.rep
 * yes    78%     147  0.000013 11542 ./traces/rm.rep
 * yes    67%     200  0.000012 16845 ./traces/rulsr.rep
 p yes     --    6495  0.000530 12252 ./traces/seglist.rep
   yes    89%      12  0.000002  5324 ./traces/short2.rep
18 17     85%  153836  0.011317 13593

Perf index = 47 (util) & 39 (thru) = 86/100
{"scores": {"Autograded Score": 86}, "scoreboard": [86, 86, 13593, 85]}
```


### 3. 调整extend_heap/sbrk的最小值

上面两个ops比较小的trace的util比较低，看了一下trace之后发现每次extend_heap最小增加（1<<12）B会造成很多external fragment，在这种access pattern里会非常明显，将其调整为（1<<8）B

```
Results for mm malloc:
  valid  util   ops    secs     Kops  trace
   yes    86%  100000  0.007480 13369 ./traces/alaska.rep
 * yes    98%    4805  0.000472 10184 ./traces/amptjp.rep
 * yes    85%    4162  0.000263 15843 ./traces/bash.rep
 * yes    77%   57716  0.003613 15973 ./traces/boat.rep
 * yes    97%    5032  0.000489 10286 ./traces/cccp.rep
 * yes    76%   11991  0.000791 15155 ./traces/chrome.rep
 * yes    99%   20000  0.001373 14569 ./traces/coalesce-big.rep
   yes    98%   14400  0.000875 16464 ./traces/coalescing-bal.rep
   yes   100%      15  0.000002  7528 ./traces/corners.rep
 * yes    98%    5683  0.000649  8754 ./traces/cp-decl.rep
 u yes    59%      --        --    -- ./traces/exhaust.rep
 * yes    79%    8000  0.000496 16130 ./traces/firefox.rep
   yes    84%   99804  0.006966 14327 ./traces/firefox-reddit.rep
   yes    87%     118  0.000011 10938 ./traces/hostname.rep
 * yes    93%   19405  0.001456 13324 ./traces/login.rep
 * yes    88%     200  0.000012 17289 ./traces/lrucd.rep
   yes    91%     372  0.000030 12364 ./traces/ls.rep
   yes    84%      10  0.000002  5981 ./traces/malloc.rep
   yes    81%      17  0.000002 10139 ./traces/malloc-free.rep
 u yes   100%      --        --    -- ./traces/needle.rep
 * yes    97%     200  0.000017 12030 ./traces/nlydf.rep
   yes    89%    1494  0.000120 12455 ./traces/perl.rep
 * yes    89%     200  0.000012 16848 ./traces/qyqyc.rep
 * yes    93%    4800  0.000598  8021 ./traces/random.rep
 * yes    90%    4800  0.000623  7706 ./traces/random2.rep
 * yes    94%     147  0.000014 10562 ./traces/rm.rep
 * yes    96%     200  0.000014 14767 ./traces/rulsr.rep
 p yes     --    6495  0.000633 10261 ./traces/seglist.rep
   yes    98%      12  0.000002  5109 ./traces/short2.rep
18 17     89%  153836  0.011524 13349

Comparison with libc malloc: mm/libc = 13349 Kops / 17773 Kops = 0.75
Perf index = 56 (util) & 39 (thru) = 95/100
{"scores": {"Autograded Score": 95}, "scoreboard": [95, 95, 13349, 89]}
```

***********
Main Files:
***********

mdriver
        Once you've run make, run ./mdriver to test your solution.

traces/
	Directory that contains the trace files that the driver uses
	to test your solution. Files corners.rep, short2.rep, and malloc.rep
	are tiny trace files that you can use for debugging correctness.

**********************************
Other support files for the driver
**********************************
config.h	Configures the malloc lab driver
fsecs.{c,h}	Wrapper function for the different timer packages
clock.{c,h}	Routines for accessing the x86-64 cycle counters
fcyc.{c,h}	Timer functions based on cycle counters
ftimer.{c,h}	Timer functions based on interval timers and gettimeofday()
memlib.{c,h}	Models the heap and sbrk function

***********************
Example malloc packages
***********************
mm.c            Empty malloc package
mm-naive.c      Fast but extremely memory-inefficient package
mm-textbook.c   Implicit list allocator based on CS:APP3e textbook

*******************************
Building and running the driver
*******************************
To build the driver, type "make" to the shell.

To run the driver on a tiny test trace:

	unix> ./mdriver -V -f traces/malloc.rep

To get a list of the driver flags:

	unix> ./mdriver -h

The -V option prints out helpful tracing information



