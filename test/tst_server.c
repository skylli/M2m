#include <string.h>
#include "../include/m2m_type.h"
#include "../include/m2m.h"
#include "../include/m2m_api.h"
#include "../include/m2m_log.h"
#include "../config/config.h"
#include "../include/app_implement.h"
#include "../include/util.h"
#include "../src/network/m2m/m2m_router.h"

#include "tst_config.h"

 /*******************
 ** It's an sample to show how to build an server use m2m library.
 ****/
 #define SERVER_HOST "127.0.0.1"
 #define REMOTE_HOST SERVER_HOST
 #define SERVER_PORT (TCONF_SERVER_PORT)
 #define LOCAL_SECRET_KEY   "1234567890123456"
 #define SERVER_SECRET_KEY  "1234560123456789"
 #define SERVER_SECRET_KEY2  "abcdefghijklnmmm"
 #define TESTA_DATA  "sending test data."
 #define SERVER_ID  "device id is 0123456"
 M2M_id_T local_id,remote_id;

 
 void receivehandle(u16 code,M2M_packet_T **pp_ack_data,M2M_packet_T *p_recv_data,void *p_arg){
    
 switch(code){
     case M2M_REQUEST_BROADCAST: 
         {
              M2M_packet_T *p_ack = mmalloc(sizeof(M2M_packet_T));
              p_ack->p_data = mmalloc( strlen(SERVER_ID) + 1 );
              p_ack->len = strlen(SERVER_ID);
              mcpy( (u8*)p_ack->p_data, (u8*)SERVER_ID, strlen(SERVER_ID));
              
              m2m_log_debug("server receive code = %d\n", code);
              if( p_recv_data->len > 0 && p_recv_data->p_data){
                   m2m_log("server receive data : %s\n",p_recv_data->p_data);
             }
             *pp_ack_data = p_ack;
         }
         break;
    case M2M_REQUEST_DATA:
        {
           if( p_recv_data->len > 0 && p_recv_data->p_data){
                   m2m_log("server receive data : %s\n",p_recv_data->p_data);
             }
        }
        break;
     default:
         break;
    }

 }

 int main(){
     M2M_T m2m;
     M2M_conf_T conf;
    // configuretion.
     conf.do_relay = 1;
     conf.max_router_tm = 60*1000;
     conf.def_enc_type = M2M_ENC_TYPE_AES128;
    
    /** config m2m *****/
     M2M_Return_T ret = m2m_int(&conf);
	m2m_log_init(M2M_LOG, "/tmp/vlog");
	m2m_log("in server test");

     /** server init. creat an server. ********/
     m2m.net = m2m_net_creat( &local_id,SERVER_PORT, strlen(TCONF_SERVER_KEY),(u8*)TCONF_SERVER_KEY, NULL,NULL, 0, (m2m_func)receivehandle,NULL);
     if( !m2m.net){
         m2m_log_error(" creat net failt !! \n");
         return -1;
     }
     while(1)
         m2m_trysync(m2m.net);
	 
	 m2m_log_uninit();
     return 0;
 }
// 保存 key。
/**********************************************
** description: 保存秘钥.每个 net 有一个秘钥。
** args:    
**          addr: 保存秘钥的地址.
**          p_key: 需要保存的秘钥，keylen: 秘钥的长度.
********/
int m2m_secretKey_write(size_t addr,u8 *p_key,u16 keylen){
    Enc_T *p_enc = (Enc_T*)p_key;
    printf(" >>> flash write key >>> succfully len =%d key : %s !!\t\n", p_enc->keylen,p_enc->key);
    return keylen;
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
void *m2m_relay_list_creat(){
    return relay_list_creat();
}
void m2m_relay_list_destory(void **pp_list_hd){

    relay_list_destory((void**) pp_list_hd);
    *pp_list_hd = NULL;
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
int m2m_relay_list_add( void **pp_r_list,M2M_id_T *p_id,M2M_Address_T *p_addr){ 

    return relay_list_add(pp_r_list, p_id, p_addr);
}
// 更新列表，当 id 超时没有刷新则直接删除该节点.
/**********************************************
** description: 更新路由列表.
** function:    1.sdk 会定时调用该函数，函数需要在每次调用是遍寻每一个 id 的注册时间，一旦超时则清理掉.
** args:  
**          p_r_list: 路由表的索引.
**          p_r_list: 最大的存活时间.
** output:  NULL.
********/
int m2m_relay_list_update(void **pp,u32 max_tm){
    return relay_list_update(pp, max_tm);
}

int m2m_relay_id_find(M2M_Address_T       *p_addr, void *p,M2M_id_T *p_id){ 
    return list_addr_find(p_addr, p, p_id);
}

