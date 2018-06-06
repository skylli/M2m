#  network
Network 即 socket 资源。每个 Network 占用一个 socket。 同一个 Network 上可以构建多个 session。

# 模式
Net 支持 server 模式，即响应 client 端的请求；同时也支持 client 模式，即主动发出请求。
## server 模式
先接收到 token_request 请求，sdk 产生一个 token 同时建立一个 session。后续的该 session 包直接通过 net 的回调返回到应用层，应用层再把数据返回给 sdk 发送到远端。注意sdk 会根据 message id 剔除早期的包。
## client 模式
应用层直接调用 api 建立 session，并发送 token requests 到远端。
# 加解密

# session 
## sdk 内部操作

	- 创建： 通过 network 获取对端的 token。
	- 会话的维持：session 的发起者会向对端定时发送 ping 包，以维持 session。
	- 数据重发： 数据包发出超时没有回应会自动重发。不需要重发的包例外，比如实时的 sensor 数据。
	- 数据接收处理：对 socket 接收到的数据查询归属的 session 并调用对应的回调函数返回到 APP 层。
	- 
	- 
## 操作
### 更新 token

	- 建立会话时获取 token。master 先发送 token requests，其token 为 0. slave 建立会话并生成 token。todo： 在 proto 层需要缓冲最近发送的包，当接收到重包时可以直接发出。
	- 刷新token。

## 通讯流程

### ping 机制
ping 用于维持连接，当底层协议为 tcp 时则没有发送 ping 的必要。 有两种 ping：

	- host 的 ping，host 为该 net 的server，需要维持与 server 的连接，当开启中转功能时，对于不是自己的包可以转发到 host。
	- 每一个 session 均有对应的 ping，其目的是维持 session 的连接。
	- 转发规则： 只有开启了转发的 net 才能转发，但是转发是单向的


