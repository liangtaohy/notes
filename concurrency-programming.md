[TOC]

# 可扩展Web体系的并发编程

参考自[Concurrent Programming for Scalable Web Architectures](http://berb.github.io/diploma-thesis/original/index.html)。

## 1 介绍

## 2 WWW、并发和可伸缩性

### 2.1 The World Wide Web

WWW又称万维网，是构建在Internet（即互联网）之上的超媒体资源分布式系统。

WWW相关的核心协议有URI、HTTP、HTML以及TCP/IP网络模型。

#### 2.1.1 URI

URI是资源描述符，形式上是一个字符串，用于指向被引用的资源。

其形式如下：

```
        [http(s)|ftp|wss]://example.com:8080/over/there?search=test#first
         \_/   \______________/\_________/ \_________/ \__/
          |           |            |            |        |
       scheme     authority       path        query   fragment
```

#### 2.1.2 HTTP(s)

HTTP中文全称为超文本传输协议，传输层为TCP/IP协议。参见RFC标准[RFC 2616](https://tools.ietf.org/html/rfc2616)。

HTTP是一种无状态的协议，适用于C/S架构，采用request/response通信模型。

Client发起http request，Server返回http response。

* request method定义

```
Method	Usage	Safe	Idempotent	Cachable
GET	This is the most common method of the WWW. It is used for fetching resource representations.	✓	✓	(✓)
HEAD	Essentially, this method is the same as GET, however the entity is omitted in the response.	✓	✓	(✓)
PUT	This method is used for creating or updating existing resources with new representations.	 	✓	 
DELETE	Existing resources can be removed with DELETE	 	✓	 
POST	POST is used to create new resources. Due to its lack of idempotence and safety, it also often used to trigger arbitrary actions.	 	 	 
OPTIONS	Method provides meta data about a resource and available representations.	✓	✓	 
```

* response status

```
Range	Status Type	Usage	Example Code
1xx	informational	Preliminary response codes	100 Continue
2xx	success	The request has been successfully processed.	200 OK
3xx	redirection	The client must dispatch additional requests to complete the request.	303 See Other
4xx	client error	Result of an erroneous request caused by the client.	404 Not Found
5xx	server error	A server-side error occured (not caused by this request being invalid).	503 Service Unavailable
```

* http latency

	* 建立tcp链接需要三次握手，其latency为1 ~ 1.5 RTT;
	* 多个资源的请求，需要在多个http request/response cycle中完成;

* 提高http性能

	* client同时发起多个http request，如浏览器一般允许最多5个http并发
	* 合理使用http cache
	* 长链接：keep-alive
	* pipeline (HTTP 1.1)
	* 合理使用http compression(gzip等)
	* chunck transfer encoding
	* 离用户近（比如静态资源使用CDN）

#### 2.1.3 Web Format

唯[HTML](https://tools.ietf.org/html/rfc7992)莫属。

### 2.2 Web Application

* Web Sites

	* Social Network Applications, Collaborative Web Applications, E-Commerce Sites

* Web Services

	* XML-RPC, SOAP, WSDL and WS-\*, Representational State Transfer

## 3 高可用WebServer架构

### 3.1 传统互联网架构

#### 3.1.1 Server端动态网页技术

* CGI (Common Gateway Interface)
	* per-request process creation
	* high over-head, high latency

* FastCGI
* Web Server Extension Modules

#### 3.1.2 分层构架

* Model-View-Control
* Presentation-Application-Persistence

#### 3.1.3 Load Balancing

* 分类

	* 网络层
		* Network Layer 3/4
	* 应用层
		* 反向代理, LVS

* 策略
	* Round Robin
	* Least connection
	* Least Response Time
	* Randomized
	* Resource-aware

#### 3.1.4 Session Stickiness带来的挑战

### 3.2 云架构

公共网关接口（CGI）[rob04]是一个标准化接口，用于将Web请求委托给处理请求并生成响应的外部应用程序。

## 4 高并发的Web服务器体系结构

### 4.1 介绍

Web Application是I/O密集型应用，其性能测量指标有

* Request throughput (/sec)
* Raw data throughput
* Response times (ms)
* Number of concurrent connections

在服务器上可直接观察的统计信息有

* CPU使用情况
* 内存使用情况
* 文件描述符使用情况
* 进程数/线程数

## 5 应用程序和业务逻辑的并发概念

## 并发

## 高并发和可伸缩的后端存储

## 推荐

## gdb使用

* attach PID
* next
* run
* break
* bt
* step
* display
* print
* info threads
* thread TID
* set scheduler-locking on|off

# 附件 并发模型

## 1 多线程

## 2 事件驱动

## 3 Actor

# 引用

1. [Actor Model of Computation: Scalable Robust Information Systems](http://arxiv.org/abs/1008.1459)
2. [International Society for Inconsistency Robustness](http://www.isir.ws)
3. [Inconsistency Robustness 2011](http://www.robust11.org)
4. [Introduction to Concurrency](https://cs.lmu.edu/~ray/notes/introconcurrency/)
5. [并发编程模型](https://www.cnblogs.com/fxjwind/p/3170032.html)
6. [LMAX Disruptor 原理](https://www.cnblogs.com/fxjwind/p/3180073.html)
http://repository.readscheme.org/ftp/papers/ai-lab-pubs/AIM-379.pdf

