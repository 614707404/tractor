# 参考muduo实现的高性能网络库
本项目是在拜读了陈硕老师的作品《Linux多线程服务端编程》后，出于学习目的仿写的高性能网络库。
网络库基本架构参照muduo库，具体实现个人完成，尽可能使用我熟悉的C++11特性，如std::thread,智能指针等；
项目的cmake结构参考了一个cmake教程中的结构，见https://cliutils.gitlab.io/modern-cmake/
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