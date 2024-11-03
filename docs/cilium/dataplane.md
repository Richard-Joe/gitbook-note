# Cilium 数据面

1. `raw` 表：主要用于禁用连接跟踪。适用于需要极低延迟或防止某些数据包被连接跟踪的场景。

```shell
iptables -t raw -A PREROUTING -p tcp --dport 80 -j NOTRACK
```

2. `mangle` 表：用于修改数据包的标头（header）信息或其他内容。

	- 修改数据包标头信息： 可以更改 IP 数据包的头部字段（如 ToS、TTL 等）。
	- 数据包标记（Marking）： 标记数据包，以便在后续的处理过程中（如路由、过滤、流量控制）能够识别和使用这些标记。
	- 质量控制（QoS）： 用于设置 QoS 标记，优先处理某些流量。
	- 改变数据包的其他属性： 如数据包的长度、校验和等。

```shell
iptables -t mangle -A PREROUTING -p tcp --dport 80 -j TOS --set-tos Minimize-Delay
iptables -t mangle -A PREROUTING -s 192.168.1.100 -j MARK --set-mark 1
iptables -t mangle -A POSTROUTING -j TTL --ttl-dec 1
```

3. `filter` 表：主要用于允许或拒绝数据包通过。

```shell
iptables -A INPUT -p tcp --dport 80 -j ACCEPT
iptables -A INPUT -s 192.168.1.100 -j DROP
iptables -A FORWARD -p tcp --dport 22 -j ACCEPT
```

![cilium](images/cilium.png)