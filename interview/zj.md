## 1. prepare

### 1.1. 网络

1. 网络特长
2. vlan问题，access和trunk区别（两种端口模式，access端口只能属于一个vlan，trunk端口可以属于多个vlan，trunk可以收发多个vlan的报文）
3. ospf和bgp等，建立邻居有什么区别

两者都是动态路由协议，路由器创建并维护一个数据库，相邻路由器共享路由表信息，如果存在到外部网络多条路径，路由器会计算到外部网络的最优路径。

- ospf：内部网关协议，IGP，工作在一个AS内，常用于LAN；用来发现、计算路由；使用**路径带宽**作为决定路径选择的主要因素；OSPF的**收敛速度**比BGP快；基于**IP协议**，协议号89；Dijkstra算法；
- bgp：外部网关协议，EGP，工作在AS间，常用于互联网冗余、WAN；用来传递、控制路由；决定路径的因素有很多：路由weight值，到目的网络的最短路径等；基于**TCP协议**，端口号179；最佳路径算法；

a、OSPF的数据包有五种，分别是Hello包、DBD包、LSR、LSU、LSACK
BGP的数据包四种，分别是Open、Keeplive、Update、Notification

b、OSPF状态机一般有七个，分别是down、init、two-way、exstart、exchange、loading、full
BGP的状态机有六个，分别是Idle、Connect、Active、Opensent、Openconfirm、Establish

c、OSPF选路规则有五个人为控制的只有cost，BGP有十二个，管理员可以灵活地控制
d、OSPF建邻条件很多，有10个；BGP的建邻只需要邻居间可达、路由表中有邻居路由即可
e、两种协议的追求不同，OSPF属于IGP协议，追求的是收敛快、选路佳、占用资源少；BGP属于EGP，追求的是可靠性、可控性强，还有就是以一个AS为一跳，即AS-BY-AS
f、防环机制不同，OSPF的防环机制主要依靠它的算法本身还有区域间水平分割等，BGP的防环机制主要有IBGP水平分割和EBGP水平分割

4. NAT场景：静态nat、动态nat、NAPT；将多个私有地址映射到少量的共有IP？

- 静态nat：将一个或一组内部地址转换为固定的外部地址；
- 动态nat：指外部地址有多个，转换时可以轮询使用；
- NAPT：网络地址端口转换；NAPT不仅转换IP包中的IP地址，还对IP包中TCP和UDP的Port进行转换；
- MASQUERADE（伪装）会自动获取网卡上的 IP 地址，而不用像 SNAT 那样需要使用 --to-source 指定，当 IP 发生变化时不需要手动改动。

5. 选路算法（迪杰斯特拉(Dijkstra)算法）
6. 三次握手和四次挥手
7. tcp/udp区别是什么？

- tcp：面向连接；一对一；可靠性；拥塞控制；头部20个字节；流式传输，没有边界；根据MSS在TCP层分片；
- udp：无连接；一对一，一对多，多对多；不可靠；拥塞不影响发包速率；头部8个字节；按包传输，有边界；根据MTU在IP层分片；

 tcp面向链接，udp不是
 tcp开销大，udp开销小
 tcp有拥塞控制，可以慢开始，拥塞避免，快重传和快恢复，udp没有，不管网络是否拥塞，udp客户端都可以一直发
 tcp数据完整，按时，按序到达，udp尽力满足
 tcp延时高，适用于数据传输准确度要求高的场合，udp适用于数据传输量大，实时性要求高的场合

8. TCP 协议四个方面的缺陷：

- 升级 TCP 的工作很困难；
- TCP 建立连接的延迟；
- TCP 存在队头阻塞问题；（TCP 层必须保证收到的字节数据是完整且有序的，如果序列号较低的 TCP 段在网络传输中丢失了，即使序列号较高的 TCP 段已经被接收了，应用层也无法从内核中读取到这部分数据。）
- 网络迁移需要重新建立 TCP 连接；

9. udp如何实现可靠传输？

TCP 流量控制是通过让「接收方」告诉「发送方」，它（接收方）的接收窗口有多大，从而让「发送方」根据「接收方」的实际接收能力控制发送的数据量。

QUIC协议（HTTP/3）如何解决的：

- 可靠传输：1. Packet Header 。 2. QUIC Frame Header。保证了数据包有序性，有重传机制；
- 解决队头阻塞问题：QUIC 给每一个 Stream 都分配了一个独立的滑动窗口，这样使得一个连接上的多个 Stream 之间没有依赖关系，都是相互独立的，各自控制的滑动窗口。
- 流量控制：Stream 级别的流量控制；Connection 流量控制；
- 拥塞控制： QUIC 可以随浏览器更新，QUIC 的拥塞控制算法就可以有较快的迭代速度。可以针对不同的应用设置不同的拥塞控制算法；默认使用了 TCP 的 Cubic 拥塞控制算法。
- 更快建立连接：QUIC 使用的是 TLS1.3，因此仅需 1 个 RTT建立首次连接；第二次连接只需0-RTT。
- 如何迁移连接：没有用四元组，QUIC 通过 连接ID 来标记通信的两个端点。

