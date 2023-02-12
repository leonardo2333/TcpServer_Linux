# TcpServer_Linux

本项目为TcpServer项目的Linux版本

## 编译

1. 新建build文件夹

2. 切换到build文件夹下

```bash
cd build
```

3. 生成Makefile

```bash
cmake ..
```

4. 编译c++代码

```bash
cmake --build .
```

编译结果在`./build/{build_type}`文件夹下。例如，当构建类型为Debug时，编译结果如下所示。其中，bin文件夹下的为可执行文件，lib文件夹下为库文件

```
build
|-...
|-Debug
  |-bin
    |-server
    |-client
  |-lib
    |-libtcp_server.so
    |-libtcp_client.so
|-...
```

