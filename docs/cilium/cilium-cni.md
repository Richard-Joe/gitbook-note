# Cilium plugins -- cni

version: cilium-1.17.0

## containerd

containerd 在创建 sandbox 时，会调用 cni 插件配置 pod 的网络命名空间。

实现在 RunPodSandbox 接口：
1. 如果开启了 hostNetwork；
2. 创建net namespace；（/var/run/netns/cni-fc979a8f-f786-8148-5220-3ab3d31c1937）
3. 执行 cni plugin 命令（cilium-cni）；
4. 将返回的结果（IP）设置到sandbox元数据中；
5. 将sandbox元数据，写入DB（/var/lib/containerd/io.containerd.metadata.v1.bolt/meta.db）

## cilium-cni

cni 插件需要实现 CNIFuncs 回调：
```go
type CNIFuncs struct {
	Add    func(_ *CmdArgs) error
	Del    func(_ *CmdArgs) error
	Check  func(_ *CmdArgs) error
	GC     func(_ *CmdArgs) error
	Status func(_ *CmdArgs) error
}
```

cilium-cni实现了 Add、Del、Check。

### Add

1. 解析入参：
- args="IgnoreUnknown=1;K8S_POD_NAMESPACE=default;K8S_POD_NAME=echo-qjx-699b4f9b85-d6nsm;K8S_POD_INFRA_CONTAINER_ID=d879d1a7c3f12c453e3c444b923ff638eddb6f60538e5af51cb48fc227397ad7;K8S_POD_UID=ea06a149-5a11-4a68-90ff-5ff71fe21f33" 
- containerID=d879d1a7c3f12c453e3c444b923ff638eddb6f60538e5af51cb48fc227397ad7 
- file-path=/opt/cni/bin 
- ifName=eth0
- netconf="..."
2. 从cilium-agent获取配置：（IPAM模式、DataPath模式(Veth、Netkit)、GRO、GSO、ip_local_reserved_ports）
3. 删除命名空间中的eth0网口；
4. （取决于IPAM模式）请求cilium-agent （PostIPAM）分配IP接口；
5. （取决于DataPath模式）创建veth-pair设备，主机侧命名为lxc-xxx，命名空间侧命名ifName。设置GRO、GSO参数。
6. 给命名空间内eth0口配置IP、路由、Rule。
7. 请求cilium-agent （PutEndpointID）创建CiliumEndpoint。
8. 根据endpoint的响应的mac地址，替换命名空间内eth0的mac。
9. 返回结果：CNIVersion、Interfaces、IPs、Routes、DNS等信息。

### Del

1. 如果cilium-agent在线，请求cilium-agent （DeleteEndpoint）删除CiliumEndpoint；
2. 如果连接不上cilium-agent，将待删除的请求入队，就是写文件到目录（/var/run/cilium/deleteQueue）。（cilium-agent启动时会消费队列）
3. 删除veth-pair设备。

### Check

1. 请求cilium-agent （GetEndpointIDHealthz）看 Endpoint是否健康；
2. 校验netns中的网络接口存在且IP正确。
