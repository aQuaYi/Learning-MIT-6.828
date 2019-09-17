# Introduction and examples

<!-- TOC depthFrom:2 -->

- [预习](#预习)
- [课堂材料](#课堂材料)
- [作业](#作业)
- [预习笔记](#预习笔记)
	- [进程与内存](#进程与内存)
	- [I/O 和文件描述](#io-和文件描述)
	- [管道](#管道)
	- [文件系统](#文件系统)
	- [现实世界](#现实世界)

<!-- /TOC -->

## 预习

阅读 [xv6 讲义](../book-riscv-rev0.pdf) 第一章

<https://www.youtube.com/watch?v=tc4ROCJYbm0> 观看这个关于 unix 的视频。

## 课堂材料

- [简介(introduction)](l-overview.txt.md)
- [示例(examples)](examples)
- [xv6 讲义](../book-riscv-rev0.pdf)

## 作业

[Lab: Xv6 与 Unix 工具](Lab_Xv6_and_Unix_utilities.html)

## 预习笔记

关键词：

- 操作系统
- 应用程序
- 进程
- xv6
- kernel 与 interface
- system call
- user space and kernel space
- shell

### 进程与内存

xv6 是一个分时操作系统。一次只有一个 process 被执行。没有被执行的 process ，其 CPU 寄存器等相关信息，会被 xv6 保存，等到需要执行的时候，再复原。

- **fork**: 会创建一个副本进程
- **exit**: 结束执行并释放资源
- **wait**: 等待子进程执行完毕
- **exec**: 加载文件，并执行

以 shell 为例子，讲解了上面 4 个 system call 的用法。

xv6 中所有的进程都以 root 方式执行。

### I/O 和文件描述

文件描述符 file descriptor 的特点：

- unix 中万物都被看成是文件
- 每个文件都有一个数字的索引号作为代号。
- 每个 process 都有一个表，用于保存 open 过的文件的 fd
- 同一个文件在不同的 process 的表中的代码，可能不一样。
- fork 后，子 process 同样会拷贝父 process 的表。
- close 后，fd 被释放。能够被别的文件占用
- open 时，总是挑选最小的可用 fd

- **read**: 读取内容
- **write**: 写入内容

file descriptor 是一个非常强大的抽象，因为它屏蔽了所有的具体细节。

### 管道

管道是一对 fd，一个用于入，一个用于出。

### 文件系统

文件系统由文件和文件夹组成。

- **chdir**：改变当前目录
- **mkdir**：创建目录
- **mknod**：创建设备文件

### 现实世界
