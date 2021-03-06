# 缓冲膨胀

缓冲膨胀（BufferBloat）是一种因数据包过度缓冲而引起的数据包交换网络高延迟原因。

一些通信设备制造商在他们的某些网络产品中不必要地设计了过大的缓冲区。在这种设备中，当网络链路拥塞时，就会发生缓冲膨胀，从而导致数据包在这些超大缓冲区中长时间排队。在先进先出队列系统中，过大的缓冲区会导致更长的队列和更高的延迟，并且不会提高网络吞吐量。

## 机制

大多数TCP拥塞控制算法都依靠测量丢包的发生来确定连接两端之间的可用带宽。该算法会加快数据传输速度，直到数据包开始丢失，然后降低传输速率。

数据包在传输之前先在网络缓冲区中排队；在有问题的情况下，仅当缓冲区已满时才丢弃数据包。在较旧的路由器上，缓冲区很小，因此缓冲区很快就装满了，因此，在链路饱和后不久，数据包就开始丢失，因此TCP协议可以进行调整，问题不会变得明显。在较新的路由器上，缓冲区已变得足够大，可以容纳几秒钟的缓冲数据。对于TCP，当缓冲区填满时，拥塞的链接似乎可以正常运行。TCP算法不知道链接已阻塞，并且直到缓冲区最终溢出并丢弃数据包后才开始采取纠正措施。

## 解决方案和缓解措施

- 针对网络的解决方案
	- AQM（主动队列管理）算法，例如CoDel和PIE
	- 混合AQM和数据包调度算法，例如FQ-CoDel

- 针对端点的解决方案
	- TCP的BBR拥塞控制算法
	- 许多BitTorrent客户端使用的微传输协议协议
	- 使用较少连接的技术，例如HTTP管道传输或HTTP / 2，而不是纯HTTP协议


> ---- 以上来自wikipedia

## 背景缘由

buffer为何会越来越大？

其实这是一种不得已的行为，最本质的原因在于：

**早期计算机的处理器性能决定了发包的速率，像思科这种厂商，他们的路由器，交换机的处理能力是发包的终端计算机难以企及的，这个阶段中间节点的高端设备只需要不多且固定数量的缓存，就可以暂时存储还没有来得及处理的数据包。
但是后来时代变了！计算机终端的处理器，网卡性能和中间转发设备的距离越来越近，而中间转发设备的性能已经快达到极限，增加处理器和线卡的数量性价比远不如将来不及处理的数据暂时存起来，这带来了一种解决方案：增加缓存大小！这并没有解决问题，而是引入了问题，网络不再是一个用完即走的设施，而成了一个巨大的缓存设施，这就是BufferBloat**

另外，摘自其他文章里的一句话：

**TCP拥塞控制的终极目标绝对不是加快数据发送的速度，而是在公平占有带宽的前提下无限度提高带宽的利用率！**
