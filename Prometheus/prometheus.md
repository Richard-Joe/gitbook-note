# Prometheus

## 1. 架构

## 2. 组件

- **Prometheus Server**：Prometheus Server负责实现对监控数据的获取，存储（TSDB）以及查询（PromQL）。
- **Exporters**：Exporter将监控数据采集的端点通过HTTP服务的形式暴露给Prometheus Server。
  - 直接采集：直接内置了对Prometheus监控的支持。
  - 间接采集：本身不支持，需根据lib编写Exporter。
- **AlertManager**：告警处理中心。
- **PushGateway**：当Prometheus Server（采用Pull模型）无法直接与Exporter进行通信时，可以通过PushGateway将内部网络的监控数据主动Push到Gateway当中。而Prometheus Server则可以采用同样Pull的方式从PushGateway中获取到监控数据。

## 3. 概念

- Prometheus 所有采集的监控数据均以**时间序列**（具有相同指标名称、相同标签集合的、有时间戳标记的数据流）的形式保存在内置的时间序列数据库（TSDB）中。
- 每一条 **时间序列** 由 **指标名称（Metrics Name）** 以及一组 **标签（键值对）** 唯一标识。
- **样本Samples** 构成了真正的时间序列值。
  - 时间戳（timestamp）：一个精确到毫秒的时间戳；
  - 样本值（value）： 一个 folat64 的浮点型数据表示当前样本的值。

## 4. 指标类型

- **Counter**（计数器）：单调递增的指标。主要有两个方法：Inc()、Add(float64)。
- **Gauge**（仪表盘）：可以任意变化的指标。
- **Histogram**（直方图）：快速了解监控样本的分布情况。
  - 样本的值分布在 bucket 中的数量，`<basename>_bucket{le="<上边界>"}`；
  - 所有样本值的大小总和，`<basename>_sum`；
  - 样本总数，`<basename>_count`；
- **Summary**（摘要）：用于表示一段时间内的数据采样结果，
  - 样本值的分位数分布情况， `<basename>{quantile="<φ>"}`；
  - 所有样本值的大小总和，`<basename>_sum`；
  - 样本总数， `<basename>_count` ；
  
## 5. 存储
  
- **本地存储**：Prometheus 按照两个小时为一个时间窗口，将两小时内产生的数据存储在一个块（Block）中。每个块都是一个单独的目录，里面含该时间窗口内的所有样本数据（chunks），元数据文件（meta.json）以及索引文件（index）。
  - 当前样本数据所在的块会被直接保存在内存中，不会持久化到磁盘中。为了确保 Prometheus 发生崩溃或重启时能够恢复数据，Prometheus 启动时会通过预写日志（write-ahead-log(WAL)）重新记录，从而恢复数据。预写日志文件保存在 wal 目录中
- **远程存储**：通过定义两个标准接口（remote_write/remote_read），让用户可以基于这两个接口对接将数据保存到任意第三方的存储服务中。

## 6. 自定义一个Exporter

https://zhuanlan.zhihu.com/p/273229856

## 参考
  
https://yunlzheng.gitbook.io/prometheus-book/
  
