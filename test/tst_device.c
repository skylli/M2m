/*********************************************************
** 功能测试： token, key 设置,广播测试.
*********************************************************/
#include <string.h>
#include "../include/m2m_type.h"
#include "../include/m2m.h"
#include "../include/m2m_api.h"
#include "../src/util/m2m_log.h"
#include "../config/config.h"
#include "../include/app_implement.h"
#include "../include/util.h"


/** 设备端 配置 ***********************************************************/
#define TST_DEV_LOCAL_ID    (2)
#define TST_DEV_LOCAL_PORT  DEFAULT_DEVICE_PORT
#define TST_DEV_LOCAL_KEY   DEFAULT_DEVICE_KEY

#define TST_DEV_SERVER_HOST DEFAULT_HOST
#define TST_DEV_SERVER_PORT DEFAULT_SERVER_PORT



/*************************************************************/


M2M_id_T device_id,server_id;
void dev_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);

void main(void){
    // 创建 net ， 谅解到远端服务器。
    // M2M_Return_T m2m_int(M2M_conf_T *p_conf);
    
    M2M_T m2m;
    M2M_conf_T conf;
    int ret;

    device_id.id[ID_LEN -1] = TST_DEV_LOCAL_ID; // 

    conf.def_enc_type = M2M_ENC_TYPE_AES128;
    conf.max_router_tm = 10*60*1000;
    conf.do_relay = 0;
    ret = m2m_int(&conf);

    m2m.net = m2m_net_creat( &device_id,TST_DEV_LOCAL_PORT, strlen(TST_DEV_LOCAL_KEY),TST_DEV_LOCAL_KEY,\
                            TST_DEV_SERVER_HOST, TST_DEV_SERVER_PORT,(m2m_func)dev_callback,NULL);
    if( m2m.net == 0 ){
        m2m_printf(" creat network failt !!\n");
        return ;
    }
    while(1){
        m2m_trysync( m2m.net );
    }
    
    m2m_net_destory(m2m.net);
    m2m.net = 0;
}
void dev_callback(int code,M2M_packet_T **pp_ack_data,M2M_packet_T *p_recv_data,void *p_arg){

    switch(code){
        case M2M_REQUEST_BROADCAST: 
            {
                 M2M_packet_T *p_ack = (M2M_packet_T*)mmalloc(sizeof(M2M_packet_T));
                 p_ack->p_data = (u8*)mmalloc( sizeof( M2M_id_T) + 1 );
                 p_ack->len = sizeof( M2M_id_T);
                 mcpy( (u8*)p_ack->p_data, (u8*)device_id.id, sizeof(M2M_id_T) );
                 
                 m2m_log_debug("server receive code = %d\n", code);
                 if( p_recv_data->len > 0 && p_recv_data->p_data){
                      m2m_log("server receive data : %s\n",p_recv_data->p_data);
                }
                *pp_ack_data = p_ack;
            }
            break;
        default:
            break;
    }

 }


/**********************************************
** description: 读取秘钥.
** args:    
**          addr: 保存秘钥的地址.
** output:
**          p_key;p_keylen;
********/
u32 m2m_secretKey_read(size_t addr,u8 *p_key,u16 *p_keylen){ return 0;}
/** router 路由*******************************************/
/**********************************************
** description: 创建路由列表用于保存：id -- address --- alive time 键值对.
** args:   NULL
** output:
**          指向该 路由表的索引.
********/
void *m2m_relay_list_creat(){ return 0;}
void m2m_relay_list_destory(void **pp_list_hd){

    return ;
}

// 若  id 存在则更新时间.
/**********************************************
** description: 添加路由设备.
** function:    1.没有该 id 则添加，存在该 id 则更新 address 和时间.
** args:  
**          p_r_list: 路由表的索引.
**          p_id：id，p_addr: 对应的地址。
** output: < 0 则不成功.
********/
int m2m_relay_list_add( void **pp,M2M_id_T *p_id,M2M_Address_T *p_addr){ return 0;}
/** 删除****/
int m2m_relay_list_dele( void *p_r_list,M2M_id_T *p_id){ return 0;}
// 更新列表，当 id 超时没有刷新则直接删除该节点.
/**********************************************
** description: 更新路由列表.
** function:    1.sdk 会定时调用该函数，函数需要在每次调用是遍寻每一个 id 的注册时间，一旦超时则清理掉.
** args:  
**          p_r_list: 路由表的索引.
**          p_r_list: 最大的存活时间.
** output:  NULL.
********/
int m2m_relay_list_update(void **pp,u32 max_tm){  return 0;}
M2M_Address_T *m2m_relay_id_find( void *p_r_list,M2M_id_T *p_id){ return 0;}


