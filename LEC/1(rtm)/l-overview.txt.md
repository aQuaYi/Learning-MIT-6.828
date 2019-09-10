6.S081/6.828 2019 Lecture 1: O/S overview

Overview

* 6.S081 goals
  * Understand operating system design and implementation
  * Hands-on experience extending a small O/S
  * Hands-on experience writing systems software

* What is the purpose of an O/S?
  * Abstract the hardware for convenience and portability
  * Multiplex the hardware among multiple applications
  * Isolate applications in order to contain bugs
  * Allow sharing among applications
  * Control sharing for security
  * Provide high performance
  * Support a wide range of applications

* What is the O/S design approach?
  * abstractions that are easy to use and share
  * a separate referee to isolate and control sharing

* Organization: layered picture
  [user/kernel diagram]
  - user applications: vi, gcc, DB, &c
  - kernel services
  - h/w: CPU, mem, disk, net, &c
  * we care a lot about the interfaces and internal kernel structure

* What services does an O/S kernel typically provide?
  * process (a running program)
  * memory allocation
  * file contents
  * directories and file names
  * access control (security)
  * many others: users, IPC, network, time, terminals

* What does the O/S kernel interface look like?
  * System calls
  * Examples, from UNIX (e.g. Linux, OSX, FreeBSD):

            fd = open("out", 1);
            write(fd, "hello\n", 6);
            pid = fork();

  * These look like function calls but they aren't 

* Why is O/S design/implementation hard/interesting?
  * unforgiving environment: quirky h/w, hard to debug
  * it must be efficient (thus low-level?)
	...but abstract/portable (thus high-level?)
  * powerful (thus many features?)
	...but simple (thus a few composable building blocks?)
  * flexible (thus allow lots of sharing?)
        ...but secure (thus forbid sharing?)
  * features interact: `fd = open(); ...; fork()`
  * behaviors interact: CPU priority vs memory allocator
  * apps are wildly diverse, O/S must always behave/perform well
  * new hardware opportunities: NVRAM, multi-core, fast networks
  * new use cases: smart-phones, cloud, virtual machines

* You'll be glad you took this course if you...
  * care about what goes on under the hood
  * like infrastructure
  * care about high performance
  * need to diagnose bugs or security problems

Class structure

* See web site: https://pdos.csail.mit.edu/6.828

* Lectures
  * O/S ideas
  * detailed case study of xv6, a small O/S
  * lab background
  * O/S papers

* Labs: 
  The point: hands-on experience
  One or two weeks each.
  Three kinds:
    Systems programming, e.g. Unix programming, due next week.
    O/S primitives, e.g. thread switching.
    O/S kernel extensions to xv6, e.g. network stack.
  Use piazza to discuss, ask/answer questions.

* Two exams: midterm during class meeting, final in finals week

* Two versions: 6.S081 for undergrads, 6.828 for M.Eng credit.
  Next year and beyond: two separate courses.
    6.828 will be a graduate research course (advanced papers and projects).
      With 6.S081 as a pre-requisite.
    6.S081 an undergraduate course.
      Only 6.004 as pre-requisite.
  This year: 6.S081 and 6.828 are combined.
    Register for 6.S081 if you are an undergraduate.
    6.828 only if you need M.Eng credit.
      6.828 students will do a project of your choice, rather than last lab.
    Web info says 6.033 is a pre-requisite, but really only 6.004.

Introduction to UNIX system calls

* 6.S081 focuses on design and implementation of system call interface.
  we'll look at UNIX (Linux, Mac, POSIX -- and xv6).
  let's look at how programs use the system call interface.
  you'll use these system calls in the first lab.

* I'll show some examples, and run them on xv6.
  xv6 has traditional design, copied from UNIX/Linux.
  but simple -- you can (and should) understand all of xv6
    accompanying book explains how xv6 works, and why
  why a UNIX-like O/S?
    open source, well documented, clean design, widely used
  xv6 has two course functions:
    example of core features: virtual memory, multi-core, interrupts, &c
    starting point for many of the labs, which add new features
  xv6 runs on RISC-V, as in current 6.004
  you'll run xv6 under the qemu virtual machine

* example: copy.c, copy input to output
  read bytes from input, write them to the output
  $ copy
  written in C, the traditional O/S language
    Kernighan and Ritchie (K&R) book is good for learning C
  read() and write() are system calls
  first read()/write() argument is a "file descriptor" (fd)
    passed to kernel to tell it which "open file" to read/write
    must previously have been opened
    an FD connects to a file/device/socket/&c
    a process can open many files, have many FDs
    UNIX convention: fd 0 is "standard input", 1 is "standard output"
  second read() argument is a pointer to some memory into which to read
  third argument is the number of bytes to read
    read() may read less, but not more
  return value: number of bytes actually read
  note: copy.c does not care about the format of the data
    UNIX I/O is 8-bit bytes
    interpretation is application-specific, e.g. C program, executable, &c
  where do file descriptors come from?

