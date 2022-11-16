# 邻居子系统

给一个 L3 地址，找到对应的 L2 地址，称为 “L3地址的解析”。这个过程是由 `邻居协议` 完成的。

- IPv4 中使用 ARP （Address Resolution Protocol，地址解析协议）
- IPv6 中使用 NDP （Neighbor Discovery Protocol，邻居发现协议）

## 1. 概念

### 1.1. 什么是邻居

连接在同一个LAN上，且拥有相同的 L3 网络配置，那么就互为邻居。

### 1.2. 为什么需要邻居协议？

L3 是逻辑地址，它可以被随意改变。某些情况下，L3 地址到L2 地址的映射关系会发生变化。比如 动态配置（DHCP）、替换主机NIC（L2 变化）、L3 地址的移动（高可用）。

### 1.3. 什么情况下不需要邻居协议？

有时候 L3 地址到 L2 地址的映射不需要任何协议就能完成：

- 点对点介质连接
- 多播地址（可参考函数arp_mc_map、ip_eth_mc_map中的实现）
- 广播地址（FF:FF:FF:FF:FF:FF）

多播 IP -> MAC 规则：（比如 239.1.1.1 ->  01:00:5E:01:01:01）

- 最高 24 位指定由IANA分配的静态地址 01:00:5E
- 低 24 位的最高位设为 0
- 剩余的低 23 位直接复制相应IP地址的低 23 位

### 1.4. 请求和应答

请求（Neighbor Solicitation）：可以单播、多播、广播。
应答（Neighbor Advertisement）：通常为单播。特殊情况，可广播。

## ARP高速缓存

## ARP分组格式

## ARP代理

## RARP