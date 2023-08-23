## MIT 操作系统课程（2022年）

### 课程资源

- 22年课程主页
    - https://pdos.csail.mit.edu/6.828/2022/schedule.html
- 翻译和视频都是20年的
    - https://www.bilibili.com/video/BV1rS4y1n7y1
    - https://mit-public-courses-cn-translatio.gitbook.io/mit6-s081/

### lab1: Unix utilities
- https://pdos.csail.mit.edu/6.828/2022/labs/util.html
- https://github.com/Alaskra/xv6-labs-2022/tree/lab1
#### 实验内容
- 编写如下shell工具：find, xargs, pingpong, sleep, primes
#### 实验目标
- 熟悉用各种系统调用：fork, exec, read, write, pipe, sleep
#### 难点
- 熟悉fork
- 父进程要使用wait来回收子进程，不然会出错
- primes的编写，注意pipe的管理

### lab2: System calls
- https://pdos.csail.mit.edu/6.828/2022/labs/syscall.html
- https://github.com/Alaskra/xv6-labs-2022/tree/lab2
#### 实验内容
- 根据gdb的结果回答问题
- 添加系统调用: trace, sysinfo
#### 实验目标
- 熟悉gdb的使用
- 学会系统调用的使用
#### 注意点
- gdb的使用
    - 主要是断点设置(b)、单步执行(s n)和打印(p)这几个功能
    - 参考链接：
        - https://pdos.csail.mit.edu/6.828/2019/lec/gdb_slides.pdf
        - https://wizardforcel.gitbooks.io/100-gdb-tips/content/print-registers.html
    - gdb默认会先执行HOME目录以及当前目录下的.gdbinit文件中的指令
    - 对kernel设置断点没有问题，但是用户程序怎么设置断点还搞不清楚
    - 还可以使用vscode进行图形化gdb调试
        - 需要安装插件：Native Debug
        - 配置.vscode/launch.json（内容参照main分支对应文件）

### lab3: Page tables
- https://pdos.csail.mit.edu/6.828/2022/labs/syscall.html
- https://github.com/Alaskra/xv6-labs-2022/tree/lab3
#### 实验内容
- Speed up system calls
- Print a page table
- Detect which pages have been accessed
#### 实验目标
- 学会调用mappages操作页表
- 熟悉三级页表结构
- 了解PTE的控制位的含义
#### 注意点
- PTE中的dirty位标志是否被访问（这些控制位好像是硬件设置的（见risc-v手册），每次读写TLB时会置1）
- 虚拟地址转换是硬件(MMU+TLB)+软件(操作系统，当缺页的时候)实现的
    - 三次页表的递归查找，也是使用硬件（MMU）完成的
    - xv6内核也使用页表，但一般虚地址和物理地址一一对应
    - 参照教程中的图片，了解内核以及用户内存空间的分布
- 先分配物理空间，然后才映射页表
    - kfree和kalloc: free和alloc物理空间，kmem.freelist是一个链表结构，保存一系列空闲的物理页地址
#### 踩坑
- 第一个实验没有按照实验的hint来实现，导致第二个实验运行结果不一致，误以为是第二个实验哪里写错了，浪费了比较长的时间
- 为了找出不一致的原因，首先确定不一致的地方是页表中虚拟地址对应的物理地址，于是怀疑是kalloc和kfree的顺序引起。对比我的代码和其它参考代码，最终发现是没考虑到exec中还有创建和删除页表的操作，原本以为只有创建和退出进程的时候才有创建和删除页表的操作。
- kalloc可以选择在不同地方执行，需要考虑不同选择的后果，以避免bug

### lab4: Traps
- https://pdos.csail.mit.edu/6.828/2022/labs/traps.html
- https://github.com/Alaskra/xv6-labs-2022/tree/lab4
#### 实验内容
- RISC-V assembly
- backtrace
- alarm(hard)
#### 实验目标
- 熟悉risc-v汇编
- 理解函数调用约定，risc-v的函数帧栈的布局
- 学会处理中断
#### 难点
- alarm的实现
- alarm中的handle函数需要另一套执行环境，因为它不是用户程序调用的，也不是在内核中执行
- handle函数会占用本该用于保存用户程序的trampframe，需要在proc结构体中额外开辟空间存保存寄存器

### lab5: Copy-on-write fork
- https://pdos.csail.mit.edu/6.828/2022/labs/cow.html
- https://github.com/Alaskra/xv6-labs-2022/tree/lab5
#### 实验内容
- 实现COW
#### 实验目标
- 理解COW优化
- 学会处理page fault
#### 难点
- 引用计数ref_cnt，记录多少PTE指向了某个物理页，要理清楚哪些地方ref_cnt需要+1或-1
- 并不需要在所有kalloc的时候ref_cnt+1，只需要在修改页表表项时+1或-1
- 在mappages中+1，特殊情况是kvmmap会调用mappages，但是这些物理地址都不是RAM的地址，不需要ref_cnt+1
- 在uvmunmap中-1
#### 坑
- 没有将ref_cnt设置锁，导致多核条件下出错

### lab6: Multithreading
- https://pdos.csail.mit.edu/6.828/2022/labs/thread.html
- https://github.com/Alaskra/xv6-labs-2022/tree/lab6
#### 实验内容
- Uthread: switching between threads，用户级别的线程系统
- Using threads和Barrier，使用pthread完成实验
#### 实验目标 
- 实现uthread，更深刻理解swtch函数的原理
- 熟悉pthread接口
#### 注意点
- 线程切换需要做哪些事
    - 修改线程状态:RUNNING->RUNNABLE
    - 调用scheduler调度线程
        - 不像xv6线程调度器是单独的线程，uthread把线程调度器实现成了一个函数用来调用
        - 模仿swtch切换thread
    - 需要有地方保存线程的执行环境：寄存器和栈
