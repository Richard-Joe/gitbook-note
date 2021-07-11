# VLAN

LAN 表示 Local Area Network，本地局域网，通常使用 Hub 和 Switch 来连接 LAN 中的计算机。 一个 LAN 表示一个广播域，它的意思是 LAN 中的所有成员都会收到 LAN 中一个成员发出的广播包。 因此，LAN 的边界在路由器或者类似的三层设备。

VLAN 表示 Virtual LAN。一个带有 VLAN 功能的 Switch 能够同时处于多个 LAN 中。简单的说， VLAN 是一种将一个交换机分成多个交换机的一种方法。

它在普通以太网帧结构 SA （src address）之后 加入了 4bytes 的 VLAN Tag/Header 数据，其中包括 12bits 的 VLAN ID。VLAN ID的最大值是 4096， 但是有效值范围是 1- 4094。

## QinQ

QinQ是为了扩大VLAN ID的数量而提出的技术（IEEE 802.1ad），外层tag称为Service Tag，而内层tag则称为Customer Tag。