
```shell
#!/usr/bin/env bash
# setup_images.sh

KIND=$1

function pull_image() {
  image=$1
  timage=quay.io/$image
  docker pull $image
  docker tag $image quay.io/$image
  kind load docker-image $timage --name $KIND
}

IMAGES=(
  'cilium/cilium:v1.13.4' \
  'cilium/operator-generic:v1.13.4' \
  'metallb/speaker:v0.13.9' \
  'metallb/controller:v0.13.9' \
  'cilium/certgen:v0.1.8' \
  'cilium/clustermesh-apiserver:v1.13.4' \
  'coreos/etcd:v3.5.7' \
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

  #cilium install --context $CLUSTER --set image.useDigest=false --set operator.image.useDigest=false --set cluster.id=$1 --set cluster.name=$CLUSTER
  helm install cilium cilium-1.13.4.tgz --kube-context $CLUSTER --set cluster.id=$1 --set cluster.name=$CLUSTER --set image.useDigest=false --set operator.image.useDigest=false --set certgen.image.tag=v0.1.8 --set clustermesh.useAPIServer=true --set clustermesh.apiserver.image.useDigest=false --set clustermesh.apiserver.etcd.image.tag=v3.5.7 --set clustermesh.apiserver.service.type=LoadBalancer

  helm install metallb metallb-0.13.9.tgz
  sleep 3
  kubectl apply -f ippool-$KNAME.yaml
}

NUMS=(1)

for number in "${NUMS[@]}"
do
  create_cluster $number
done
```