- pthread_cond_wait和pthread_cond_broadcast就相当于xv6的sleep和wakeup
- 对hash table的不同bucket分别加锁，以减少竞争锁带来的开销

### lab7: Network driver
- https://pdos.csail.mit.edu/6.828/2022/labs/net.html
- https://github.com/Alaskra/xv6-labs-2022/tree/lab7
#### 实验内容
- networking，实现网络驱动
#### 实验目标
- 了解xv6操作系统的网络协议栈
- 熟悉e1000硬件的ring结构
- 了解怎么写驱动代码
#### 难点
- 读懂硬件文档，理解E1000硬件的过程（还是读不来
- e1000_transmit E1000_TXD_CMD_EOP忘记设置（但只看文档，看不出为什么要设置这个寄存器啊
- e1000_recv 需要使用循环进行多次net_rx
#### 疑问：e1000_recv不直接读取E1000_RDT作为索引，而要+1的理解（所以手册上的图是画错了吗？
- 这里约定head和tail指NIC的寄存器，环形队列的队头和队尾分别指消费者指针和生产者指针
- 数据结构：环形队列
    - 根据head=tail表示环形队列为空还是为满(可以自行约定)，环形队列有两种表示方法，分别对应tx_ring和rx_ring
    - 一般来说，tail表示生产者指针，head表示消费者指针（但是对于NIC接收消息的情况(rx_ring)，head和tail是反过来的，因为约定硬件总是操作HEAD寄存器）
    - 生产者指针总是指向下一个要生产的位置
    - 如果约定head=tail表示队列空，则消费者指针总是指向下一个要消费的位置
    - 如果约定head=tail表示队列满，则消费者指针+1才指向下一个要消费的位置
- NIC（网卡硬件）控制head寄存器（TDH或RDH）,当head!=tail时，向head写数据，然后head+1。直到head=tail
    - 当head=tail时，NIC停止运行
- CPU控制tail寄存器
- 在发送消息的情况下，即tx_ring
    - 生产者是CPU，消费者是NIC。即队头和队尾分别是head和tail
    - 队列空时NIC才停止运行，所以head=tail对应于队列空
    - 初始队列为空，所以head=tail=0
    - 生产者CPU往tail写数据然后tail++
- 在接受消息的情况下，即rx_ring
    - 生产者是NIC，消费者是CPU。即队头和队尾分别是tail和head
    - 队列满时NIC才停止运行，所以head=tail对应于队列满
    - 初始队列为空，所以head=0，tail=15
    - 消费者CPU从tail+1读数据，然后tail++

### lab8: Parallelism/locking
- https://pdos.csail.mit.edu/6.828/2022/labs/lock.html
- https://github.com/Alaskra/xv6-labs-2022/tree/lab8
#### 实验内容
- Memory allocator, 减少kalloc和kfree导致的竞争
- Buffer cache，减少disk cache的竞争
#### 实验目标
- 理解锁的竞争导致性能下降
- 学会避免死锁
#### 注意点
- 先熟悉不同lock的用途
- 通过按序获得锁来避免死锁

### lab9: File system
- https://pdos.csail.mit.edu/6.828/2022/labs/fs.html
- https://github.com/Alaskra/xv6-labs-2022/tree/lab9
#### 实验内容
- Large files
- Symbolic links
#### 实验目标
- 熟悉inode的结构，如何根据inode找到数据存储在哪个磁盘block上
- 了解符号链接的原理
#### 踩坑
- fs.h中的NDBLINDIRECT值搞错了，导致代码中数组越界。这种错误，而且还是在操作系统代码中，太难根据测试来定位了，既然代码不多，应该直接从源代码定位的。但是本能地忽略了.h文件中宏定义错误的可能性，只检查.c文件中的逻辑，导致一直找不到错误。
- symlink递归查找时，没有释放锁就覆盖了inode指针，导致bug

### lab10: Mmap
- https://pdos.csail.mit.edu/6.828/2022/labs/mmap.html
- https://github.com/Alaskra/xv6-labs-2022/tree/lab10
#### 实验内容
- mmap
#### 实验目标
- 实现mmap，学会mmap的原理
#### 不太优雅的实现
- 为了简单，mmap直接从heap起始地址开始映射，mmap之后，p->sz相应增长，并且没有实现内存管理，munmap并不会回收mmap的内存，会导致最终内存耗尽
- 采用lazy allocate，只要没有使用虚拟地址，它在页表中相应的位置就是invalid
- 这导致heap空间有很多invalid的页表项，和之前xv6的假设不符，导致要修改两行其它部分的内核代码
#### 注意点
- 整体逻辑还算好理，但是具体实现需要了解很多细节，比如文件的读写怎么操作
- 如果没有实验指导中的hints，想不到要对进程exit以及fork进行修改
- 如果把unmap的逻辑放在exit的更后面，会有panic: sched locks
    - 原因是unmap有写磁盘的操作，可能会因为sleep lock而导致调度，需要注意锁的问题
    - 所以，应该将unmap的逻辑放在和close file(这个操作也可能导致调度)一样的位置
