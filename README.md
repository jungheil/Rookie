## 大创项目——Rookie行人跟随机器人

[TOC]

### 环境

* ubuntu18
* opencv4
* realsense
* libtorch
---
* web交互程序依賴：
   + python3
   + flask
   + gevent
   + opencv-python

### 开始

1. 安装Opencv4

   略。。。

2. 安装librorch

   建议使用源码安装：

   ```bash
   git clone --recursive https://github.com/pytorch/pytorch
   cd pytorch
   git checkout 1.6
   # if you are updating an existing checkout
   git submodule sync
   git submodule update --init --recursive
   
   python tools/build_libtorch.py
   ```

3. 安装realsense

   安装步骤参照[这里](https://github.com/IntelRealSense/librealsense/blob/master/doc/distribution_linux.md)。

4. 修改CmakeLists文件

   将下面一行修改成自己libtorch的目录：

   ```bash
   set(CMAKE_PREFIX_PATH ~/install/pytorch/torch/)
   ```

5. 编译

   ```bash
   mkdir build
   cd build
   cmake -DCMAKE_BUILD_TYPE=Release ..
   make -j6
   ```
---
* web交互界面依賴安裝
   ```bash
   pip3 install opencv-python
   pip3 install flask
   pip3 install gevent
   ```
   

### 参考

github: [YOLOv5-LibTorch](https://github.com/Nebula4869/YOLOv5-LibTorch)



