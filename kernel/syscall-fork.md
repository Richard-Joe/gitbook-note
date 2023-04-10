# kernel_clone

用户空间通过 `fork()` 函数， `vfork()` 函数， `pthread_create()` 函数创建进程/线程，内核空间通过 `kernel_thread()` 函数创建线程，最终都会通过`kernel_clone()` 函数创建。

```c
pid_t kernel_clone(struct kernel_clone_args *args)
{
	struct task_struct *p;
	struct pid *pid;

	...
	// 创建进程的主要函数，包括复制父进程的相关资源；
	p = copy_process(NULL, trace, NUMA_NO_NODE, args);
	...
	// 为子进程分配PID
	pid = get_task_pid(p, PIDTYPE_PID);
	...
	// 将子进程加入就绪队列，等待被调度；
	wake_up_new_task(p);
	...
}
```

## copy_process

```c
copy_process()
  | p = dup_task_struct(current, node); // 创建新的task_struct结构体，然后复制父进程的task_struct到子进程；
    | tsk = alloc_task_struct_node(node); // 从kmem_cache中创建task_struct结构体；
    | err = arch_dup_task_struct(tsk, orig); // 复制父进程的task_struct到子进程；
    | err = alloc_thread_stack_node(tsk, node); // 为task_struct分配内核栈空间；
    | clear_tsk_need_resched(tsk); // 清空子进程需要调度的标志位；
    | set_task_stack_end_magic(tsk); // 设置栈顶标记，用来检测栈溢出；

  | retval = copy_creds(p, clone_flags); // 复制父进程的credentials；

  | retval = sched_fork(clone_flags, p); 
    | __sched_fork(clone_flags, p); // 对task_struct中与调度相关的信息进行初始化；
    | p->__state = TASK_NEW; // 设置进程状态，表示新创建的进程；
    | p->prio = current->normal_prio; // 设置进程优先级；
    | if (rt_prio(p->prio)) p->sched_class = &rt_sched_class;
    | else p->sched_class = &fair_sched_class; // 设置进程调度类；
    | init_task_preempt_count(p); // 初始化进程preempt_count；

  | retval = copy_files(clone_flags, p); // 复制进程打开的文件描述符表；

  | retval = copy_fs(clone_flags, p); // 复制进程的文件系统信息；（当前工作目录、根目录、挂载的文件系统等信息）

  | retval = copy_sighand(clone_flags, p); // 复制进程的信号处理程序（signal handler）表；

  | retval = copy_signal(clone_flags, p); // 复制信号相关的条件，比如rlimit、oom_score_adj；

  | retval = copy_mm(clone_flags, p); // 复制进程的内存信息；
    | if (clone_flags & CLONE_VM) mm = oldmm; // 如果设置了CLONE_VM，新进程则与当前进程共享mm；
    | else mm = dup_mm(tsk, current->mm); // 否则，复制当前进程的mm_struct；（分配一个mm_struct，复制父进程的VMA）

  | retval = copy_namespaces(clone_flags, p); // 根据clone时传的标志，来为子进程创建独立的命名空间（mnt/uts/ipc/pid/cgroup/net/time）

  | retval = copy_thread(p, args); // 复制进程的CPU体系结构的相关信息；

  | pid = alloc_pid(...); // 分配pid结构;

  | copy_seccomp(p); // 复制父进程的安全过滤器（seccomp filter）;
```

### copy_thread

```c
struct task_struct {
	struct thread_info		thread_info;
	...
	void				*stack;
	...
	/* CPU-specific state of this task: */
	struct thread_struct		thread;
};

/* arm64 */
struct cpu_context {
	unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long fp;
	unsigned long sp;
	unsigned long pc;
};

struct thread_struct {
	struct cpu_context	cpu_context;	/* cpu context */
	...
};

struct pt_regs {
	union {
		struct user_pt_regs user_regs;
		struct {
			u64 regs[31];
			u64 sp;
			u64 pc;
			u64 pstate;
		};
	};
	...
};
```

- `cpu_context`：在进程切换时用来保存上一个进程的寄存器的值；
- `thread_struct`：在内核态的两个进程发生切换时，用来保存上一个进程相关寄存器；
- `pt_regs`：当用户态进程发生异常（系统调用、中断等）并进入内核态时，用来保存用户态进程的寄存器状态；

```c
copy_thread()
  | memset(&p->thread.cpu_context, 0, sizeof(struct cpu_context));
  | p->thread.cpu_context.pc = (unsigned long)ret_from_fork; // 设置新进程的PC指针为ret_from_fork；新进程运行时会从ret_from_fork运行，它是汇编语言编写的；
```

## wake_up_new_task

```c
wake_up_new_task(struct task_struct *p)
  | WRITE_ONCE(p->__state, TASK_RUNNING); // 设置进程状态为TASK_RUNNING
  | __set_task_cpu(p, select_task_rq(p, task_cpu(p), WF_FORK)); // 选择在哪个CPU上运行；
  | activate_task(rq, p, ENQUEUE_NOCLOCK); // 加入该CPU的就绪队列，等待调度；
```

## ret_from_fork

```armasm
SYM_CODE_START(ret_from_fork)
	bl	schedule_tail
	cbz	x19, 1f				// not a kernel thread
	mov	x0, x20
	blr	x19
1:	get_current_task tsk
	mov	x0, sp
	bl	asm_exit_to_user_mode
	b	ret_to_user
SYM_CODE_END(ret_from_fork)
NOKPROBE(ret_from_fork)
```
