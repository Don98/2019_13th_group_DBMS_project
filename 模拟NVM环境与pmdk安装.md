### GRUB配置

首先通过dmesg命令查看可用的内存范围(标注为usable)，建议使用4G以上的内存

随后打开grub文档进行配置

```
# vi /etc/default/grub
```

在GRUB_CMDLINE_LINUX_DEFAULT一项中添加memmap=nG!mG，意为mG之上的nG内存

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190423193525330.png)

随后执行以下命令使操作生效，并重启虚拟机

```
# sudo grub-mkconfig -o /boot/grub/grub.cfg
# reboot
```

然后创建一个文件夹进行挂载操作：

```
sudo mkdir /mnt/mem
sudo sudo mkfs.ext4 /dev/pmem0
sudo mount -o dax /dev/pmem0 /mnt/mem
```



### 安装PMDK

建议使用ubuntu18.04的环境，具体安装过程请参考官方文档

<https://docs.pmem.io/getting-started-guide/installing-pmdk/compiling-pmdk-from-source>
