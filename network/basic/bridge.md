# Bridge

两个netns可以通过`veth pair`连接，两个以上netns相互连接，就需要使用`bridge`了。

Linux bridge有多个端口，数据可以从任何端口进来，进来之后从哪个口出去则取决于目的MAC地址。

**网桥是一种以独立于协议的方式将两个以太网段连接在一起的方式。数据包时基于以太网地址转发的，而不是IP地址。由于转发是在L2完成的，因此所有协议都可以通过网桥透明地传输。**

## 实验

![bridge-images/bridge-test.png](bridge-images/bridge-test.png)

```bash
# 创建bridge并启用
ip link add name br0 type bridge
ip link set br0 up

# 创建netns
ip netns add ns1
ip netns add ns2
ip netns add ns3

# 创建veth-pair
ip link add veth1-ns type veth peer name veth1-br
ip link add veth2-ns type veth peer name veth2-br
ip link add veth3-ns type veth peer name veth3-br

# 将veth的一端移动到netns中
ip link set veth1-ns netns ns1
ip link set veth2-ns netns ns2
ip link set veth3-ns netns ns3

# 给netns中的veth配置IP并启用
ip netns exec ns1 ip addr add 10.0.0.1/24 dev veth1-ns
ip netns exec ns1 ip link set veth1-ns up
ip netns exec ns2 ip addr add 10.0.0.2/24 dev veth2-ns
ip netns exec ns2 ip link set veth2-ns up
ip netns exec ns3 ip addr add 10.0.0.3/24 dev veth3-ns
ip netns exec ns3 ip link set veth3-ns up

# 将veth的另一端启动并挂载到bridge上
ip link set veth1-br up
ip link set veth2-br up
ip link set veth3-br up
ip link set dev veth1-br master br0
ip link set dev veth2-br master br0
ip link set dev veth3-br master br0

# 测试连通性
ip netns exec ns1 ping 10.0.0.2
ip netns exec ns1 ping 10.0.0.3
ip netns exec ns2 ping 10.0.0.3
```

## 源码分析

`bridge`的源码在`net/bridge`下

### 数据结构

```c
#define ETH_ALEN	6		/* Octets in one ethernet addr	 */

// 网桥ID
struct bridge_id {
	unsigned char	prio[2]; 			// 网桥优先级
	unsigned char	addr[ETH_ALEN];		// MAC地址
};

// MAC地址
struct mac_addr {
	unsigned char	addr[ETH_ALEN];
};

struct net_bridge_fdb_key {
	mac_addr addr;
	u16 vlan_id;
};

// 转发数据库的记录项。网桥所学到的每个MAC地址都有这样一个记录。（记录MAC地址和网桥端口的映射关系）
struct net_bridge_fdb_entry {
	struct rhash_head		rhnode;		// fdb hash表节点
	struct net_bridge_port		*dst;	// 网桥端口

	struct net_bridge_fdb_key	key;	// MAC地址
	struct hlist_node		fdb_node;	// ？
	unsigned long			flags;
	...
	struct rcu_head			rcu;
};

// 网桥端口
struct net_bridge_port {
	struct net_bridge		*br;	// 所属网桥设备对象
	struct net_device		*dev;	// 网络接口设备对象
	struct list_head		list;	// 链表节点
	u16				port_no;		// 端口号
	...
};

struct net_bridge {
	struct net_device		*dev;			// 网络接口设备对象
	struct rhashtable		fdb_hash_tbl;	// fdb hash表（net_bridge_fdb_entry）
	struct list_head		port_list;		// 端口列表（net_bridge_port）
	struct hlist_head		fdb_list;		// ？
	...
};
```

### 模块初始化

```c
static int __init br_init(void)
{
	...
	// 注册ioctl钩子
	brioctl_set(br_ioctl_deviceless_stub);
	...
}

static int (*br_ioctl_hook) (struct net *, unsigned int cmd, void __user *arg);

void brioctl_set(int (*hook) (struct net *, unsigned int, void __user *))
{
	mutex_lock(&br_ioctl_mutex);
	br_ioctl_hook = hook;
	mutex_unlock(&br_ioctl_mutex);
}

int br_ioctl_deviceless_stub(struct net *net, unsigned int cmd, void __user *uarg)
{
	...
	switch (cmd) {
	case SIOCBRADDBR:
	case SIOCBRDELBR:
	{
		...
		if (cmd == SIOCBRADDBR)
			ret = br_add_bridge(net, buf);
		else
			ret = br_del_bridge(net, buf);
	}
	}
	...
}
```
注册ioctl响应操作，
- 添加网桥：`br_add_bridge`；
- 删除网桥：`br_del_bridge`；

### 创建 bridge
```c
int br_add_bridge(struct net *net, const char *name)
{
	struct net_device *dev;
	int res;

	// 申请网络设备
	dev = alloc_netdev(sizeof(struct net_bridge), name, NET_NAME_UNKNOWN,
			   br_dev_setup);

	...
	// 注册网络设备
	res = register_netdevice(dev);
	...
}

static const struct net_device_ops br_netdev_ops = {
	// 设置发包函数回调
	.ndo_start_xmit		 = br_dev_xmit,
	// 设置dev ioctl
	.ndo_do_ioctl		 = br_dev_ioctl,
};

void br_dev_setup(struct net_device *dev)
{
	struct net_bridge *br = netdev_priv(dev);
	...
	dev->netdev_ops = &br_netdev_ops;
	...
	br->dev = dev;
	INIT_LIST_HEAD(&br->port_list);
	INIT_HLIST_HEAD(&br->fdb_list);
	// 定时清理任务
	INIT_DELAYED_WORK(&br->gc_work, br_fdb_cleanup);
}

int br_dev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct net_bridge *br = netdev_priv(dev);

	switch (cmd) {
	...
	case SIOCBRADDIF:
	case SIOCBRDELIF:
		return add_del_if(br, rq->ifr_ifindex, cmd == SIOCBRADDIF);

	}
	...
}
```

注册dev ioctl响应操作，
- 给网桥添加端口：`br_add_if`；
- 给网桥删除端口：`br_del_if`；

### 给网桥添加端口

```c
int br_add_if(struct net_bridge *br, struct net_device *dev,
	      struct netlink_ext_ack *extack)
{
	struct net_bridge_port *p;
	// 创建新的网桥端口
	p = new_nbp(br, dev);
	// 将网桥端口挂到port_list链表上
	list_add_rcu(&p->list, &br->port_list);
	// 插入net_bridge_fdb_entry结点到fdb_hash_tbl
	br_fdb_insert(br, p, dev->dev_addr, 0);
}
```
