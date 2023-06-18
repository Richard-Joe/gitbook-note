# 问题复盘

## 1. 2023/06/07

- 流控，补充令牌的时间；
- RPS和RFS实现原理；他们在哪个阶段？
- 硬中断和软中断；
- 内核性能优化；
- 内核有哪些锁？rcu锁原理；只读的情况下rcu锁和读锁性能是一样的吗？
- kmalloc和vmalloc的区别，vmalloc申请4字节的实现流程；
- 中断上申请一页内存；
- TAP和TUN的区别；
- TCP握手丢包问题；2MSL；拥塞控制；
- ebpf原理
- 算法：二维数组，行递增，列递增，查找目标值；

1. rcu锁分析：

- rcu_read_lock实现，其实就是禁止内核抢占。没有其他多余操作。
- read_lock实现，其实就是自旋锁，写的时候才会自旋。读可以并发。
- read_lock_irq，禁止中断并获取锁。

## 2. 2023/06/09

- 聊k8s；
- kmalloc和vmalloc的区别；
- 内核并发方式，（多线程、抢占、中断，多核并发访问）
- 比如系统调用和软中断抢锁，应该如何使用锁；（使用带bh的锁）
- 优先级：中断上下文>软中断上下文>进程上下文；
- 多核系统（SMP）和单核系统中，spin_lock的区别；
	- SMP：关闭抢占后？
	- UP：如果支持抢占，仅仅是关闭抢占；如果不支持抢占，就是空操作；
- 逻辑题：早上8点发一辆，每15分钟发一辆，晚18点发最后一辆；共计发了多少辆；
- 拓扑题：PCa ---- CPa =========== CPb ---- PCb；中间是VPN隧道，PCa向PCb发TCP大包，描述下流程。

1. 自旋锁分析：

- spin_lock：关闭抢占
- spin_lock_irq：比如中断和进程之间出现竞争，需要关闭中断；
- spin_lock_bh：比如下半部和进程之间出现竞争，需要关闭软中断；
- 多核系统下：加解锁的操作函数需要具备原子性或独占性；
	- 加锁：1.先将next变量+1，不管能不能获得锁；2.循环判断next和owner变量是否相等，相等才能完成加锁，不相等就调用wfe指令进入idle状态。
	- 解锁：把owner+1
	- 为什么要使用两个变量管理？（可以对锁请求进行排序，相当于状态记录）
	- 独占性：CPU提供了特殊的独占指令：strex、ldrex。（当处理器 A 使用了 ldrex 指令独占访问了 next 时，处理器 B 使用 ldrex 回写 next 时就会失败）
	- 内存屏障：为了防止 CPU 的乱序执行而导致的问题；