# 实验

## 待修正

```bash
# 创建netns
ip netns add ns1
ip netns add ns2

# 创建veth-pair
ip link add veth1 type veth peer name veth2

# 将veth移动到netns中
ip link set veth1 netns ns1
ip link set veth2 netns ns2

# 给veth配置IP并启用
ip netns exec ns1 ip addr add 6.6.6.1/32 dev veth1
ip netns exec ns1 ip link set veth1 up
ip netns exec ns2 ip addr add 6.6.6.2/32 dev veth2
ip netns exec ns2 ip link set veth2 up

# 创建tun
ip tuntap add dev tun1 mode tun
ip tuntap add dev tun2 mode tun

# 将tun移动到netns中
ip link set tun1 netns ns1
ip link set tun2 netns ns2

# 给tun配置IP并启用
ip netns exec ns1 ip addr add 2.2.2.2/32 dev tun1
ip netns exec ns1 ip link set tun1 up
ip netns exec ns2 ip addr add 9.9.9.9/32 dev tun2
ip netns exec ns2 ip link set tun2 up

# 修改路由出口为veth
ip netns exec ns1 ip route add 9.9.9.9 via 6.6.6.1
ip netns exec ns2 ip route add 2.2.2.2 via 6.6.6.2
ip netns exec ns1 ip route change 10.0.0.0/24 via 0.0.0.0 dev veth1
ip netns exec ns2 ip route change 10.0.0.0/24 via 0.0.0.0 dev veth2

# 测试ns1和ns2的连通性
ip netns exec ns1 ping 9.9.9.9
ip netns exec ns2 ping 2.2.2.2
```

