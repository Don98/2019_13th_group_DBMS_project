# 2019_13th_group_DBMS_project
It is the repository for 13th group to store their data

## 我在这里增加一些GitHub的使用提示，虽然不是很详细，但是胜在比较简便

## **1、安装**
GitHub方便大家协作完成项目，首先需要在本地安装git才可以使用。可以从[这里](https://git-scm.com/downloads)进行下载,选择自己合适的版本进行下载。他有命令行版本和GUI版本以及GitHub Desktop版本，但是推荐大家使用命令行版本，同时这里也只是对命令行版本进行介绍（GUI版本一打开基本就知道怎么使用了。）

在Linux下可以直接使用命令sudo apt install git进行下载

## **2、配置账户**

第一步是本地生成ssh key，使用命令

	ssh-keygen -t rsa -C "你的邮件地址"

在浏览器中登录GitHub的setting中添加ssh key，把key.pub的内容复制到key里面。

然后配置Git配置文件

	git config --global user.name "你的用户名"
	git config --global user.email "你的邮箱

接下来是配置在本地避免输入账号密码： 
	cd ~ 
	touch .git-credentials 
	vim .git-credentials 
	https://{username}:{password}@github.com

在终端下输入： 
	git config –global credential.helper store

然后在配置成功后就可以直接使用了。

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

## **4、从远程仓库更新的步骤**

	git status（查看本地分支文件信息，确保更新时不产生冲突）
	git checkout – [file name] （若文件有修改，可以还原到最初状态; 若文件需要更新到服务器上，应该先merge到服务器，再更新到本地）
	git branch（查看当前分支情况）
	git checkout remote branch (若分支为本地分支，则需切换到服务器的远程分支)
	git pull（pull下来）

若命令执行成功，则更新代码成功！


## **5、更新冲突问题**

有的时候你在修改完文件准备提交的时候发现GitHub的仓库已经被人更新了，那么这个时候你要push就会和别人的产生冲突，这个时候怎么处理呢？

**首先提供比较正式的方法：**

第一步：使用git stash，把本地的修改暂时存储起来

	git stash

第二步：把远程的内容pull下来

	git pull

第三步：还原暂存的内容

	git stash pop stash@{0}

第四步：这个时候系统会有提示系统进行了自动合并，但是有些冲突不知道如何解决，需要手动解决这些问题。（它会把发生冲突的文件列出来）

然后这个时候你修改了那些冲突的文件之后再进行push就能够成功了。


**第二种比较粗暴的方法**

在你进行push的时候发生了冲突，那么这个时候你就直接pull下来，系统也会自动进行合并，然后直接到了上面的第四步。