* example: open.c, create a file
  $ open
  $ cat output.txt
  FD is a small integer
  FD indexes into a per-process table maintained by kernel
  [user/kernel diagram]
  different processes have different FD name-spaces
    i.e. FD 1 often means different things to different processes
  note: these examples ignore errors -- don't be this sloppy!
    find out about system call arguments/return with "man 2 open"

* what happens when a program calls the open() system call?
  looks like a function call, but it's actually a special instruction
  hardware saves some user registers
  hardware increases privilege level
  hardware jumps to a known "entry point" in the kernel
  now running C code in the kernel
  kernel calls system call implementation
    open() looks up name in file system
    it might wait for the disk
    it updates kernel data structures (cache, FD table)
  restore user registers
  reduce privilege level
  jump back to calling point in the program, which resumes
  we'll see more detail later in the course

* I've been typing to UNIX's command-line interface, the shell.
  the shell is what you see on Athena time-sharing machines
  the shell lets you run various UNIX command-line utilities
    useful for system management, messing with files, development
    $ ls
    $ ls > out
    $ grep x < out
  UNIX supports other kinds of programs too
    window systems, GUIs, servers, compute clusters, &c.
  but the shell and command-line utilities were the original interface.
  we can exercise many system calls via the shell.

* example: fork.c, create a new process
  the shell creates a new process for each command you type, e.g. for
    $ echo hello
  the fork() system call creates a new process
    $ fork
  the kernel copies the parent process to produce a child
    instructions, data, registers, file descriptors, current directory
  only difference: fork() returns a pid in parent, 0 in child
  a pid (process ID) is an integer, kernel gives each process a different pid
  thus:
    fork.c's "fork() returned" executes in *both* processes
    the "if(pid == 0)" allows code to distinguish
  ok, now we have *two* shell processes...
    what about running a program?

* example: exec.c, replace process with an executable file
  how does the shell actually run
    $ echo hello
  a program is stored in a file: instructions and initial memory
    created by the compiler and linker
  so there's a file called echo, containing instructions
  $ exec
  exec() replaces current process with an executable file
    discards instruction and data memory
    loads instructions and memory from the file
    preserves file descriptors
  exec(filename, argument-array)
    argument-array holds command-line arguments; exec passes to main()
    cat user/echo.c
    echo.c shows how a program looks at its command-line arguments

* example: forkexec.c, fork() a new process, exec() a program
  $ forkexec
  this is a common UNIX idiom:
    fork() a child process
    exec() a command in the child
    parent wait()s for child to finish
  the shell does fork/exec/wait for every command you type
    after wait(), the shell prints the next prompt
    if no wait(), background (shell's & skips the wait())
  note: the fork() copies, followed by exec() discarding memory
    this may seem wasteful
    you'll transparently optimize away the copy in a lab!

* example: redirect.c, redirect the output of a command
  what does the shell do for
    $ echo hello > out
  answer: fork, change FD 1 in child, exec
  $ redirect
  $ cat output.txt
  note: open() always chooses lowest unused FD; 1 due to close(1).
  it's neat that I/O setup can be separated from program logic
    programs like echo do not need to understand redirection
      they just read from FD 0, write to FD 1
    only the shell has to know about redirection
  FDs help make this work
    a level of indirection -- FD 1 rather than a specific file
    kernel maintains FD tables across fork() and exec()
  fork/exec separation helps make this work
    gives sh a chance to change FDs before exec

* Why these I/O and process abstractions?
  Why not something else?
  Why FS in kernel? Why not let user programs use disk their own way?
  Why the FD level of indirection? Why not pass filename to write()?
  Why streams of bytes, not disk blocks, or formatted records?
  Why are fork() and exec() separate? Why not combine them?
  The UNIX design works well, but we will see other designs!

* example: pipe1.c, communicate through a pipe
  how does the shell implement
    $ ls | grep x
  $ pipe1
  FD can refer to a "pipe", as well as a file
  the pipe() system call creates a pair of FDs
    read from the first FD
    write to the second FD
  kernel maintains pipe buffer internally
    [u/k diagram]
    write() appends to the buffer
    read() waits until there is data

* example: pipe2.c, communicate between processes
  pipes combine well with fork() to implement ls | grep x
    shell creates a pipe,
    then forks (twice),
    then connects ls's FD 1 to pipe's write FD,
    and grep's FD 0 to the pipe
    [diagram]
  $ pipe2 -- a simplified version
  inheritance of FDs helps make pipe/fork/exec work
  the point: pipes are a separate abstraction, but combine well w/ fork()

* example: list.c, list files in a directory
  how does ls get a list of the files in a directory?
  you can open a directory and read it -> file names
  "." is a pseudo-name for a process's current directory
  see ls.c for more details

* Summary
  * We've looked at UNIX's I/O, file system, and process abstractions.
  * The interfaces are simple -- just integers and (copied) buffers.
  * The abstractions combine well, e.g. for I/O redirection.

You'll use UNIX system calls in the first lab, due next week.
