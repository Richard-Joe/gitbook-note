# Cluster Mesh

## 1. 搭建环境

```yaml
# Cluster1
---
## c1.yaml
kind: Cluster
apiVersion: kind.x-k8s.io/v1alpha4
networking:
  disableDefaultCNI: true
  kubeProxyMode: "iptables"
  podSubnet: "10.111.0.0/16"
  serviceSubnet: "10.11.0.0/16"
nodes:
- role: control-plane
- role: worker

# Cluster2
---
## c2.yaml
kind: Cluster
apiVersion: kind.x-k8s.io/v1alpha4
networking:
  disableDefaultCNI: true
  kubeProxyMode: "iptables"
  podSubnet: "10.222.0.0/16"
  serviceSubnet: "10.22.0.0/16"
nodes:
- role: control-plane
- role: worker

# ippool-c1.yaml
---
apiVersion: metallb.io/v1beta1
kind: IPAddressPool
metadata:
  name: c1-ip-pool
spec:
  addresses:
  - 172.18.0.100-172.18.0.110
---
apiVersion: metallb.io/v1beta1
kind: L2Advertisement
metadata:
  name: c1-layer2
spec:
  ipAddressPools:
  - c1-ip-pool
  interfaces:
  - eth0

# ippool-c2.yaml
---
apiVersion: metallb.io/v1beta1
kind: IPAddressPool
metadata:
  name: c2-ip-pool
spec:
  addresses:
  - 172.18.0.200-172.18.0.210
---
apiVersion: metallb.io/v1beta1
kind: L2Advertisement
metadata:
  name: c2-layer2
spec:
  ipAddressPools:
  - c2-ip-pool
  interfaces:
  - eth0
```

```shell
#!/usr/bin/env bash
# setup_images.sh

KIND=$1

function pull_image() {
  image=$1
  docker pull $image
  kind load docker-image $image --name $KIND
}

IMAGES=(
  'quay.io/cilium/cilium:v1.13.4' \
  'quay.io/cilium/operator-generic:v1.13.4' \
  'quay.io/metallb/speaker:v0.13.10' \
  'quay.io/metallb/controller:v0.13.10' \
  'quay.io/cilium/certgen:v0.1.8' \
  'quay.io/cilium/clustermesh-apiserver:v1.13.4' \
  'quay.io/coreos/etcd:v3.5.4' \
)

for image in "${IMAGES[@]}"
do
  pull_image $image
done
```

```shell
#!/usr/bin/env bash
# cluster.sh

function create_cluster() {
  KNAME=c$1
  CLUSTER=kind-$KNAME

  kind create cluster --name $KNAME --config $KNAME.yaml

  ./setup_images.sh $KNAME

  helm install cilium cilium-1.13.4.tgz -n kube-system --kube-context $CLUSTER --set ipam.mode=kubernetes --set cluster.id=$1 --set cluster.name=$CLUSTER --set image.useDigest=false --set operator.image.useDigest=false --set certgen.image.tag=v0.1.8 --set clustermesh.useAPIServer=true --set clustermesh.apiserver.image.useDigest=false --set clustermesh.apiserver.etcd.image.tag=v3.5.4 --set clustermesh.apiserver.service.type=LoadBalancer
  kubectl --context $CLUSTER rollout status ds/cilium -n kube-system -w

  helm install metallb metallb-0.13.9.tgz -n kube-system
  kubectl --context $CLUSTER rollout status ds/metallb-speaker -n kube-system -w
  kubectl --context $CLUSTER apply -f ippool-$KNAME.yaml
}

NUMS=(1)

for number in "${NUMS[@]}"
do
  create_cluster $number
done

cilium clustermesh connect --context kind-c1 --destination-context kind-c2 -n default
```

```bash
[root@fedora kind]# cilium clustermesh status --context kind-c1
✅ Cluster access information is available:
  - 172.18.0.100:2379
✅ Service "clustermesh-apiserver" of type "LoadBalancer" found
✅ All 2 nodes are connected to all clusters [min:1 / avg:1.0 / max:1]
🔌 Cluster Connections:
- kind-c2: 2/2 configured, 2/2 connected
🔀 Global services: [ min:6 / avg:6.0 / max:6 ]

[root@fedora kind]# cilium clustermesh status --context kind-c2
✅ Cluster access information is available:
  - 172.18.0.200:2379
✅ Service "clustermesh-apiserver" of type "LoadBalancer" found
✅ All 2 nodes are connected to all clusters [min:1 / avg:1.0 / max:1]
🔌 Cluster Connections:
- kind-c1: 2/2 configured, 2/2 connected
🔀 Global services: [ min:6 / avg:6.0 / max:6 ]
```

