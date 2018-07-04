# M2M 协议 API
M2M 底层是 coap 封装包，提供路由(未实现，只能单向，有缺陷)，提供连接维持。

  - 每一个 net 维护一个 socket，一个 net 可以单独指定一个 host，sdk 会自动发 ping 到 host 维持跟 host 的连接.
  - Net 下可以建立多个 session 跟远端连接。
  - session 有一套自身的 token,其加密的 key 由对端 net 确定。


## 命令

## API 函数

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