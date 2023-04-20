# Summary

* [Introduction](README.md)

* [gitbook搭建](./gitbook/index.md)

* [网络]()
  * [网络基础](./network/basic/index.md)
    * [内核是如何接收一个网络包的](./network/basic/recv_one_package.md)
    * [内核DMA机制](./network/basic/dma.md)
    * [veth设备](./network/basic/veth.md)
    * [tun/tap设备](./network/basic/tun-tap.md)
    * [Bridge](./network/basic/bridge.md)
    * [VXLAN](./network/basic/vxlan.md)
    * [GRE](./network/basic/gre.md)
    * [邻居子系统](./network/basic/neighbor.md)
    * [路由子系统](./network/basic/route.md)
    * [netfilter](./network/basic/netfilter.md)

  * [负载均衡](./network/lb/LB.md)
    * [LVS](./network/lb/LVS.md)
      * [实验](./network/lb/lvs-test.md)
    * [Maglev](./network/lb/Maglev.md)

  * [流量控制](./network/traffic_control/tc.md)
    * [缓冲膨胀](./network/traffic_control/bufferbloat.md)
    * [公平队列控制延迟](./network/traffic_control/fq_codel.md)

* [Kubernetes]()
  * [Jobs](./Kubernetes/Jobs.md)
  * [CronJob](./Kubernetes/CronJob.md)
  * [CRD](./Kubernetes/CRD.md)
  * [Operator](./Kubernetes/Operator.md)
  * [kubelet](./Kubernetes/kubelet.md)
  * [Informer](./Kubernetes/Informer.md)

* [OVS](./OVS/index.md)

* [eBPF](./eBPF/index.md)

* [XDP](./XDP/index.md)

* [cilium](./cilium/index.md)

* [Prometheus](./Prometheus/prometheus.md)

* [内核]()
  * [进程调度](./kernel/进程调度.md)
    * [CFS](./kernel/ps-1-cfs.md)
    * [EEVDF](./kernel/ps-2-eevdf.md)
  * [内存管理](./kernel/内存管理.md)
    * [将虚拟地址转换为物理地址](./kernel/mm-1-va2pa.md)
    * [物理内存模型](./kernel/mm-2-physical-memory-model.md)
    * [memblock](./kernel/mm-3-memblock.md)
  * [系统调用]()
    * [fork](./kernel/syscall-fork.md)
  * [Kdump](./kernel/kdump.md)
  * [coredump](./kernel/coredump.md)
  * [tcpdump](./kernel/tcpdump.md)
  * [Kprobes](./kernel/kprobe.md)
  * [Systemtap](./kernel/systemtap.md)
  * [RCU](./kernel/rcu.md)

* [共识算法]()

* [一致性算法]()
  * [Gossip](./consistency/gossip.md)
  * [MemberList](./consistency/memberlist.md)

* [八股]()
  * [老一套](./BAGU/0_classic.md)
  * [一致性hash](./BAGU/1_consistent_hash.md)
  * [Golang](./BAGU/2_golang.md)
  * [Redis](./BAGU/3_redis.md)

* [杂项]()
  * [centos安装v2ray](./misc/centos-install-v2ray.md)
  * [docker/containerd设置代理实现从外网拉取镜像](./misc/docker-pull-use-proxy.md)
  * [编译安装ovs](./misc/install-ovs.md)
  * [k8s 将docker切换为containerd](./misc/docker-to-containerd.md)
  * [编译内核](./misc/compile-kernel.md)

* [时间线](KEEP.md)
