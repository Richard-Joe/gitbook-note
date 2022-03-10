# k8s 将docker切换为containerd

master上操作
```bash
// 查看CONTAINER-RUNTIME
[root@master ~]# kubectl get node -owide

// 切换的node改为不可调度
[root@master ~]# kubectl cordon node1

// 驱逐该node上的pod资源
[root@master ~]# kubectl drain node1 --delete-local-data --force --ignore-daemonsets

// 查看生效
[root@master ~]# kubectl get node
node1    Ready,SchedulingDisabled   <none>                 14m   v1.20.6
```

node上操作
```bash
// 卸载docker
[root@node1 ~]# systemctl stop kubelet
[root@node1 ~]# systemctl stop docker
[root@node1 ~]# yum remove -y docker-ce docker-ce-cli

// 配置内核模块和参数
cat <<EOF | sudo tee /etc/modules-load.d/containerd.conf
overlay
br_netfilter
EOF

modprobe overlay
modprobe br_netfilter

cat <<EOF | sudo tee /etc/sysctl.d/99-kubernetes-cri.conf
net.bridge.bridge-nf-call-iptables  = 1
net.ipv4.ip_forward                 = 1
net.bridge.bridge-nf-call-ip6tables = 1
EOF

sysctl --system

// 生成默认配置
containerd config default | sudo tee /etc/containerd/config.toml

// 修改配置
// sandbox_image: 将镜像地址替换为国内阿里云的
// SystemdCgroup: 指定使用systemd作为Cgroup的驱动程序(在options下一行添加的内容)
// endpoint:      修改镜像加速地址
[root@node1 ~]# cat -n  /etc/containerd/config.toml |egrep "sandbox_image|SystemdCgroup |endpoint "
    57      sandbox_image = "registry.aliyuncs.com/google_containers/pause:3.2"   
    97            SystemdCgroup = true
   106            endpoint = ["https://x9jcawxv.mirror.aliyuncs.com"]
   
// 重启containerd
[root@node1 ~]# systemctl restart containerd.service
[root@node1 ~]# systemctl enable containerd.service

// 配置kubelet使用containerd
[root@node1 ~]# cat  /etc/sysconfig/kubelet
KUBELET_EXTRA_ARGS="--container-runtime=remote --container-runtime-endpoint=unix:///run/containerd/containerd.sock --cgroup-driver=systemd"

// 重启kubelet
[root@node1 ~]# systemctl restart kubelet
```

master上操作
```bash
// 查看CONTAINER-RUNTIME是否切换为containerd
[root@master ~]# kubectl get node -owide

// 取消node不可调度的标记
[root@master ~]# kubectl uncordon node1
```

crictl配置
```bash
[root@master /]# cat /etc/crictl.yaml
runtime-endpoint: unix:///run/containerd/containerd.sock
image-endpoint: unix:///run/containerd/containerd.sock
timeout: 10
debug: false
```