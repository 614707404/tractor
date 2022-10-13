# 参考muduo实现的高性能网络库 tractor
* 本项目是在拜读了陈硕老师的作品《Linux多线程服务端编程》后，出于学习目的仿写的高性能网络库；
* 网络库基本架构参照muduo库，具体实现个人完成，尽可能使用我熟悉的C++11特性，如std::thread，å智能指针等；
* 取名为tractor主要是因为muduo就像一辆精美的汽车，而我个人仿写实现的就像一个粗旷的拖拉机了。共同点是都能跑，且原理是一样滴；
* 项目的cmake结构参考了一个cmake教程中的结构，见https://cliutils.gitlab.io/modern-cmake/
## 项目构建
To configure:

```bash
cmake -S . -B build
```


To build:

```bash
cmake --build build
```
## 性能测试
用webbench测试用tractor实现的http服务
并发数100，运行十秒，结果如下：
```
[root@iZbp10aly5tyyoe7qr2ot9Z ~]# webbench -c 100  -t 10 http://127.0.0.1:8000/index
Webbench - Simple Web Benchmark 1.5
Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.

Benchmarking: GET http://127.0.0.1:8000/index
100 clients, running 10 sec.

Speed=179070 pages/min, 1736979 bytes/sec.
Requests: 29845 susceed, 0 failed.

```
并发数10000，运行100秒，结果如下：
```
[root@iZbp10aly5tyyoe7qr2ot9Z ~]# webbench -c 10000  -t 100 http://127.0.0.1:8000/index
Webbench - Simple Web Benchmark 1.5
Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.

Benchmarking: GET http://127.0.0.1:8000/index
10000 clients, running 100 sec.

Speed=4359 pages/min, 40804 bytes/sec.
Requests: 8033 susceed, 283 failed.
```
结果并不理想，我认为原因和我的阿里云服务器配置有关（1核(vCPU) 2 GiB）。所以尽管是在代码中实现了多线程，但是上还是同步执行，而且在机器上同时跑服务、webbench还有ssh，把机器的性能吃满了。CPU的占有率达到了84.5%，VS Code的SSH服务直接崩溃了。