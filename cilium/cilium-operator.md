# cilium-operator

## 职责

1. 给节点分配podcidr
2. 重启coredns
3. 其他：LB IPAM、各种crd的GC、ingress/gateway/loadbalancer controller

## IPAM

### 1. kubernetes

1. kube-controller-manager启动时会注册一个名称为 `nodeipam` 的controller；
2. nodeipam_controller监听到Node事件，会为其分配podCIDRs；

默认情况下，kube-controller-manager会按24位mask为节点预留256个IP。可以用通过指定--node-cidr-mask-size来调整。

```yaml
kind: ClusterConfiguration
networking:
  podSubnet: 10.234.1.0/24
controllerManager:
  extraArgs:
    node-cidr-mask-size: "26"
```

```yaml
kind: Node
spec:
  podCIDR: 10.234.1.64/26
  podCIDRs:
  - 10.234.1.64/26
```

伪代码实现：

```go
for i := 0; i < 3; i++ {
    nextCIDR := toCIDR(toInt(10.234.1.0) | (i << 6), 26)
}
// 10.234.1.0/26
// 10.234.1.64/26
// 10.234.1.128/26
```

clusterCIDR支持多个子网吗？不支持，k8s双栈模式下，只支持一个ipv4子网和一个ipv6子网。

```go
// [kube-controller-manager/app/core.go](https://github.com/kubernetes/kubernetes/blob/master/cmd/kube-controller-manager/app/core.go#L809)
if len(clusterCIDRs) > 2 {
    return nil, fmt.Errorf("length of clusterCIDRs is:%v more than max allowed of 2", len(clusterCIDRs))
}
```

### 2. cluster-pool

1. cilium-agent启动时，会创建CiliumNode资源；
2. cilium-operator为CiliumNode资源分配podCIDRs；

比如clusterCIDR为 `10.100.0.0/16`，且要求每个节点最多预留 `128` 个IP。

```yaml
kind: CiliumNode
spec:
  ipam:
    podCIDRs:
    - 10.100.0.128/25
```


## 重启coredns

当coredns不存在CiliumEndpoint时，重启coredns。