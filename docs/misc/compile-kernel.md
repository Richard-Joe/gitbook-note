# 编译内核

```bash
# 0. 系统 Fedora 36
# 1. 内存尽量 >= 4G
# 2. 磁盘尽量大，40G肯定是不够的

# 3. 安装依赖
$ dnf groupinstall "C Development Tools and Libraries"
$ dnf install openssl openssl-devel dwarves rpm-build libelf-devel elfutils-libelf-devel ncurses

# 4. 下载内核代码，解压
$ cd linux-6.2

# 5. 配置内核
$ cp /boot/config-*.x86_64 .config
$ make menuconfig
$ vim .config   # 注释掉 CONFIG_SYSTEM_TRUSTED_KEYS

# 6. 编译内核，并安装
$ make -j8
$ make modules_install
$ make install

# 7. 更新grub配置
$ grub2-mkconfig -o /boot/grub2/grub.cfg
$ grubby --set-default /boot/vmlinuz-5.6.9
$ grubby --info=ALL # 查看是否修改成功
$ reboot
```