10. 内核是如何收包的？

- 网卡中的信号转换模块，将信号转换到网卡中的缓冲区；
- 网卡通过DMA将数据包复制到内核缓冲区（ring buffer）；
- 网卡会向CPU发起一个硬中断，通知CPU有数据到达。（（由网卡驱动程序注册）硬中断处理函数做的事情非常少，它把napi poll挂到当前cpu的sd上，触发一个收包软中断，就结束了）
- 内核线程来执行收包软中断处理函数，调用网卡驱动注册的poll函数，读取ring buffer中的数据，经过GRO优化（分片重组），RPS优化（将数据包负载均衡到CPU，入队backlog），这时候数据包已经是协议栈能够处理的sk_buff。
- ip_rcv，netfilter，tcp/udp rcv，送入到socket的接收buffer。

11. 网络性能优化

- 调整ring buffer
- 开启网卡多队列
- 单队列网卡怎么办？开启RPS/RFS，RPS（把数据包均衡到不同的 CPU），RFS（将相同 flow 的包送到相同的 CPU 进行处理，充分利用CPU cache）
- 协议栈呢？TCP的半连接队列/全连接队列，SYN重传次数，SYN攻击（开启tcp_syncookies）

### 1.2. 操作系统

1. 进程和线程区别
2. 堆和栈，分别存储在哪里
3. 进程死锁的四大条件

4. 从网络方向提升系统性能？

从OSI七层模型：

- 物理层：网卡（ring_buffer）
- 链路层：交换机（MTU）
- 网络层：路由器（路由信息，算法）
- 传输层：TCP（半连接、全连接队列，tcp fast open）、UDP
- 应用层：nginx（进程数，连接数）

5. 进程栈大小是多少？如何变成实现栈溢出？（8M，ulimit命令可以看；1.递归调用函数、2.分配大量局部变量）
6. 如何编程算出L1、L2、L3 cache大小？


### 1.3. 算法

1. 两个链表，求第一个公共节点。
2. 堆排和快排
3. 读入一个字符串str，输出字符串str中的连续最长的数字串。（例如：输入abcd12345ed125ss123456789aa123456  输出123456789）
4. 有N张卡牌堆成一摞，每张卡牌上都会有一个整数标记其分数。

现有两个人要交替从牌堆顶拿牌，每次至少拿一张，至多拿M张，直到牌堆被拿完。

每个人拿至手中的牌的分数和即为其最终得分。假设两个人都会采取最佳策略拿牌来使自己的得分最大化，请问先手拿牌的人的得分为多少？

输入描述：
输入第一行是样例个数；

对于每个样例，第一行是N，M，第二行有N个整数，代表牌堆顶到牌堆底N张牌的分数。

0 < N, M < 1,000,000,  每张牌的分数在-100和100之间

输出描述：
每个样例，输出一个整数代表先手得分。

示例1

输入
2
4 2
1 1 1 1
5 2
3 -4 1 1 7

输出
2
6

### 1.4. 语言篇 golang


## 2. 问题复盘（2023-05-17）

1. ebpf如何保证安全的

eBPF可以应用在安全、跟踪、性能分析、网络数据包处理、观测、监控等不同领域。
Linux kernel为了保证eBPF程序的安全性，在加载的时候添加了许多限制，包括：（验证器）

- 指令限制：每个程序的最大指令限制为 4096 条 BPF 指令；（到5.1版本后，改为 100万条）
- 不能有循环：保证程序能终止；
- 不得使用未初始化变量；
- tail call嵌套有上限：33 个调用上限
- 资源上的限制（比如内存）；eBPF程序大小限制；多可以消耗 8KB 的堆栈空间；
- 根据程序类型，限制可以调用哪些内核函数、可以访问哪些数据结构、是否可以访问网络数据包内容；

2. 如何优化ipvs性能（dpdk-dpvs）

3. SDWAN网络架构 ？

4. SDWAN如何实现选路的？

- 链路质量选路：可以指定应用的主备链路，当主链路质量不满足应用SLA要求时，应用流量自动进行主备链路的切换。
- 负载均衡选路：当两个站点之间存在多条链路时，系统按照链路的接口带宽比例，对流量进行同比例的逐流负载分担。
- 带宽选路：通过设置链路的带宽占用阈值，当链路带宽占用超过阈值时，流量选择其他链路进行转发。即基于链路的带宽占用率，提供过载保护功能，避免链路超负荷使用。
- 应用优先级选路：为应用设置优先级，当链路拥塞时，低优先级应用切换到其他链路以避让高优先级的应用。

5. 负载均衡有哪几层？如何实现

- 三层：基于IP分发，等价路由（等价多路径，ECMP）
- 四层：基于TCP/UDP分发，iptables、LVS
- 七层：基于HTTP分发，Nginx

