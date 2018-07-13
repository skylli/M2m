# 描述
test 文件涵盖了 libm2m 库的所有测试，必须所有测试均通过才能发布.
# 测试文件

```shell
├── readme.md
├── tst_client_logic.c
├── tst_client_mutiple_network.c
├── tst_client_mutiple_session.c
├── tst_device.c
├── tst_server.c
├── tst_obs_client.c
├── tst_obs_device.c



    - tst_device.c 测试时作为虚拟设备，不输出测试结果。
    - tst_server.c 测试时作为虚拟服务器，不输出测试结果。
    - tst_client_logic.c 库的功能测试，涵盖所有的功能。
    - tst_client_mutiple_network.c 同时构建多个 network 向 tst_device 发送数据.
    - tst_client_mutiple_session.c 构建一个 network，多个 session 向 tst_device 发送数据.
    - tst_obs_client.c  作为observer 监听的发起端，发送完 observer start 请求后，等待 device 的 notify。 
    - tst_obs_device.c  接受 observer start 请求，并推送 notify， 支持通过键盘输入 notify 并推送到远端。

```

## 功能测试 logic
共开启三大进程， tst_device，tst_server，tst_client_logic。 tst_client_logic 先往 tst_server 端发送 online check，接着往 tst_device 发送其它功能进行测试
```shell
./tst_server 
./tst_server &
./tst_logic_client

```

## observer  测试
共开启两个进程，tst_obs_device、tst_obs_client； tst_obs_client 先发送 observer start 到 client， client 接受到 observer 之后会模拟设备定时推送消息。
配置键盘输入以及配置定时推送如下：

```c
// 开启了 键盘输入之后就不可以定时推送
#define USE_KEYBOARD_INPUT	        // 开启键盘可输入 string 并推送
#define NOTIFY_INTERVAL_TM 	(1000)  // 定时发送 notify 的时间间隔
```

