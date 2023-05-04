# Raft

Raft 在 Multi-Paxos 的基础之上做了两个限制：

- Raft 中**追加日志的操作必须是连续的**，而 Multi-Paxos 中追加日志的操作是并发的；
- Raft 对 Leader 选举的条件做了限制，**只有拥有最新、最全日志的节点才能够当选 Leader**；Multi-Paxos 只是在选择 Leader 之后需要将 Leader 中的日志补全。

在任意时刻，每一台服务器只可能处于 Leader、Follower 以及 Candidate 三种状态。

在处于正常的状态时，集群中只会存在一个 Leader，其余的服务器都是 Follower。

所有的 Follower 节点都是被动的，它们不会主动发出任何的请求，只会响应 Leader 和 Candidate 发出的请求，对于每一个用户的可变操作，都会被路由给 Leader 节点进行处理，除了 Leader 和 Follower 节点之外，Candidate 节点其实只是集群运行过程中的一个临时状态。
