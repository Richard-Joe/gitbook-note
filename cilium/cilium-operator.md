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

cluster-pool模式下，集群的podCIDR是可以支持多个ipv4子网的。比如：

```
--cluster-pool-ipv4-cidr='192.168.1.0/26,192.168.3.0/26,192.168.5.0/26'
--cluster-pool-ipv4-mask-size='26'
```

那这样也只是单节点上所有Pod IP处于同一个网段，可以支持分布到多个网段吗？

cluster-pool不支持，但multi-pool支持。

### 3. multi-pool

multi-pool 允许Pod或者某个namespace下的pod，从指定的IPAM池中分配IP。

集群初始化的时候，可以自动为节点创建默认的IPAM池(default)，比如：

```
--auto-create-cilium-pod-ip-pools='default=ipv4-cidrs:10.20.30.0/27,192.168.1.0/27,200.200.199.0/27;ipv4-mask-size:27'
```

创建IPAM池，并创建Pod指定annotation为该池：

```yaml
apiVersion: cilium.io/v2alpha1
kind: CiliumPodIPPool
metadata:
  name: podippool-2
spec:
  ipv4:
    cidrs:
    - 100.100.0.0/16
    maskSize: 24
```

运行效果：
```
NAMESPACE            NAME                                       READY   STATUS    RESTARTS         AGE     IP               NODE               NOMINATED NODE   READINESS GATES
default              echo-1-84f6ccbccc-6tztq                    1/1     Running   0                16m     100.100.0.144    c1-worker          <none>           <none>
default              echo-2-5dfb5c57f9-2ggc5                    1/1     Running   0                16m     100.100.1.167    c1-worker2         <none>           <none>
kube-system          coredns-5d78c9869d-9bq58                   1/1     Running   0                72m     10.20.30.27      c1-worker2         <none>           <none>
kube-system          coredns-5d78c9869d-rsc5v                   1/1     Running   0                72m     200.200.199.21   c1-control-plane   <none>           <none>
```

观察CiliumNode配置情况，c1-worker2节点从IPAM池(podippool-2)中获取CIDR（100.100.1.0/24），echo-2和coredns的pod ip处于不同网段。

```
kind: CiliumNode
metadata:
  name: c1-worker
spec:
  ipam:
    pools:
      allocated:
      - cidrs:
        - 192.168.1.0/27
        pool: default
      - cidrs:
        - 100.100.0.0/24
        pool: podippool-2

kind: CiliumNode
metadata:
  name: c1-worker2
spec:
  ipam:
    pools:
      allocated:
      - cidrs:
        - 10.20.30.0/27
        pool: default
      - cidrs:
        - 100.100.1.0/24
        pool: podippool-2
```

## 重启coredns

当coredns不存在CiliumEndpoint时，重启coredns。
