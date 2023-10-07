# Mudong-EV : A multi-threaded network library in C++20

## 项目简介

mudong-ev是一款基于C++20开发的适用于Linux的事件驱动型多线程网络库，附带有日志、定时器、线程池模块，只依赖STL，无第三方依赖。日志模块使用了C++20标准库中的**format**来进行格式化输出，也是整个项目唯一依赖C++20的模块，其余部分均依赖于C++11实现。参考陈硕的muduo网络库，基于one loop per thread搭配线程池的方式实现multi-reactor架构。

## 开发环境

| Tool | Version |
| :---- | :------: |
| Ubuntu(WSL2) | 20.04 |
| GCC |  13.1.0 |
| Cmake | 3.23.0 |
| Git | 2.25.1 |

## 项目架构

![](https://cdn.statically.io/gh/moonlightleaf/pics@master/项目记录/ev架构.5xw73lsoobs0.webp)

mudong-ev的核心架构如上图所示，服务端为一个**TcpServer**实例，该实例中可包含有多个**TcpServerSingle**实例，每个**TcpServerSingle**实例都对应有一个**EventLoop**，运行在一个独立线程中。每个**TcpServerSingle**都包含有独立的**Acceptor**负责监听服务器端口并在连接到来时通过调用**newConnectionCallback**来对连接事件进行处理。该回调函数由**TcpServerSingle**传入**Acceptor**，负责建立连接对象**TcpConnection**。当客户端连接请求到来时，若有多个**Acceptor**监听同一个服务器端口（已开启**SO_REUSEPORT**选项），则由操作系统采用相对公平的方式决定将连接分配给哪个线程，从而实现负载均衡。

## 使用示例

项目代码仓库中的**examples**文件夹下，有**EchoServer**和**AddOneServer**两个示例，并给出了相应的客户端代码。先运行服务端程序，可以在标准输出流中观察到Log的输出（也可以自定义使得Log输出到指定文件）。之后运行客户端代码，并输入合法的字符串即可收到服务端的返回值。**AddOneServer**操作示例，以两个客户端为例：

addOne_client 0 :

```shell
# input
123
# server return
Connection handled by tid 27793, 
calculation handled by tid 27758: 124
```

addOne_client 1 :
```shell
# input
234
# server return
Connection handled by tid 27800, 
calculation handled by tid 27759: 235
```

addOne_server :
```shell
# log
20231007 13:41:11.610 [27757] [  INFO] create TcpServer 0.0.0.0:9877 - TcpServer.cc:16
20231007 13:41:11.611 [27757] [  INFO] TcpServer::start() 0.0.0.0:9877 with 32 eventLoop thread(s) - TcpServer.cc:62
20231007 13:41:16.527 [27757] [  INFO] connection 127.0.0.1:60642 -> 0.0.0.0:9877 is up - AddOneServer.cc:42
20231007 13:41:19.816 [27757] [ TRACE] connection 127.0.0.1:60642 -> 0.0.0.0:9877 recv 3 byte(s) - AddOneServer.cc:51
20231007 13:41:23.760 [27757] [  INFO] connection 127.0.0.1:60644 -> 0.0.0.0:9877 is up - AddOneServer.cc:42
20231007 13:41:26.212 [27757] [ TRACE] connection 127.0.0.1:60644 -> 0.0.0.0:9877 recv 3 byte(s) - AddOneServer.cc:51
20231007 13:41:35.721 [27757] [  INFO] connection 127.0.0.1:60644 -> 0.0.0.0:9877 is down - AddOneServer.cc:42
20231007 13:41:38.072 [27757] [  INFO] connection 127.0.0.1:60642 -> 0.0.0.0:9877 is down - AddOneServer.cc:42
20231007 13:42:11.616 [27757] [  INFO] server quit after 5 second(s)... - AddOneServer.cc:123
20231007 13:42:12.616 [27757] [  INFO] server quit after 4 second(s)... - AddOneServer.cc:125
20231007 13:42:13.616 [27757] [  INFO] server quit after 3 second(s)... - AddOneServer.cc:125
20231007 13:42:14.616 [27757] [  INFO] server quit after 2 second(s)... - AddOneServer.cc:125
20231007 13:42:15.616 [27757] [  INFO] server quit after 1 second(s)... - AddOneServer.cc:125
20231007 13:42:16.616 [27757] [  INFO] server quit after 0 second(s)... - AddOneServer.cc:125
```

**AddOneServer**采用multi-reactor接受连接请求，线程池处理计算任务的架构，从客户端得到的返回结果可以看出，两个客户端的连接请求分别由不同的线程接受，两个客户端的计算任务也由线程池中两个不同的线程执行。

## 测试&&性能

使用**JMeter**对示例中的**AddOneServer**进行压测，TPS可上万。

## 编译&&使用

```shell
$ git clone https://github.com/moonlightleaf/mudong-ev.git
$ cd mudong-ev
$ mkdir build && cd build
$ cmake [-DCMAKE_BUILD_TESTS=1] [-DCMAKE_BUILD_EXAMPLES=1] ..
$ make install
```

可以通过选择是否添加`-DCMAKE_BUILD_TESTS=1`和`-DCMAKE_BUILD_EXAMPLES=1`选项，来决定是否要对`test`和`examples`目录下的文件进行编译。

## 参考

- [muduo](https://github.com/chenshuo/muduo): Event-driven network library for multi-threaded Linux server in C++11