# 有用的操作

### Git获取最新的标签
```bash
latest_tag=$(git describe --tags $(git rev-list --tags --max-count=1))
```

### Centos7升级内核
（参考：http://elrepo.org/tiki/tiki-index.php）
```bash
// 启用 ELRepo 仓库
rpm --import https://www.elrepo.org/RPM-GPG-KEY-elrepo.org
rpm -Uvh http://www.elrepo.org/elrepo-release-7.0-2.el7.elrepo.noarch.rpm

// 列出可用的内核包
yum --disablerepo="*" --enablerepo="elrepo-kernel" list available

// 安装最新的内核版本
yum --enablerepo=elrepo-kernel install kernel-ml
```

### k8s Token过期
```bash
// 查看当前有效的令牌
kubeadm token list

// 生成有效的令牌（永不失效）
kubeadm token create --print-join-command --ttl=0
```
