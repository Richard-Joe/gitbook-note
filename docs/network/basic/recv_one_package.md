# 内核是如何接收一个网络包的

## 一、准备工作

内核版本`5.14.14`

### 注册硬中断
---

以Intel e1000 网卡驱动为例，找到`e1000_main.c`源码：

```c
static struct pci_driver e1000_driver = {
	...
	.name     = e1000_driver_name,	// 驱动名称
	.probe    = e1000_probe,		// 设备插入内核时调用
	.remove   = e1000_remove,		// 设备从内核移除时调用
	.shutdown = e1000_shutdown,		// 设备关闭时调用
	...
};
static int __init e1000_init_module(void)
{
	...
	// 注册网卡驱动
	ret = pci_register_driver(&e1000_driver);
	...
}
static void __exit e1000_exit_module(void)
{
	// 注销网卡驱动
	pci_unregister_driver(&e1000_driver);
}
```

可以看到`probe`是关键，那`probe`什么时候被调用的呢？（很多网上文章说是在`call_driver_probe`里面调用，其实还没说到底，仔细看会发现函数定义类型不一样）

```c
struct pci_driver {
	...
	int  (*probe)(struct pci_dev *dev, const struct pci_device_id *id);	/* New device inserted */
	...
};

struct bus_type {
	...
	int (*probe) (struct device *dev);
	...
};

struct device_driver {
	...
	int (*probe) (struct device *dev);
	...
};
```

那我们继续看`pci_register_driver`的实现
```c
struct bus_type pci_bus_type = {
	...
	.name		= "pci",
	.probe		= pci_device_probe,
	...

};
int __pci_register_driver(struct pci_driver *drv, struct module *owner,
			  const char *mod_name)
{
	/* initialize common driver fields */
	...
	drv->driver.bus = &pci_bus_type; // 这一步会设置总线类型
	...

	spin_lock_init(&drv->dynids.lock);
	INIT_LIST_HEAD(&drv->dynids.list);

	/* register with core */
	return driver_register(&drv->driver);
}
```
在`pci_device_probe -> pci_call_probe -> local_pci_probe -> `中找到了：
```c
static long local_pci_probe(void *_ddi)
{
	...
	pci_dev->driver = pci_drv;
	rc = pci_drv->probe(pci_dev, ddi->id);
	...
}
```

所以，网卡驱动`probe`调用链为：`pci_register_driver -> driver_register -> bus_add_driver -> __driver_attach ->  driver_probe_device -> really_probe -> call_driver_probe -> bus的probe -> pci_device_probe -> pci_call_probe -> local_pci_probe`

> 这里有必要简单说下，`bus`、`driver`、`device`的区别：
> `bus`：总线（Bus）是指计算机组件间规范化的交换数据（data）的方式，即以一种通用的方式为各组件提供数据传送和控制逻辑。如果说主板（Mother Board）是一座城市，那么总线就像是城市里的公共汽车（bus），能按照固定行车路线，传输来回不停运作的比特（bit）。-- wikipedia
> `driver`：驱动程序，提供操作的软件接口。
> `device`：设备就是连接在总线上的物理实体。
> ![recv_one_package-images/bus.png](recv_one_package-images/bus.png)

那下面接着看`probe`的具体做了些什么：
```c
static const struct net_device_ops e1000_netdev_ops = {
	.ndo_open			= e1000_open,
	.ndo_stop			= e1000_close,
	.ndo_start_xmit		= e1000_xmit_frame,
	.ndo_do_ioctl		= e1000_ioctl,
	...
};
static int e1000_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	...
	// 设置net_device_ops
	netdev->netdev_ops = &e1000_netdev_ops;
	// 注册ethtool实现函数
	e1000_set_ethtool_ops(netdev);
	// NAPI初始化，注册poll函数（e1000_clean）
	netif_napi_add(netdev, &adapter->napi, e1000_clean, 64);
	// 获取MAC地址
	e1000_read_mac_addr(hw);
	// 注册net_device
	strcpy(netdev->name, "eth%d");
	err = register_netdev(netdev);
	...
}
```

