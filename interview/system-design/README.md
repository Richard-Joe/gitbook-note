# 系统设计

## 参考：

https://github.com/donnemartin/system-design-primer/

## CAP 理论

https://robertgreiner.com/cap-theorem-revisited/

- **Consistency**，一致性：读取保证返回的是最新写入；
- **Availability**，可用性：一个非故障节点将在合理的时间内返回合理的响应（无错误或超时）；
- **Partition Tolerance**，分区容错：当网络分区发生时，系统将继续运行；

CAP 定理指出，在分布式系统（共享数据的互连节点集合）中，您只能在写/读对中获得以下三种保证中的两种：一致性、可用性和分区容错性 - 其中之一他们必须被牺牲。

根据 CAP 定理，这意味着我们有两个选择：一致性和可用性。

- **CP** - 一致性/分区容错性 - 等待分区节点的响应，这可能会导致超时错误。
- **AP** - 可用性/分区容错性 - 返回您拥有的最新版本的数据，这可能是陈旧的。

### 一致性模式

有同一份数据的多份副本，我们面临着怎样同步它们的选择，以便让客户端有一致的显示数据。

- **弱一致性**：在写入之后访问，可能看到，也可能看不到（写入数据）。场景：VoIP，视频聊天和实时多人游戏等。
- **强一致性**：在写入后，访问立即可见。**数据被同步复制**。场景：文件系统、关系型数据库。
- **最终一致性**：在写入后，访问最终能看到写入数据（通常在数毫秒内）。**数据被异步复制**。

### 可用性模式

- **故障切换**（fail-over）；
	- 两种方式：主备切换、主主切换；
	- 缺点：1. 需添加额外硬件；2. 可能会丢失数据（新数据还未同步到备用系统）；
- **复制**（replication）；
	- 主从复制
	- 主主复制