## 2. 测试连通性

```yaml
# echo.yaml
---
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: echo-1
spec:
  selector:
    matchLabels:
      run: echo-1
  template:
    metadata:
      labels:
        run: echo-1
    spec:
      containers:
        - image: jmalloc/echo-server
          imagePullPolicy: IfNotPresent
          name: echo-1
          ports:
            - containerPort: 8080
      tolerations:
      - operator: Exists
---
apiVersion: v1
kind: Service
metadata:
  name: echo-1
  annotations:
    service.cilium.io/global: "true"
  labels:
    run: echo-1
spec:
  ports:
    - port: 80
      targetPort: 8080
      protocol: TCP
  type: ClusterIP
  selector:
    run: echo-1
```

```bash
# 在 C1 和 C2 均创建资源
[root@fedora kind]# kubectl --context kind-c1 apply -f echo.yaml
[root@fedora kind]# kubectl --context kind-c2 apply -f echo.yaml

# 查看资源状态
[root@fedora kind]# kubectl --context kind-c1 get all -owide -l run=echo-1
NAME               READY   STATUS    RESTARTS      AGE     IP             NODE               NOMINATED NODE   READINESS GATES
pod/echo-1-fkxm6   1/1     Running   1 (36m ago)   2d13h   10.111.1.176   c1-worker          <none>           <none>
pod/echo-1-x974r   1/1     Running   1 (36m ago)   2d13h   10.111.0.236   c1-control-plane   <none>           <none>

NAME             TYPE        CLUSTER-IP      EXTERNAL-IP   PORT(S)   AGE     SELECTOR
service/echo-1   ClusterIP   10.11.194.118   <none>        80/TCP    2d13h   run=echo-1

[root@fedora kind]# kubectl --context kind-c2 get all -owide -l run=echo-1
NAME               READY   STATUS    RESTARTS      AGE     IP             NODE               NOMINATED NODE   READINESS GATES
pod/echo-1-sdslz   1/1     Running   1 (36m ago)   2d13h   10.222.0.213   c2-control-plane   <none>           <none>
pod/echo-1-vzvzv   1/1     Running   1 (36m ago)   2d13h   10.222.1.11    c2-worker          <none>           <none>

NAME             TYPE        CLUSTER-IP     EXTERNAL-IP   PORT(S)   AGE     SELECTOR
service/echo-1   ClusterIP   10.22.148.88   <none>        80/TCP    2d13h   run=echo-1
```

### 2.1. pod-to-pod

集群 C2 访问集群 C1 的Pod：
```bash
[root@fedora kind]# kubectl --context kind-c2 run curl -it --rm --image=curlimages/curl -- sh
If you don't see a command prompt, try pressing enter.

~ $ curl 10.111.1.176:8080
Request served by echo-1-fkxm6

GET / HTTP/1.1

Host: 10.111.1.176:8080
Accept: */*
User-Agent: curl/8.2.1
~ $
```

### 2.2. pod-to-service（global）

- 集群 C2 访问集群 C1 的ClusterIP Service：

```bash
~ $ curl 10.11.194.118
^C
# 访问不通，ClusterIP的IP只能集群内访问
```

- 集群 C2 访问 global Service：

```bash
~ $ curl echo-1
Request served by echo-1-x974r

GET / HTTP/1.1

Host: echo-1
Accept: */*
User-Agent: curl/8.2.1

~ $ curl echo-1
Request served by echo-1-sdslz

GET / HTTP/1.1

Host: echo-1
Accept: */*
User-Agent: curl/8.2.1

# 可以访问到 C1 的Pod，也可以访问到 C2 的Pod
```

## 3. 数据面流程

数据面流程上：`跨集群访问`跟`集群内跨节点`完全一致。

- `pop-to-pod`：cilium_ipcache中记录了其他集群的Pod IP；
- `pop-to-service`：cilium_lb4_services_v2中记录了全局的后端；