上面网卡驱动初始化都完成后，就可以启动网卡了。当启动一个网卡时，`net_device_ops`中的`e1000_open`会被调用。
```c
int e1000_open(struct net_device *netdev)
{
	...
	// 分配RingBuffer，分配RX、TX队列内存，DMA初始化
	err = e1000_setup_all_tx_resources(adapter);
	err = e1000_setup_all_rx_resources(adapter);
	// 注册中断处理函数（e1000_intr）
	err = e1000_request_irq(adapter);
	// 启用NAPI
	napi_enable(&adapter->napi);
	...
}
```

### 注册软中断

---

Linux的软中断都是在专门的内核线程`ksoftirqd`中进行的。该进程的数量等于机器的核数。
```shell
root@ubuntu:~# ps aux | grep ksoft
root          12  0.0  0.0      0     0 ?        S    18:16   0:00 [ksoftirqd/0]
root          20  0.0  0.0      0     0 ?        S    18:16   0:00 [ksoftirqd/1]
root          26  0.0  0.0      0     0 ?        S    18:16   0:00 [ksoftirqd/2]
root          32  0.0  0.0      0     0 ?        S    18:16   0:01 [ksoftirqd/3]

```

系统初始化时，调用`spawn_ksoftirqd`来创建出`ksoftirqd`进程。
当`ksoftirqd`被创建出来以后，它就会进入自己的线程循环函数`ksoftirqd`和`ksoftirqd`了。不停地判断有没有软中断需要被处理。

```c
static struct smp_hotplug_thread softirq_threads = {
	.store				= &ksoftirqd,
	.thread_should_run	= ksoftirqd_should_run,
	.thread_fn			= run_ksoftirqd,
	.thread_comm		= "ksoftirqd/%u",
};

static __init int spawn_ksoftirqd(void)
{
	...
	BUG_ON(smpboot_register_percpu_thread(&softirq_threads));

	return 0;
}
early_initcall(spawn_ksoftirqd);

static int smpboot_thread_fn(void *data)
{
	struct smpboot_thread_data *td = data;
	struct smp_hotplug_thread *ht = td->ht;

	while (1) {
		...
		if (!ht->thread_should_run(td->cpu)) {
			preempt_enable_no_resched();
			schedule();
		} else {
			__set_current_state(TASK_RUNNING);
			preempt_enable();
			ht->thread_fn(td->cpu);
		}
	}
}
```

linux内核通过调用`subsys_initcall`来初始化各个子系统。网络子系统的初始化阶段会进行注册软中断处理函数。
```c
/*
 *	The list of packet types we will receive (as opposed to discard)
 *	and the routines to invoke.
 *
 *	Why 16. Because with 16 the only overlap we get on a hash of the
 *	low nibble of the protocol value is RARP/SNAP/X.25.
 *
 *		0800	IP
 *		0001	802.3
 *		0002	AX.25
 *		0004	802.2
 *		8035	RARP
 *		0005	SNAP
 *		0805	X.25
 *		0806	ARP
 *		8137	IPX
 *		0009	Localtalk
 *		86DD	IPv6
 */
#define PTYPE_HASH_SIZE	(16)

struct list_head ptype_base[PTYPE_HASH_SIZE] __read_mostly;
struct list_head ptype_all __read_mostly;	/* Taps */

static int __init net_dev_init(void)
{
	...

	INIT_LIST_HEAD(&ptype_all);
	// 初始化hash表
	for (i = 0; i < PTYPE_HASH_SIZE; i++)
		INIT_LIST_HEAD(&ptype_base[i]);

	...

	// 为每个CPU都申请一个softnet_data数据结构，sd里面的poll_list等待驱动程序将其poll函数注册进来（后面会说到）
	for_each_possible_cpu(i) {
		struct softnet_data *sd = &per_cpu(softnet_data, i);

		skb_queue_head_init(&sd->input_pkt_queue);
		skb_queue_head_init(&sd->process_queue);

		INIT_LIST_HEAD(&sd->poll_list);

		...

		init_gro_hash(&sd->backlog);
		sd->backlog.poll = process_backlog;
		sd->backlog.weight = weight_p;
	}
	
	// 注册发包软中断处理函数
	open_softirq(NET_TX_SOFTIRQ, net_tx_action);
	// 注册收包软中断处理函数
	open_softirq(NET_RX_SOFTIRQ, net_rx_action);
	...
}
subsys_initcall(net_dev_init);

// 注册软中断回调函数
void open_softirq(int nr, void (*action)(struct softirq_action *))
{
	// 记录到softirq_vec中
	softirq_vec[nr].action = action;
}
```

