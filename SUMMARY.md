# Summary

* [Introduction](README.md)

* [gitbook搭建](./gitbook/index.md)

* [网络](./network/index.md)
  * [网络基础](./network/basic/index.md)
    * [内核是如何接收一个网络包的](./network/basic/recv_one_package.md)
    * [内核DMA机制](./network/basic/dma.md)
    * [veth设备](./network/basic/veth.md)
    * [tun/tap设备](./network/basic/tun-tap.md)
    * [bridge](./network/basic/bridge.md)
    * [vxlan](./network/basic/vxlan.md)
    * [邻居子系统](./network/basic/neighbor.md)
    * [路由子系统](./network/basic/route.md)

  * [负载均衡](./network/load_balance.md)
    * [LVS](./network/lb/LVS.md)
    * [Maglev](./network/lb/Maglev.md)

  * [流量控制](./network/traffic_control/tc.md)
    * [缓冲膨胀](./network/traffic_control/bufferbloat.md)
    * [公平队列控制延迟](./network/traffic_control/fq_codel.md)

* [Kubernetes](./Kubernetes/index.md)
  * 工作负载
    * [Jobs](./Kubernetes/Jobs.md)
    * [CronJob](./Kubernetes/CronJob.md)
  * [CRD](./Kubernetes/CRD.md)
  * [Operator](./Kubernetes/Operator.md)


* [OVS](./OVS/index.md)

* [eBPF](./eBPF/index.md)

* [XDP](./XDP/index.md)

* [cilium](./cilium/index.md)

* [八股](./BAGU/index.md)

* [杂项]()
  * [centos安装v2ray](./misc/centos-install-v2ray.md)
  
  * [docker/containerd设置代理实现从外网拉取镜像](./misc/docker-pull-use-proxy.md)
  
  * [编译安装ovs](./misc/install-ovs.md)
  
  * [k8s 将docker切换为containerd](./misc/docker-to-containerd.md)


* [时间线](KEEP.md)
