# 简介
m2m 基于 udp 层的链接协议，能自动维护 session，同时知道 aes 加密，用于 iot 之间的通讯，协议自身有路由的设计，可以在节点之间终中转传递，该部分尚未支持。

# M2M 协议 API

M2M 底层是 coap 封装包，提供路由(未实现，只能单向，有缺陷)，提供连接维持。


## 各大模块简介
### net
sdk 可以建立多个net， 每一个 net 维护一个 socket，一个 net 可以单独指定一个 host，sdk 会自动发 ping 到 host 维持跟 host 的连接， 
server host 一般会利用该 ping 包的注册该net的ip 和 port， 如此，其它 net 可以通过 server host 找到对应的 id 的 net。

  - 每个 net 均独占一个 socket。
  - 每个 net 拥有自身的秘钥 aes key， 该net下创建的 session 初始化的秘钥通net 一致，知道对端进行更新。
  - 每个 net 可以设置一个对应的 server host，并定时向改 host 发送 ping 包维持链接，便于其它 net 通过 server host 中转数据包。
  - 一旦 net 接受到不到 server host 回应的 ping 包，则认为该net 与 server host 处于短线状态。

### session
 一般的数据传输通过 session进行，session 所有的资源都是继承 net的，通过 net 的 socket 收发数据包。

  - 每个 session 通过 ctoken, stoken 唯一标识， stoken 由回话发起端生成，ctoken 由回话接受端生成。
  - session 没有拿到 对端token 时为未认证状态，该状态下所有的请求均暂存不会发送到对端。
  - 每个 session 会主动传输 ping 包以维持该会话不被链路回收。
  - 每个 session 均有自身独有的秘钥 aes-key, 创建时初始化为 net 秘钥。

## API 函数

## 初始化

typedef struct M2M_CONF_T{
    u8 def_enc_type;  // 秘钥的类型，默认使用 M2M_ENC_TYPE_AES128
    u8 do_relay;	 // 是否支持中转，若开启该功能则需要实现中转函数，只有服务的需要打开该功能。	
    Func_arg cb;	//  仅仅在 python 用于释放资源。
    u32 max_router_tm;
}M2M_conf_T;


### m2m_net_creat
// 创建一个 net
// 一个 net 维护一个 port，同时在一个 net 里可以创建多个 session
/**
** decription: 创建 net，建立 socket，初始化本地一个 net。
**          net 可以维护多个 session，同时维持一个跟 host 的连接.
**          server 端单独调用该接口就可以直接 调用 trysync 对表进行接收并处理;
** args:    p_id - net 的 id, port - socket 端口.
**          p_key - net 本地的秘钥，其它 net 要跟该net 通讯时需要使用该秘钥加解密。 key_len - 秘钥长度.
**          p_host - 该 net 连接的 host.
**          p_func - 该 net 接收到数据包的回调函数. 作为 server 时，net 接收到的包会触发该回调。
**              注意： 收到发送给本 id 的包才会触发，其它包则丢弃，或者 直接转发.
**              
**********************/
size_t m2m_net_creat( M2M_id_T *p_id,int port,u8 *p_key, int key_len,u8 *p_host,Func_arg *p_func)
/**
**description: 销毁 net。
**          清除net 内所有的 session，以及对应的节点，关闭 socket。
****************/
M2M_Return_T m2m_net_destory(size_t net);

/**
** description : 创建 session.
**          1. 获取远端    ip 和 port.
**          2. 创建 session.
** args:
**      net - 当下使用的 net. p_id -  远端的 id.
**      p_host - 远端 host.   p_enc - 远端 net 的加密.
**      p_user_func - creat 创建成功触发的回调。
*************************/
size_t m2m_session_creat(size_t net,M2M_id_T *p_id,u8 *p_host,Enc_T *p_enc,Func_arg *p_user_func);

## 注意 

