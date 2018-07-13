/***********************************************************************
** devices  *****************************************************
** 作用：构建 server，中转数据，不响应数据。
** 使用: ./app_server <本地 ID> <本地port> <本地 key>.
**
***********************************************************************
**********************************************************************/
#include <stdlib.h>
#include <string.h>
#include "../include/m2m_type.h"
#include "../include/m2m.h"
#include "../include/m2m_api.h"
#include "../src/util/m2m_log.h"
#include "../config/config.h"
#include "../include/util.h"
#include "../include/app_implement.h"
#include "../src/network/m2m/m2m_router.h"

M2M_id_T *p_sid;
 void receivehandle(u16 code,M2M_packet_T **pp_ack_data,M2M_packet_T *p_recv_data,void *p_arg);

 int main(int argc, char **argv){
     M2M_T sm2m;
    int i, ret,keylen;
    M2M_id_T s_id;
    int sport = 0;
    u8 p_key[20];
    M2M_conf_T sconf;

    /******** server config *************/
    sconf.def_enc_type = M2M_ENC_TYPE_AES128;
    sconf.max_router_tm = 10*60*1000;
    sconf.do_relay = 1;

    /******** get parameter *************/
    if(argc < 3){
        m2m_printf("Need more parameter/n please input : %s <server's id> <port> <secret key> \n",argv[0]);
        return -1;
    }

    m2m_printf("server start: ");
    for(i=0;i<argc;i++)
        m2m_printf("{%s}",argv[i]);
    // get id
    if( sizeof(M2M_id_T) < strlen(argv[1]) ){
       m2m_printf("\n\tsecret key %s was too largst\n", argv[1]);
       return -1;
    }

    mmemset( (u8*)&s_id,0,sizeof(M2M_id_T));
    // get id
    STR_2_INT_ARRAY( s_id.id, argv[1], strlen(argv[1]));
    p_sid = &s_id;
    // get port 
    sport = atoi(argv[2]);
    // get key
    mmemset( (u8*)p_key, 0, 20);
    keylen = strlen(argv[3]);
    keylen = (keylen>16)?16:keylen;
    mcpy( (u8*)p_key, (u8*)argv[3], keylen);

    
    m2m_bytes_dump((u8*)"\n\tserver id: ", s_id.id, sizeof(M2M_id_T));
    m2m_printf("\tserver listing port %d.\n",sport);
    m2m_bytes_dump((u8*)"\tserver secret key: ",p_key,16);
    m2m_printf("\t\n");
    
    m2m_int(&sconf);
    sm2m.net = m2m_net_creat(&s_id, sport, 16, p_key, NULL, NULL, 0,(m2m_func)receivehandle, NULL);
    while(1){
        m2m_trysync(sm2m.net);
    }

    m2m_net_destory(sm2m.net);
    m2m_deint();
    return 0;
 }

 void receivehandle(u16 code,M2M_packet_T **pp_ack_data,M2M_packet_T *p_recv_data,void *p_arg){
     m2m_log_debug("callback:: receive code = %d\n", code);
     switch(code){
         case M2M_REQUEST_BROADCAST: 
             {
                  M2M_packet_T *p_ack = mmalloc(sizeof(M2M_packet_T));
                  p_ack->p_data = mmalloc( sizeof(M2M_id_T) );
                  p_ack->len = sizeof(M2M_id_T);
                  mcpy( (u8*)p_ack->p_data, (u8*)p_sid->id, sizeof(M2M_id_T));
                  
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

M2M_Address_T *m2m_relay_id_find( void *p,M2M_id_T *p_id){ 
    return list_addr_find( p, p_id);
}

