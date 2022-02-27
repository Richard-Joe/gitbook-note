# 编译安装ovs

## 构建
环境：Ubuntu20.04

```bash
# 安装依赖
apt-get update
apt-get -y install build-essential fakeroot
apt-get -y install graphviz autoconf automake bzip2 debhelper dh-autoreconf libssl-dev libtool openssl
apt-get -y install procps python3-all python3-sphinx python3-twisted python3-zope.interface libunbound-dev libunwind-dev

# 下载代码
git clone https://github.com/openvswitch/ovs.git
git checkout branch-2.17

# 进入源码目录
cd ovs

# 检查依赖
# if everything is ok then this should return no output
dpkg-checkbuilddeps

# 构建configure脚本
./boot.sh

# 配置
./configure

# 编译并安装（默认安装在/usr/local下）
make
make install

# 加载内核模块
modprobe openvswitch

# 查看模块
lsmod | grep openvswitch

# 启动
ovs-ctl start
```

## 验证

```bash
ovs-vsctl add-br br0
ovs-vsctl show
```

## 沙盒
```bash
# 这是一个模拟环境
cd ovs
./tutorial/ovs-sandbox
----------------------------------------------------------------------
You are running in a dummy Open vSwitch environment.  You can use
ovs-vsctl, ovs-ofctl, ovs-appctl, and other tools to work with the
dummy switch.

Log files, pidfiles, and the configuration database are in the
"sandbox" subdirectory.

Exit the shell to kill the running daemons.
```

## 搭建 Faucet
```bash
git clone https://github.com/faucetsdn/faucet.git
cd faucet
latest_tag=$(git describe --tags $(git rev-list --tags --max-count=1))
git checkout $latest_tag

# 构建镜像
# --- 会很慢，修改脚本install-faucet.sh 
# --- 修改alpine源：sed -i 's/dl-cdn.alpinelinux.org/mirrors.aliyun.com/g' /etc/apk/repositories
# --- 修改pip3源：pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple
docker build -t faucet/faucet -f Dockerfile.faucet .

# Faucet 配置将写入inst/faucet.yaml，其主日志写入inst/faucet.log
mkdir inst

# 创建容器并启用
docker run -d --name faucet --restart=always -v $(pwd)/inst/:/etc/faucet/ -v $(pwd)/inst/:/var/log/faucet/ -p 6653:6653 -p 9302:9302 faucet/faucet

# 查看日志inst/faucet.log（还没创建inst/faucet.yaml，会启动失败）

# 编写配置inst/faucet.yaml
dps:
    switch-1:
        dp_id: 0x1
        timeout: 3600
        arp_neighbor_timeout: 1200
        interfaces:
            1:
                native_vlan: 100
            2:
                native_vlan: 100
            3:
                native_vlan: 100
            4:
                native_vlan: 200
            5:
                native_vlan: 200
vlans:
    100:
    200:

# 重启faucet容器
docker restart faucet

# 查看inst/faucet.log
Feb 27 09:05:51 faucet INFO     Add new datapath DPID 1 (0x1)
# Faucet 现在正在等待数据路径 ID 为 0x1 的交换机通过 OpenFlow 连接到它

# 使用 OVS 创建一个交换机并使其连接到 Faucet
# 在sandbox中，创建一个名为 br0 的交换机，将其数据路径 ID 设置为 0x1，向其添加名为p1~p5的模拟端口，并告诉它连接到faucet。
ovs-vsctl add-br br0 \
         -- set bridge br0 other-config:datapath-id=0000000000000001 \
         -- add-port br0 p1 -- set interface p1 ofport_request=1 \
         -- add-port br0 p2 -- set interface p2 ofport_request=2 \
         -- add-port br0 p3 -- set interface p3 ofport_request=3 \
         -- add-port br0 p4 -- set interface p4 ofport_request=4 \
         -- add-port br0 p5 -- set interface p5 ofport_request=5 \
         -- set-controller br0 tcp:127.0.0.1:6653 \
         -- set controller br0 connection-mode=out-of-band

# 查看inst/faucet.log（faucet 识别并配置了新交换机及其端口）
Feb 27 09:11:43 faucet.valve INFO     DPID 1 (0x1) switch-1 Configuring VLAN 100 vid:100 untagged: Port 1,Port 2,Port 3
Feb 27 09:11:43 faucet.valve INFO     DPID 1 (0x1) switch-1 Configuring VLAN 200 vid:200 untagged: Port 4,Port 5
Feb 27 09:17:00 faucet.valve INFO     DPID 1 (0x1) switch-1 Port 1 (1) up
Feb 27 09:17:00 faucet.valve INFO     DPID 1 (0x1) switch-1 Port 2 (2) up
Feb 27 09:17:00 faucet.valve INFO     DPID 1 (0x1) switch-1 Port 3 (3) up
Feb 27 09:17:00 faucet.valve INFO     DPID 1 (0x1) switch-1 Port 4 (4) up
Feb 27 09:17:00 faucet.valve INFO     DPID 1 (0x1) switch-1 Port 5 (5) up

# 查看sandbox/ovs-vswitchd.log（OpenFlow 会话设置和 Faucet 对交换机端口和功能的探测）

# 目前拥有以下表
Table 0 (vlan)
	Ingress VLAN processing
Table 1 (eth_src)
	Ingress L2 processing, MAC learning
Table 2 (eth_dst)
	Egress L2 processing
Table 3 (flood)
	Flooding
```

### MAC 学习

```bash
save-flows br0 > flows1
ovs-appctl ofproto/trace br0 in_port=p1,dl_src=00:11:11:00:00:00,dl_dst=00:22:22:00:00:00 -generate
diff-flows flows1 br0
```

## 路由

```bash
# 启用路由，编辑inst/faucet.yaml
vlans:
    100:
        faucet_vips: ["10.100.0.254/24"]
    200:
        faucet_vips: ["10.200.0.254/24"]
routers:
    router-1:
        vlans: [100, 200]

# 重新加载配置
docker exec faucet pkill -HUP -f faucet.faucet

# 增加了FIB和VIP表
Table 0 (vlan)
	Ingress VLAN processing
Table 1 (eth_src)
	Ingress L2 processing, MAC learning
Table 2 (ipv4_fib)
	L3 forwarding for IPv4
Table 3 (vip)
	Virtual IP processing, e.g. for router IP addresses implemented by Faucet
Table 4 (eth_dst)
	Egress L2 processing
Table 5 (flood)
	Flooding
	
# 查看添加了哪些新的流规则
diff-flows flows1 br0 | grep +
```

## ACL

```bash
# 编辑inst/faucet.yaml
acls:
    1:
        - rule:
            dl_type: 0x800
            nw_proto: 6
            tcp_dst: 8080
            actions:
                allow: 0
        - rule:
            actions:
                allow: 1

# 重新加载配置
docker exec faucet pkill -HUP -f faucet.faucet
```