# Kprobes

	Kprobes enables you to dynamically break into any kernel routine and collect debugging and performance information non-disruptively. You can trap at almost any kernel code address 1, specifying a handler routine to be invoked when the breakpoint is hit.

目前有两种探针：
- kprobes：几乎可以插入内核中的任何指令（当然也有黑名单）
- kretprobes：在指定函数返回时触发

可以通过两种方式使用kprobes：

- 一种是`编写内核模块`，调用register_kprobe/unregister_kprobe，向内核注册探测点，灵活编写pre_handler 和 post_handler回调函数；
- 另一种是使用 `kprobes + ftrace`

## 1. 实践

### 1.1. kprobe_example.c、kretprobe_example.c

源码在内核samples/kprobes目录下

```bash
# 编译kprobe_example.ko
# insmod kprobe_example.ko
# 随便敲几个命令
# dmesg
[ 3116.494530] handler_pre: <kernel_clone> p->addr = 0x0000000005f99348, ip = ffffffffa20fffc5, flags = 0x206
[ 3116.494535] handler_post: <kernel_clone> p->addr = 0x0000000005f99348, flags = 0x206
[ 3119.966756] handler_pre: <kernel_clone> p->addr = 0x0000000005f99348, ip = ffffffffa20fffc5, flags = 0x206
[ 3119.966762] handler_post: <kernel_clone> p->addr = 0x0000000005f99348, flags = 0x206

# rmmod kprobe_example
```

### 1.2. kprobes + ftrace

```bash
/sys/kernel/debug/kprobes/list: 内核中已经设置kprobes断点的函数
/sys/kernel/debug/kprobes/enabled: kprobes开关
/sys/kernel/debug/kprobes/blacklist: kprobes黑名单（无法设置断点函数）
/proc/sys/debug/kprobes-optimization: kprobes优化开关
```

```bash
# 开启trace
echo 1 > /sys/kernel/debug/tracing/tracing_on

# 注册
echo 'p:myprobe icmp_rcv' > /sys/kernel/debug/tracing/kprobe_events
echo 1 > /sys/kernel/debug/tracing/events/kprobes/myprobe/enable

# 随便ping几个包到本机
# 查看
cat /sys/kernel/debug/tracing/trace
# tracer: nop
#
# entries-in-buffer/entries-written: 6/6   #P:4
#
#                                _-----=> irqs-off/BH-disabled
#                               / _----=> need-resched
#                              | / _---=> hardirq/softirq
#                              || / _--=> preempt-depth
#                              ||| / _-=> migrate-disable
#                              |||| /     delay
#           TASK-PID     CPU#  |||||  TIMESTAMP  FUNCTION
#              | |         |   |||||     |         |
          <idle>-0       [001] ..s1.  4084.212468: myprobe: (icmp_rcv+0x4/0x3c0)
          <idle>-0       [001] ..s1.  4085.213941: myprobe: (icmp_rcv+0x4/0x3c0)
          <idle>-0       [001] ..s1.  4135.129815: myprobe: (icmp_rcv+0x4/0x3c0)
          <idle>-0       [001] ..s1.  4136.132458: myprobe: (icmp_rcv+0x4/0x3c0)

# 删除
echo 0 > /sys/kernel/debug/tracing/events/kprobes/myprobe/enable
echo '-:myprobe' > /sys/kernel/debug/tracing/events/kprobe_events
```

## 2. 原理


## 3. 参考

https://www.kernel.org/doc/html/latest/trace/kprobes.html
https://www.kernel.org/doc/html/latest/trace/kprobetrace.html