# cilium-operator

## 职责

1. 给节点分配podcidr
2. 重启coredns
3. 其他：LB IPAM、各种crd的GC、ingress/gateway/loadbalancer controller

## IPAM

1. kubernetes

3. cluster-pool

## 重启coredns

当coredns不存在CiliumEndpoint时，重启coredns。
