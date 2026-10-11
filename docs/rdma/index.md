# RDMA

学习 RDMA 建议按“理论 → 用户态 → 内核态 → 高级主题”四步推进，总周期 6–8 个月。下面把完整路线、资源与动手步骤浓缩成一张“速查表”，你可以直接照表执行。

------------------------------------------------
一、0 阶段：基础补缺（1-2 周）
1. 网络编程：把《UNP》卷 1 的 TCP/UDP 例程跑一遍。
2. Linux 系统编程：复习 `epoll` + `mmap` + `pthread`。
3. C/C++：能裸写 `makefile` 并调试 `gdb` 即可。
------------------------------------------------
二、1 阶段：RDMA 理论（2-3 周）
任务单  
① 通读 RDMA Aware Programming User Manual 第 1-3 章（官方免费下载）。  
② 画两张图：  
  a. TCP/IP vs RDMA 数据路径对比图；  
  b. IB/RoCE/iWARP 三兄弟协议栈层次图。  
③ 记住 7 个核心对象：Context-PD-MR-CQ-QP-AH-WR 的依赖关系 。  
④ 把 QP 状态机（RESET→INIT→RTR→RTS）手写 3 遍，后续写代码全靠它 。  
------------------------------------------------
三、2 阶段：用户态动手（3-4 周）
2.1 环境  
 • 真机：两台带 Mellanox CX-4/5/6，连同一 IB 交换机；  
 • 没钱：单机 `sudo modprobe rdma_rxe` 跑 SoftRoCE 也能学 。  
 • 必装包：`rdma-core perftest librdmacm-dev`；验证 `ibv_devices` 能看到设备。  

2.2 第一个程序  
 ① 把 `rdma-core/examples/rc_pingpong.c` 编译跑通；  
 ② 把代码精简成“Hello RDMA”——client 用 RC SEND 把字符串发给 server，server 回显。  
 ③ 用 `ib_write_bw` 测一遍带宽、`ib_write_lat` 测延迟，建立性能基线 。  

2.3 三种传输模式  
 • RC（ Reliable Connected ）：双向可靠，必须掌握；  
 • UD（Unreliable Datagram）：无连接，理解 GRH 和 Address Handle；  
 • UC（Unreliable Connected）：了解即可。  
 作业：把上面 Hello 程序改写成 UD 版，观察丢包差异 。  

2.4 RDMA CM  
 用 `rdma_cm` 库把 RC 版的地址解析、连接建立全部替换掉，学会事件通道 `rdma_get_cm_event` 。  

2.5 小项目：分布式 KV  
 功能：PUT/GET/DELETE；  
 技术：server 预注册 1 GB MR，client 用 RDMA Write/Read 直接存取；  
 输出：github 可克隆的完整 repo + README 性能数据。  
------------------------------------------------
四、3 阶段：性能与并发（4-6 周）
1. 多 QP：每核 1 个 QP，把吞吐打满。  
2. 内存管理：做 MR 池 + 循环缓冲区，避免反复 `ibv_reg_mr`。  
3. 无锁队列：用 `rte_ring` 或 `boost::lockfree` 把 CQ 事件分发到多线程。  
4. 调优工具：  
 • `perf` 看 CPU 热点；  
  • `ibv_read_counters` 看 HW 重传；  
  • `flamegraph` 可视化延迟分布 。  
------------------------------------------------
五、4 阶段：内核与驱动（6-8 周）
5.1 读源码路线  
 drivers/infiniband/core/ → verbs.c → qp.c → cq.c  
 drivers/infiniband/hw/mlx5/ → 了解如何把 Verbs 映射到固件命令 。  

5.2 写最简 ULP  
 实现一个“字符设备”：`ioctl` 创建 QP，用户态 `mmap` 后可直接 RDMA Write；  
 目标：把内核模块 `insmod` 后，用户态 50 行代码就能发 RDMA。  

5.3 调试技巧  
 • `dynamic_debug` 打开 IB 子系统日志；  
 • `ftrace` 跟踪 `ib_post_send` 进入驱动的路径；  
 • `/sys/class/infiniband/mlx5_0/ports/1/counters` 看硬件重传 。  
------------------------------------------------
六、5 阶段：高级主题（持续）
• GPUDirect RDMA：CUDA 10+ 之后 20 行代码就能把 GPU 显存注册成 MR，实现 GPU-GPU 直接 RDMA。  
• NVMe-oF：把本地 NVMe 盘通过 NVMe-oF target 暴露给远端，后端用 RDMA；对比内核态 vs SPDK 用户态性能。  
• 容器化：Kubernetes device-plugin 把 RDMA 设备直通到 Pod，配合 Multus 做 SR-IOV CNI。  
------------------------------------------------
七、配套资源
书（必读）  
1. 《RDMA Aware Programming User Manual》  
2. 《InfiniBand Architecture Specification》第 9、10 章  

博客/社区  
• RDMAmojo.com（中文翻译多）  
• Linux RDMA 邮件列表（提问前先看 archive）  

性能工具速查  
`ib_write_bw` `ib_read_lat` `perfquery` `rdma statistic show` 。  
------------------------------------------------
八、时间预算（可业余）
每天 2 h：  
月 1-2 基础 + 理论  
月 3-5 用户态 + 小项目  
月 6-8 内核 + ULP  
月 9+  高级主题 + 生产优化  
------------------------------------------------
九、Checklist（每阶段打卡）
☐ 能徒手画出 QP 状态机  
☐ 能独立编译运行 RC/UD 例程  
☐ 能把 TCP 版的 Echo 改写成 RDMA 版且延迟 < 5 µs  
☐ 能定位“RC QP 建链失败”三类常见错误：GID 不匹配、PFC 未开、PSN 超时  
☐ github 有 3 个 RDMA 项目且 CI 能跑通 `ib_write_bw` 自测  
全部 √ 即出关。

照这个路线“理论—代码—调优—内核”一路打怪升级，6 个月后即可胜任 RDMA 应用开发或系统优化岗位。祝学习顺利！