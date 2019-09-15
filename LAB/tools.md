# 构建工具链

本文根据官方的 [tools](https://pdos.csail.mit.edu/6.828/2019/tools.html) 指南编写。
目的是为 `xv6` 构建运行环境。

<!-- TOC depthFrom:2 -->

- [安装工具链](#安装工具链)
	- [(推荐)在 Manjaro 上安装工具](#推荐在-manjaro-上安装工具)
	- [(可选)在 Ubuntu 18.04 上编译工具](#可选在-ubuntu-1804-上编译工具)
	- [安装 `QEMU`](#安装-qemu)
	- [检查安装](#检查安装)
- [安装 `xv6`](#安装-xv6)
- [运行 xv6](#运行-xv6)

<!-- /TOC -->

## 安装工具链

MacOS，Linux 及 Windows 都能安装工具链，具体安装方式在官方 [tools](https://pdos.csail.mit.edu/6.828/2019/tools.html) 指南中都有说明。我试用了以下两种安装方式。

### (推荐)在 Manjaro 上安装工具

[Manjaro](https://manjaro.org/download/xfce/)基于 Arch linux，与 Arch 使用同样的软件仓库。所以，可以使用以下方式安装。

```shell
sudo pacman -S riscv64-linux-gnu-binutils riscv64-linux-gnu-gcc riscv64-linux-gnu-gdb qemu-arch-extra
```

### (可选)在 Ubuntu 18.04 上编译工具

下载源代码，

```shell
$ git clone https://github.com/riscv/riscv-gnu-toolchain
$ cd riscv-gnu-toolchain
$ git submodule update --init --recursive
$
```

直接下载 `qemu` 模块的速度慢的令人发指，[让终端走代理](https://blog.fazero.me/2015/09/15/%E8%AE%A9%E7%BB%88%E7%AB%AF%E8%B5%B0%E4%BB%A3%E7%90%86%E7%9A%84%E5%87%A0%E7%A7%8D%E6%96%B9%E6%B3%95/)会快很多。

下载完毕后，运行以下命令，检查是否下载成功了。

```shell
$ git submodule update --init --recursive
# 没有任何输出，代表下载完毕
$
```

clone 完成后，安装编译所需的软件

```shell
$ sudo apt-get install autoconf automake autotools-dev curl libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev
$
```

正式开始编译工作，

```shell
$ cd riscv-gnu-toolchain
$ ./configure --prefix=/usr/local
$ sudo make # 官方指南的代码是错误的
$ cd ..
$
```

### 安装 `QEMU`

下载并解压 `QEMU 4.1` 源代码:

```shell
$ wget https://download.qemu.org/qemu-4.1.0.tar.xz
$ tar xf qemu-4.1.0.tar.xz
$
```

构建 `QEMU` 的 `riscv64-softmmu`:

```shell
$ cd qemu-4.1.0
$ ./configure --disable-kvm --disable-werror --prefix=/usr/local --target-list="riscv64-softmmu"
# 如何上面的代码报错，请搜索错误信息。很容易解决的。
$ make
$ sudo make install
$ cd ..
$
```

### 检查安装

为了确保已经成功安装了工具链，应该出现以下结果：

```shell
$ riscv64-unknown-elf-gcc --version
riscv64-unknown-elf-gcc (GCC) 9.2.0
$ qemu-system-riscv64 --version
QEMU emulator version 4.1.0
```

## 安装 `xv6`

出于个人习惯，我 `fork` 了 `https://github.com/mit-pdos/xv6-riscv-fall19` 。然后，把 `fork` 后的项目，添加了为了此项目的子模块。

```shell
# 在本项目的根目录下
$ git submodule add https://github.com/aQuaYi/xv6-riscv-fall19 xv6-riscv
$ cd xv6-riscv
$
```

## 运行 xv6

You should also be able to compile and run xv6:

```shell
# in the xv6-riscv directory
$ make qemu
# ... lots of output ...
init: starting sh
$
```
