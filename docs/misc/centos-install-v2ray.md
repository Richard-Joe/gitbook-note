# Centos 安装v2ray

## 安装v2ray

```bash
# bash <(curl -L https://raw.githubusercontent.com/v2fly/fhs-install-v2ray/master/install-release.sh)
```

```bash
# systemctl enable v2ray
# systemctl start v2ray

# ps aux | grep v2ray
nobody    69343  0.3  1.4 5054208 27336 ?       Ssl  23:10   0:06 /usr/local/bin/v2ray -config /usr/local/etc/v2ray/config.json
```

## 修改配置

将配置写到/usr/local/etc/v2ray/config.json，重启v2ray即可


## 访问Google

```bash
# export https_proxy=http://127.0.0.1:10809

# curl https://www.google.com -I
HTTP/1.1 200 Connection established

HTTP/1.1 200 OK
```