上面说了`内核线程的启动`、`注册软中断处理函数`，下面接着说，内核线程如何处理软中断的。

我们接着看`ksoftirqd_should_run`、`run_ksoftirqd`。

```c
#define local_softirq_pending()	(__this_cpu_read(local_softirq_pending_ref))
static int ksoftirqd_should_run(unsigned int cpu)
{
	return local_softirq_pending();
}
```
这里看到，仅仅是读取软中断标记。如果在硬中断处理流程中设置了标记（后面在硬中断处理中可以看到），这里肯定能读到。

```c
static void run_ksoftirqd(unsigned int cpu)
{
	...
	__do_softirq();
	...
}

// 根据当前CPU的软中断类型，调用其注册的action方法。
asmlinkage __visible void __softirq_entry __do_softirq(void)
{
	set_softirq_pending(0);
	
	h = softirq_vec;
	while ((softirq_bit = ffs(pending))) {
		h += softirq_bit - 1;
		...
		h->action(h);
		...
		h++;
		pending >>= softirq_bit;
	}
}
```
这里注意一个细节，硬中断处理中设置软中断标记，和这里判断是否有软中断到达，都是基于当前cpu的。这意味着只要硬中断在哪个CPU上被响应，那么软中断也是在这个CPU上处理的。

如果发现你的linux上软中断CPU消耗都集中在一个核上，做法是调整硬中断的CPU亲和性，来将硬中断打散到不同的CPU核上去。


---
当做好以上准备工作后，就可以开始接收数据包了。

## 二、从网线到网卡

![recv_one_package-images/pci.png](recv_one_package-images/pci.png)

总之这个过程，**实质上就是把网线中的高低电平，转换到网卡上的一个缓冲区中存储着**。

## 三、从网卡到内存

数据到达了网卡这个硬件的缓冲区中，现在要把它弄到**内存中的缓冲区**。
![recv_one_package-images/dma.png](recv_one_package-images/dma.png)

这个过程完全不需要 CPU 参与，只需要 **DMA** 这个硬件设备，**DMA** 等网卡的缓冲区有数据到来时，把它拷贝到内存里。
![recv_one_package-images/dma_1.png](recv_one_package-images/dma_1.png)

## 四、硬中断处理

当DMA操作完成后，网卡会向CPU发起一个硬中断，通知CPU有数据到达。

上面我们说了，硬中断处理函数是`e1000_intr`

```c
static irqreturn_t e1000_intr(int irq, void *data)
{
	...
	__napi_schedule(&adapter->napi);
	...
}

static inline void ____napi_schedule(struct softnet_data *sd, struct napi_struct *napi)
{
	...
	// 把napi挂到softnet_data链表上
	list_add_tail(&napi->poll_list, &sd->poll_list);
	// 触发收包软中断
	__raise_softirq_irqoff(NET_RX_SOFTIRQ);
	...
}

#define or_softirq_pending(x)	(__this_cpu_or(local_softirq_pending_ref, (x)))
void __raise_softirq_irqoff(unsigned int nr)
{
	...
	// 触发软中断（仅仅对变量的一次或运算）
	or_softirq_pending(1UL << nr);
}
```
上面可以看到，硬中断处理过程真的非常短。只是记录了一个寄存器，修改了CPU的poll_list，然后发出软中断。

## 五、软中断处理

内核线程检测到软中断触发后，会调用注册的`net_rx_action`函数。

```c
static __latent_entropy void net_rx_action(struct softirq_action *h)
{
}
```