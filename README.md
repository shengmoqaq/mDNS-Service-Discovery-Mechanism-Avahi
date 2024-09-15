# 利用Avahi库实现基于mDNS协议的服务发现机制

## 一、mDNS简介
mDNS（Multicast DNS，多播 DNS）是一个协议，允许在无中央 DNS 服务器的局域网中通过多播 DNS 查询来实现设备的自动服务发现和主机名解析。它是Zeroconf（零配置网络）的一部分，主要用于在局域网中实现无需手动配置的自动网络发现功能
## 二、项目简介
利用两台虚拟机模拟两台局域网设备，第一台模拟器主机名study03运行服务端，第二台模拟器主机名study02运行客户端。客户端输出服务端主机名、IPv4、IPv6地址、服务器名称（MQTTServer）、类型（_mqtt._tcp）、地址（mqtts://tb.com）
## 三、配置环境
如果两台虚拟机上运行需要配置环境，如果在一台虚拟机上两个终端运行则不需要此步骤。
* 确保虚拟机在同一局域网
1 命令查看IP地址，根据IP地址可以确定两台虚拟机是否在同一个子网

``` shell
Ifconfig
```

2 两台虚拟机分别让其IP地址与子网掩码相遇如果结果相等则在同一个子网内

2. 确认虚拟机的网络模式
* 两台虚拟机均为NAT模式可以通信

![image](https://github.com/user-attachments/assets/b5e61496-b8b6-4910-ab16-2a930e943948)


3. 确保防火墙没有阻止 UDP 5353端口
* 检查防火墙状态

```shell
sudo ufw status
```

* 设置允许5353端口

```shell
sudo ufw allow 5353/udp
```

* 或者直接关闭防火墙

```shell
sudo ufw disable
```

## 四、安装依赖

```shell
sudo apt-get update
sudo apt-get install avahi-daemon libavahi-client-dev libavahi-common-dev
```

* avahi-daemon包用于开启守护进程，libavahi-client-dev包用于提供头文件、解析mDNS发现的服务等，libavahi-common-dev包用于内存管理、线程控制等。

## 五、编译
* 进入程序文件路径，编译程序

```shell
gcc -o server.out server.c -lavahi-client -lavahi-common
gcc -o client.out client.c -lavahi-client -lavahi-common
```

## 六、开启守护进程
* avahi-daemon包是Avahi 服务的核心守护进程，它在网络上提供多种服务，包括服务发现、名称解析和服务注册。Avahi 是一个开源的实现了Zeroconf（零配置网络）标准的服务，主要用于在本地网络中自动发现服务和设备。

* 命令开启守护进程并检查状态（是否开启）

```shell
sudo systemctl start avahi-daemon
sudo systemctl status avahi-daemon
```
* 其它常用的avahi-daemon命令
 * 停止/重启/开机自启/关闭开机自启 服务
sudo systemctl stop/restart/enable/disable avahi-daemon

## 七、运行
* 分别运行服务端和客户端

```shell
./server.out
./client.out
```

* 服务端

![image](https://github.com/user-attachments/assets/65f3b395-9188-4cc8-a786-985b9fc3bb67)

* 客户端

![image](https://github.com/user-attachments/assets/562d23de-ac12-4d76-88ac-6b19a95f249e)


## 七、报错
* 如果遇到报错，请检查头文件是否完整等等

* 检查服务命令

```shell
avahi-browse -r _http._tcp
avahi-browse -a
```

* 常见问题
1. 报错(error)：Local name collision
客户端多次打印
