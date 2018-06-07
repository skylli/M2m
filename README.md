
# Describe
M2M protocol is use in communication between machines. Both endpoint are equal.Most of  reference of Coap and Mqtt.

# Directory structure
```shell
.
├── app
├── bin
├── build
│   ├── CMakeLists.txt
│   └── M2mConfig.h.in
├── config
│   └── m2m.conf
├── doc
│   └── m2m_protoco.md
├── include
│   ├── m2m.h       # m2m sdk and the 3rd party Application need to be included.
│   ├── m2m_api.h   # api that provide to thrid party applications.
│   ├── m2m_type.h  # data format declaration.
│   ├── util.h
│   └── utlist.h
├── lib
├── package
├── README.md
└── src
    ├── crypt
    ├── network
    └── util

```
# target 

    - Build m2m library for arduino/embedded device/server/ios or android application.
    - Build simple sample application for this library.

# build

```shell
$ cd build
$ cmake ...
$ make 

```
# sample 

    - server ：建立 udpserver 对包进行中转。
        -./app_server <server's id> <port> <secret key>  
    - device ： 建立 device， 回应广播包，同时监听对应 server 的包。
        - ./app_device <device's id> <port> <local key> <server's id> <server's host> <server's port>
        - 不链接服务器时可以：./app_device <device's id> <port> <local key>  
    - application: 构建客户端，并向 remote 发送数据
        - ./app_client <local id> <local port> < loacl secret key>< remote id > < remote host > <remote port> <remote key> < transmit data> <server id > < server host> < server port>
        - ./app_client <local id> <local port> < loacl secret key>< remote id > < remote host > <remote port> <remote key> < transmit data> 
```shell
$ cd bin
$ ./app_server  0  9527   000
$ ./app_device   1  9528 111 0 
$ ./app_client  2 9529 222 1 127.0.0.1 9528  111 sampleData 
$
$ ./app_device   1  9528 111 0 127.0.0.1  9527
$ ./app_client  2 9529 222 1 127.0.0.1 9528  111 sampleData 0 127.0.0.1  9527
$
```


