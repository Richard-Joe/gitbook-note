# 问题复盘

## 6.8

- 介绍项目；
- 多集群网络如何实现的？
- k8s 调度器；
- iptables/ipvs/cilium；
- kube-proxy原理；
- cni开发流程，实现原理；
- cni选型；以及性能测试；
- cilium实现原理；
- kubelet的三种探针；
- 为什么存活探针一开始要设置为true？
- operator有哪几种脚手架？（kube-builder，operator-sdk、kudo）
- client-go的原理；
- 对称加密和非对称加密的区别；
- HTTPS中间人攻击问题；
- 用户态如何转换到内核态？有哪些场景？
- 你了解系统调用有哪些？malloc
- fork是系统调用吗？（clone系统调用实现了fork）

## 6.12

- 聊项目
- k8s中固定IP如何做？
- pod作为server端，长链接方式，节点挂掉，如果做迁移？
- pod依赖另外一个cache系统（比如redis）时，如何部署？
- pod的流量控制如何做？
- k8s的资源保障的三个级别？
- k8s各个组件的功能？
- TCP的四次挥手过程
- 如何把进程绑到指定的CPU上？
- awk的使用
- golang中channel是阻塞和非阻塞的？如何设置
- 堆排序的实现

绑定进程到CPU上：

- taskset命令，（进程的所有线程都会绑定到指定CPU）
- sched_setaffinity系统调用，将进程绑定到特定的CPU，（只能绑定主线程）
- pthread_setaffinity_np函数，将线程绑定到CPU上

golang中channel是阻塞的，使用select变成非阻塞。

k8s的资源保障的三个级别？（Guaranteed、Burstable、BestEffort）
