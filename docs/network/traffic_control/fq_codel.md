# 公平队列控制延迟

在网络路由中，CoDel（Controlled Delay）是Van Jacobson和Kathleen Nichols开发的网络调度器中使用的调度算法 。 它旨在通过设置缓冲区中网络数据包的延迟限制来克服网络硬件（如路由器）中的 `缓冲膨胀` ，改善了随机早期检测（RED）算法的整体性能，设计上 CoDel 比 RED 更容易管理和配置。

`CoDel`变体称为`fq_codel`，代表“公平队列控制延迟”。

CoDel区分两种队列：

- 好队列：不显示缓冲膨胀的队列。通信突发只会导致队列延迟的暂时增加，网络链接利用率最大化。
- 坏队列：显示缓冲区膨胀的队列。通信突发会导致缓冲区填满并保持填满，从而导致利用率低和缓冲区延迟不断增加。

为了有效地防止缓冲区膨胀， 主动队列管理 （AQM）算法形式的解决方案必须能够 **<u>识别缓冲区膨胀的发生并通过部署有效的对策进行反应</u>**。

## 算法

CoDel旨在将CoDel管理的缓冲区队列中的数据包延迟控制在最小延迟下。目标是将此最小延迟保持在5毫秒以下。 如果最小延迟上升到一个太大的值，则将数据包从队列中丢弃，直到延迟降至最大级别以下。 

算法描述：

**该算法在一个间隔 （最初为100毫秒）内运行。 通过监控每个数据包的排队延迟。 每个数据包出队的时候会被转发。先计算每个包的排队延迟（数据包在队列中等待了多少时间）。在这个时间间隔内的最小 排队延迟 要存储下来。当这个时间间隔内的最后一个数据包出队时，如果该间隔的最小 排队延迟 大于5毫秒，则会丢弃此单个数据包，并缩短用于下一组数据包的时间间隔。 如果该时间间隔的最低排队延迟小于5毫秒，则转发数据包并将该时间间隔重置为100毫秒。**

**当缩短间隔时，将根据由于过多排队延迟而丢包的连续间隔数的倒数平方根来执行此操作。 间隔的顺序是 **
$$
100, \frac{100}{\sqrt{2}}, \frac{100}{\sqrt{3}}, \frac{100}{\sqrt{4}}, \frac{100}{\sqrt{5}}    ...
$$

## 内核 Fair Queue CoDel discipline 实现

实验的系统是ubuntu，内核版本5.14.14

系统默认disc查看以及驱动查看：
```shell
root@joe-ubuntu:~# sysctl -a | grep default_qdisc
net.core.default_qdisc = fq_codel

root@joe-ubuntu:~# lsmod | grep fq_codel
sch_fq_codel           20480  3
```

准则：

- 数据包按流分类。（随机模型，多个流可能在同一个hash槽上）
- 每个流都有一个CoDel管理队列。
- 流被链接到两个RR（Round Robin）链表上，因此新的流比旧的流有优先级。
- 对于给定的流，包不会被重新排序(CoDel使用FIFO)，只会丢弃头。
- 默认情况下ECN功能是开启的。
- 低内存占用，每个流的数据结构最多只占64字节。

### 驱动的安装和卸载

```c
static const struct Qdisc_class_ops fq_codel_class_ops = {
	.leaf		=	fq_codel_leaf,
	.find		=	fq_codel_find,
	.tcf_block	=	fq_codel_tcf_block,
	.bind_tcf	=	fq_codel_bind,
	.unbind_tcf	=	fq_codel_unbind,
	.dump		=	fq_codel_dump_class,
	.dump_stats	=	fq_codel_dump_class_stats,
	.walk		=	fq_codel_walk,
};

static struct Qdisc_ops fq_codel_qdisc_ops __read_mostly = {
	.cl_ops		=	&fq_codel_class_ops,
	.id			=	"fq_codel",
	.priv_size	=	sizeof(struct fq_codel_sched_data),
	.enqueue	=	fq_codel_enqueue,
	.dequeue	=	fq_codel_dequeue,
	.peek		=	qdisc_peek_dequeued,
	.init		=	fq_codel_init,
	.reset		=	fq_codel_reset,
	.destroy	=	fq_codel_destroy,
	.change		=	fq_codel_change,
	.dump		=	fq_codel_dump,
	.dump_stats =	fq_codel_dump_stats,
	.owner		=	THIS_MODULE,
};

static int __init fq_codel_module_init(void)
{
	return register_qdisc(&fq_codel_qdisc_ops);
}

static void __exit fq_codel_module_exit(void)
{
	unregister_qdisc(&fq_codel_qdisc_ops);
}
```

### 入队


### 出队

