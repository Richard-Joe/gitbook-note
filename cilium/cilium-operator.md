# cilium-operator

## 职责

1. 给节点分配podcidr
2. 重启coredns
3. 其他：LB IPAM、各种crd的GC、ingress/gateway/loadbalancer controller

## IPAM

### 1. kubernetes

1. kube-controller-manager启动时会注册一个名称为 `nodeipam` 的controller

### 2. cluster-pool

1. cilium-agent启动时，会创建CiliumNode资源；
2. cilium-operator为CiliumNode资源分配podCIDRs；

比如集群的podcidr为 `10.100.0.0/16`，且要求每个节点最多预留 `128` 个IP。

```yaml
kind: CiliumNode
spec:
  ipam:
    podCIDRs:
    - 10.100.0.128/25
```

伪代码实现：

```go
for i := 0; i < 10; i++ {
    nextCIDR := toCIDR(toInt(10.100.0.0) | (i << 7), 25)
}
// 10.100.0.0/25
// 10.100.0.128/25
// 10.100.1.0/25
// 10.100.1.128/25
```

## 重启coredns

当coredns不存在CiliumEndpoint时，重启coredns。
