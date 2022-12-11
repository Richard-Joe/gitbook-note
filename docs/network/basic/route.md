# 路由子系统

## 1. 概念

- 互联网服务提供商（ISP）：提供访问互联网的公司或组织。

- 不可路由地址：10.0.0.0/8、172.16.0.0/16~172.31.0.0/16、192.168.0.0/16、127.0.0.0/8

- Scope：在linux中，路由的scope表示到目的网络的距离；IP的scope表示该IP距离本地主机有多远。

```c
enum rt_scope_t {
	RT_SCOPE_UNIVERSE=0,	// 下一跳网关的路由项
/* User defined values  */
	RT_SCOPE_SITE=200,		// 
	RT_SCOPE_LINK=253,		// 目的地为本地网络地址的路由项；子网广播地址的路由项
	RT_SCOPE_HOST=254,		// 本地接口的路由项
	RT_SCOPE_NOWHERE=255	// 非法的scope，没有目的地的路由项
};
```

- 默认网关：当目的地没有确定的路由时使用的路由。

- 定向广播（Directed Broadcasts）：定向广播的目的地是远端子网的广播地址。由于无法区分恶意还是善意，Linux路由子系统不允许丢弃任何定向广播，管理员可以使用过滤系统来做处理。

- 主地址和辅助地址：每个接口上可以配置多个主地址和多个辅助地址。对于一个特定的子网掩码，只能有一个主地址。

> 1. 主地址可以降低CPU开销
> 2. 当删除一个主地址时，所有关联的辅助地址也被删除。但通过配置/proc选项（net.ipv4.conf.eth0.promote_secondaries），使得在当前主地址被删除时可以将辅助地址提升为主地址。
> 3. 当主机为本地生成的流量选择IP地址时，只考虑主地址。

- 路由表

默认情况下，Linux使用两张路由表：
1. 一张表用于本地地址。从该表查找成功表明封包要交给主机自己。
2. 一张表用于所有其他的路由。可以手工配置或由路由协议动态插入。

路由类型：

```c
enum {
	RTN_UNSPEC,
	RTN_UNICAST,		/* Gateway or direct route	*/
	RTN_LOCAL,			/* Accept locally		*/
	RTN_BROADCAST,		/* Accept locally as broadcast, send as broadcast */
	RTN_ANYCAST,		/* Accept locally as broadcast, but send as unicast */
	RTN_MULTICAST,		/* Multicast route		*/
	RTN_BLACKHOLE,		/* Drop				*/
	RTN_UNREACHABLE,	/* Destination is unreachable   */
	RTN_PROHIBIT,		/* Administratively prohibited	*/
	RTN_THROW,			/* Not in this table		*/
	RTN_NAT,			/* Translate this address	*/
	RTN_XRESOLVE,		/* Use external resolver	*/
	__RTN_MAX
};
```

路由缓存：Linux同时使用了路由缓存和路由表。

1. 路由缓存查找：从一张简单hash表中寻找完全匹配项。
2. 路由表查找：基于最长前缀匹配（Longest Prefix Match，LPM）算法。

缓存垃圾回收：

1. 同步回收：缓存数量达到阈值；内存紧张。
2. 异步回收：定时器实现。过期应当被删除；没有过期但内存紧张。

封包接收和发送都需要路由表。一般一个封包仅需要进行一次查找，但有些情况（IP-over-IP）可能要多做一次路由查找。

## 2. 高级路由