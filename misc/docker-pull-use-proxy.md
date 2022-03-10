# docker/containerd设置PROXY拉取外网镜像

修改配置
```bash
# mkdir -p /etc/systemd/system/docker.service.d
# mkdir -p /etc/systemd/system/containerd.service.d

# vi /etc/systemd/system/docker.service.d/http-proxy.conf
# vi /etc/systemd/system/containerd.service.d/http-proxy.conf
[Service]
Environment="HTTP_PROXY=127.0.0.1:10809"

# vi /etc/systemd/system/docker.service.d/https-proxy.conf
# vi /etc/systemd/system/containerd.service.d/https-proxy.conf
[Service]
Environment="HTTPS_PROXY=127.0.0.1:10809"
```

重启docker
```bash
# systemctl daemon-reload
# systemctl restart docker
# systemctl restart containerd

# docker info
 HTTP Proxy: 127.0.0.1:10809
 HTTPS Proxy: 127.0.0.1:10809
```

拉取镜像
```bash
# docker pull k8s.gcr.io/ingress-nginx/controller:v1.1.1
# crictl pull k8s.gcr.io/ingress-nginx/controller:v1.1.1
v1.1.1: Pulling from ingress-nginx/controller
a0d0a0d46f8b: Pull complete 
3aae86482564: Pull complete 
c0d03781abb3: Pull complete 
0297e2ef8f7f: Pull complete 
866a68ce3c13: Pull complete 
95659ee0b3b1: Pull complete 
004aaf0049fd: Pull complete 
79acb83cc767: Pull complete 
152f0f0a27e1: Pull complete 
256313a1425c: Pull complete 
81e081df9e03: Pull complete 
1b6179f6007a: Pull complete 
3e4300421da9: Pull complete 
Digest: sha256:0bc88eb15f9e7f84e8e56c14fa5735aaa488b840983f87bd79b1054190e660de
Status: Downloaded newer image for k8s.gcr.io/ingress-nginx/controller:v1.1.1
k8s.gcr.io/ingress-nginx/controller:v1.1.1
```
