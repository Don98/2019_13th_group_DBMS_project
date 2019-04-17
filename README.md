# 2019_13th_group_DBMS_project
It is the repository for 13th group to store their data

## 我在这里增加一些GitHub的使用提示，虽然不是很详细，但是胜在比较简便

## **1、安装**
GitHub方便大家协作完成项目，首先需要在本地安装git才可以使用。可以从[这里](https://git-scm.com/downloads)进行下载,选择自己合适的版本进行下载。他有命令行版本和GUI版本以及GitHub Desktop版本，但是推荐大家使用命令行版本，同时这里也只是对命令行版本进行介绍（GUI版本一打开基本就知道怎么使用了。）

在Linux下可以直接使用命令sudo apt install git进行下载

## **2、配置账户**

第一步是本地生成ssh key，使用命令

	ssh-keygen -t rsa -C "你的邮件地址"

在浏览器中登录GitHub的setting中添加ssh key，可以通过命令

	ssh -T 你的@github.com

进行查看是否配置成功，如果失败则使用

	ssh -add

然后配置Git配置文件

	git config --global user.name "你的用户名"
	git config --global user.email "你的邮箱"
到这一步基本就配置差不多了。

## **3、使用GitHub**

这里提供一些简单的GitHub使用指令，虽然不是非常的全面但是已经足够完成作业的使用了，如果在后期感觉有所欠缺可以后续自己谷歌。

首先是clone一个仓库

	git clone 地址

然后他就会把整个仓库克隆到你的本地。这个时候你就可以在本地查看和修改。

然后是如何进行提交

	git add 要提交的东西
	git commit -m 备注
	git push

一连串的三条指令就可以了。

add是添加东西到你的本地缓冲区，可以这么理解，然后使用commit添加备注，最后使用push上传到服务器。

注：git add ./ 可以把所有东西都上传，不过使用这条指令的时候要谨慎，确保不要覆盖了其他人的东西。

## **4、从网上更新的步骤**

	git status（查看本地分支文件信息，确保更新时不产生冲突）
	git checkout – [file name] （若文件有修改，可以还原到最初状态; 若文件需要更新到服务器上，应该先merge到服务器，再更新到本地）
	git branch（查看当前分支情况）
	git checkout remote branch (若分支为本地分支，则需切换到服务器的远程分支)
	git pull（pull下来）

若命令执行成功，则更新代码成功！