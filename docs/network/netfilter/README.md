# netfilter

netfilter是Linux内核的包过滤框架，它提供了一系列的钩子（Hook）供其他模块控制包的流动。这些钩子包括

 - NF_IP_PRE_ROUTING：刚刚通过数据链路层解包进入网络层的数据包通过此钩子，它在路由之前处理
 - NF_IP_LOCAL_IN：经过路由查找后，送往本机（目的地址在本地）的包会通过此钩子
 - NF_IP_FORWARD：不是本地产生的并且目的地不是本地的包（即转发的包）会通过此钩子
 - NF_IP_LOCAL_OUT：所有本地生成的发往其他机器的包会通过该钩子
 - NF_IP_POST_ROUTING：在包就要离开本机之前会通过该钩子，它在路由之后处理