6. 内核如何实现网桥的？

- 传统HUB：每收到一个数据包，就在其所有端口广播，由接收主机来判断数据包是不是给自己的。浪费网络资源。
- 网桥：学习MAC地址，建立 [MAC地址--端口] 对照表。发包时，查表来选择端口发送。

- 我们使用ip link add br0 type bridge等命令时，内核帮我们注册一个net_device，并执行了设备初始化函数。比如设置了发包函数br_dev_xmit。
- 我们给网桥添加端口（interface）时，内核给这个端口设备，设置收包函数br_handle_frame。

- br_handle_frame：如果是本机mac地址，执行local_in hook，进IP协议栈了。执行 pre hook，查fdb表，找得到就指定端口转发br_forward，找不到就所有端口转发br_flood。执行 forward hook，然后post_routing hook，最后dev_queue_xmit发包。
- br_dev_xmit：根据skb目的MAC地址匹配，广播地址就br_flood，多播地址就br_multicast_flood，fdb表能查到就br_forward

STP：生成树协议。桥接设备之间通过使用网桥协议数据单元（Bridge Protocol Data Unit，BPDU）交换各自状态信息。避免网桥环路。

7. 讲一下netfilter

- 地址族和hook：arp、bridge、ip、netdev
- 拿IP层举例：五个hook点

8. 讲一下NAT

- SNAT hook 挂在两个位置：LOCAL_IN、POST_ROUTING
- DNAT hook 挂在两个位置：PRE_ROUTING、LOCAL_OUT
- 流程：执行hook（iptables），匹配和修改tuple，执行NAT操作。

9. 讲一下连接跟踪

基本元素：

- 根据**元组**（五元组），来跟踪一条连接；
- 解释下**连接**，非TCP/UDP中的连接概念。ICMP也会被跟踪，但不是所有协议都能被跟踪。目前支持的协议：TCP、UDP、ICMP、DCCP、SCTP、GRE。
- **连接跟踪表**：hash表，用于连接插入，查找等。key就是根据tuple来计算。
- GC：回收。
- Zone：多租户。多租户环境下，无法只用 tuple 来区分 CT

基本流程：

- nf_conntrack_in：PRE_ROUTING，LOCAL_OUT。负责创建CT，记录到skb中。
- nf_conntrack_confirm：LOCAL_IN、POST_ROUTING。负责确认CT，将CT插入到hash表中。

nf_conntrack_confirm为什么放在最后处理？（关闭/开启软中断、对hash槽加解锁，影响性能）

10. cilium如何使用ebpf实现连接跟踪的

首先如果卸载netfilter，cilium缺什么功能？（1. NAT；2. NAT依赖连接跟踪）

- 内核里的CT基于netfilter hooks
- cilium的CT基于BPF hooks（lxc，host，netdev，overlay）

流程：

- 比如，service ip转换为pod ip，执行DNAT时，用到连接跟踪。
- 首包，根据service信息选择一个backend，创建CT，记录backend_id到CT中，并记录CT到map中。
- flow包，根据tuple信息，从bpf map中查ct，根据记录的backend_id来执行DNAT操作。

算法题：
1. https://leetcode.cn/problems/ugly-number/description/

```c++
class Solution {
public:
    bool isUgly(int n) {
        if (n <= 0) {
            return false;
        }
        vector<int> factors = {2, 3, 5};
        for (int factor : factors) {
            while (n % factor == 0) {
                n /= factor;
            }
        }
        return n == 1;
    }
};
```

2. https://leetcode.cn/problems/ugly-number-ii/description/

```c++
class Solution {
public:
    int nthUglyNumber(int n) {
        int i=0,j=0,k=0;
	    int result[n];
	    result[0]=1;
	    for(int cnt=1;cnt<n;cnt++) {
		    result[cnt]=min(result[i]*2,min(result[j]*3,result[k]*5));
		    if(result[cnt]==result[i]*2)i++;
		    if(result[cnt]==result[j]*3)j++;
		    if(result[cnt]==result[k]*5)k++;
        }
        return result[n-1];
    }
};
```

## 3. 问题复盘（2023-05-29）

1. 如何阅读内核代码
2. 项目
3. 算法

迷宫，二维数组，1代表墙，0代表过道，给起点和终点，问能否逃脱？

回溯解决。（空间复杂度和时间复杂度）

回溯的时间复杂度如何计算？

## 4. 问题复盘（2023-07-25）

1. 流控怎么做的？为什么不用TC？
2. ebpf map 有哪些类型？
3. ebpf map如何共享的？（文件描述符，pin方式，需要看lib_bpf代码）
4. VPN实现；
5. 内核收发包流程；
6. 中断和软中断；
7. 收包流程中，VLAN和Bridge，谁先谁后？
8. 算法题：小于n的最大数。n = 23121，使用数组[2,4,9]中的元素组合小于n的最大数22999。（DFS，贪心+二分）