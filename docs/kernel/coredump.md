# coredump

## 1. 简介

当进程接收到某些 `信号` 而导致异常退出时，内核会根据进程当时的内存信息，就会生成 `coredump` 文件。

```bash
# 开启 coredump

# 确保 用户的limits 来自于/etc/security/limits.conf，修改文件：/etc/pam.d/sshd
session    required     pam_limits.so

# 确保 login 使用了 pam，修改文件：/etc/pam.d/login
session    required   pam_limits.so

# 确保 sshd 使用了 pam，修改文件：/etc/ssh/sshd_config
UsePAM yes

# 修改 /etc/security/limits.conf，指定coredump大小
root             soft    core            1048576

# 查看生效结果：ulimit -a
core file size          (blocks, -c) 1048576

# 设置 core 路径
kernel.core_uses_pid = 1
kernel.core_pattern = /var/core/core-%e-%p-%s-%t
```

## 2. 哪些信号会导致产生 coredump ？

- **SIGQUIT** : 用户按下键盘的 Ctrl+\ 组合键时会发送该信号。
- **SIGILL** : 在处理器执行非法指令时会发送该信号。
- **SIGABRT** : 调用abort()函数时会发送该信号。
- **SIGFPE** : 在发生除以零或浮点溢出等错误时会发送该信号。
- **SIGSEGV** : 在访问非法地址或地址越界时会发送该信号。
- **SIGBUS** : 在非法地址上进行内存访问时会发送该信号。
- **SIGSYS** : 在使用了不存在或无权访问的系统调用时会发送该信号。
- **SIGTRAP** : 由机器的breakpoint指令生成，用于调试。

当然还有一些信号也会产生，想要更全面一点，直接看代码：
```c
#define SIG_KERNEL_COREDUMP_MASK (\
        rt_sigmask(SIGQUIT)   |  rt_sigmask(SIGILL)    | \
	rt_sigmask(SIGTRAP)   |  rt_sigmask(SIGABRT)   | \
        rt_sigmask(SIGFPE)    |  rt_sigmask(SIGSEGV)   | \
	rt_sigmask(SIGBUS)    |  rt_sigmask(SIGSYS)    | \
        rt_sigmask(SIGXCPU)   |  rt_sigmask(SIGXFSZ)   | \
	SIGEMT_MASK				       )
```

使用示例：

- 比如直接使用 `kill -s SIGQUIT pid` 向进程发送信号；
- 比如调用syscall(999)访问不存在的系统调用；（/usr/include/asm/unistd_64.h 文件中可以看系统调用名称和编号）
- 比如使用 `__asm__("int3");` 汇编代码来会触发一个中断，这个中断对应着 SIGTRAP 信号；

## 3. coredump 如何产生的？

当进程从 `内核态` 返回到 `用户态` 前，内核会查看进程的信号队列中是否有信号没有处理，如果有就调用 `do_signal` 内核函数处理信号。

进程从 `内核态` 返回到 `用户态` 的场景：

- 从系统调用返回；
- 从硬中断处理程序返回；
- 从进程调度程序返回；
- 从处理器异常返回；（比如除法错误，内核处理完异常，切换回用户态）

```c
exit_to_user_mode
  | prepare_exit_to_user_mode
     | do_notify_resume
        | if (thread_flags & (_TIF_SIGPENDING | _TIF_NOTIFY_SIGNAL))
				      do_signal(regs);

// arm64 代码
do_signal
  | get_signal(&ksig) 
    | for (;;) {
          signr = dequeue_signal(...); // 从队列里取出一个信号
          if (sig_kernel_coredump(signr)) { // 判断信号是否需要产生coredump
              do_coredump(&ksig->info);  // 执行coredump生成流程
          }
    | }

do_coredump
  | binfmt = mm->binfmt; // 获取当前进程的可执行文件格式（比如ELF）
  | format_corename(...); // 根据core_pattern来格式化core文件名
  | if (cprm.limit < binfmt->min_coredump) // 判断当前进程所需要的资源是否足够
        goto fail_unlock;
  | cprm.file = filp_open(cn.corename, open_flags, 0600); // 创建core文件
  | core_dumped = binfmt->core_dump(&cprm); // 调用可执行文件格式指定的方法，把进程内存信息写入core文件

// 假设用的ELF格式
// Actual dumper
elf_core_dump
  | // 写入 ELF 头部
  | // 写入 Program 头部
  | // 写入 ELF note
  | // 把每一段 VMA 都写进去
```
