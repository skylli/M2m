# 描述
test 文件涵盖了 libm2m 库的所有测试，必须所有测试均通过才能发布.
# 测试文件

```shell
├── readme.md
├── tst_client_logic.c
├── tst_client_mutiple_network.c
├── tst_client_mutiple_session.c
├── tst_device.c
└── tst_server.c

    - tst_device.c 测试时作为虚拟设备，不输出测试结果。
    - tst_server.c 测试时作为虚拟服务器，不输出测试结果。
    - tst_client_logic.c 库的功能测试，涵盖所有的功能。
    - tst_client_mutiple_network.c 同时构建多个 network 向 tst_device 发送数据.
    - tst_client_mutiple_session.c 构建一个 network，多个 session 向 tst_device 发送数据.
    
```