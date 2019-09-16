# Introduction and examples

<!-- TOC depthFrom:2 -->

- [预习](#预习)
- [课堂材料](#课堂材料)
- [作业](#作业)
- [预习笔记](#预习笔记)
	- [进程与内存](#进程与内存)

<!-- /TOC -->

## 预习

阅读 [xv6 讲义](../book-riscv-rev0.pdf) 第一章

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

