/***********************************************************************
** devices  *****************************************************
** 作用：构建 device，只响应数据.
** 使用: ./app_device <本地 ID> <本地port> <本地 key> <服务器 id> <服务器 host> <服务器 port>  
**
***********************************************************************
**********************************************************************/
#include <stdlib.h>
#include <string.h>
#include "../include/m2m_type.h"
#include "../include/m2m.h"
#include "../include/m2m_api.h"
#include "../include/m2m_log.h"
#include "../config/config.h"
#include "../include/util.h"
#include "../include/app_implement.h"
#include "../three_party/hadc_protocol/hadc.h"

/** 设备端 配置 ***********************************************************/
static M2M_id_T *p_device_id = NULL;
static int app_d_end = 3;
/*************************************************************/
void dev_callback(int code,M2M_packet_T **pp_ack_data,M2M_packet_T *p_recv_data,void *p_arg);

int main(int argc, char **argv){

    M2M_T dm2m;
    int i, ret,keylen;
    M2M_id_T d_id,s_id;
    int dport = 0,sport = 0;
    u8 p_key[20];
    M2M_conf_T dconf;
    u8 *p_server_host = NULL;

    /******** server config *************/
    dconf.def_enc_type = M2M_ENC_TYPE_AES128;
    dconf.max_router_tm = 10*60*1000;
    dconf.do_relay = 0;

    /******** get parameter *************/
    if(argc < 3){
       m2m_printf("Need more parameter/n please input : %s <device's id> <port> <local key> <server's id> <server's host> <server's port>\n",argv[0]);
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

    mmemset( (u8*) &d_id,0,sizeof(M2M_id_T));
    mmemset( (u8*) &s_id,0,sizeof(M2M_id_T));
    // get id
    STR_2_INT_ARRAY( d_id.id, argv[1], strlen(argv[1]));
    p_device_id = &d_id;
    // get port 
    dport = atoi(argv[2]);
    // get key
    mmemset( p_key, 0, 20);
    keylen = strlen(argv[3]);
    keylen = (keylen>16)?16:keylen;
    mcpy( (u8*)p_key, (u8*)argv[3], keylen);
    
    if(argc >=6){
    // get server id
        STR_2_INT_ARRAY( s_id.id, argv[4], strlen(argv[4]));
    // get server host
        p_server_host = argv[5];
        sport = atoi(argv[6]);
    }

    m2m_bytes_dump((u8*)"\n\tdevice id: ", d_id.id, sizeof(M2M_id_T));
    m2m_printf("\tdevice listing port %d.\n",dport);
    m2m_bytes_dump((u8*)"\tdevice secret key: ",p_key,16);

    if(argc >=6){
        m2m_bytes_dump("\tdevice id: ",s_id.id, sizeof(M2M_id_T));
        m2m_printf("\tserver host %s\n",p_server_host);
        m2m_printf("\tserver port %d\n",sport);
    }

    m2m_int(&dconf);
    dm2m.net = m2m_net_creat(&d_id, dport, 16, p_key, &s_id,p_server_host, sport, (m2m_func)dev_callback, &app_d_end);
    while(app_d_end){
       m2m_trysync(dm2m.net);
    }

    m2m_net_destory(dm2m.net);
    m2m_deint();
    return 0;
}

void dev_callback(int code,M2M_packet_T **pp_ack_data,M2M_packet_T *p_recv_data,void *p_arg){
    m2m_log_debug("callback:: receive code = %d\n", code);

    switch(code){
        case M2M_REQUEST_BROADCAST: 
            {
                 M2M_packet_T *p_ack = mmalloc(sizeof(M2M_packet_T));
                 p_ack->p_data = mmalloc( sizeof( M2M_id_T) + 1 );
                 p_ack->len = sizeof( M2M_id_T);
                 mcpy((u8*) p_ack->p_data, (u8*)p_device_id->id, sizeof(M2M_id_T) );
                 
                 m2m_log_debug("server receive code = %d\n", code);
                 if( p_recv_data->len > 0 && p_recv_data->p_data){
                      m2m_log("server receive data : %s\n",p_recv_data->p_data);
                }
                *pp_ack_data = p_ack;
            }
            break;
    case M2M_REQUEST_DATA:
           if( p_recv_data && p_recv_data->len > 0 && p_recv_data->p_data){
                u8 *p_data=NULL, cmd =0;
                int recv_len = hadc_package_decode( &p_data, &cmd, p_recv_data->len, p_recv_data->p_data);
                if(recv_len){
                    m2m_log("receive cmd %d data : %s\n", cmd,p_data);
                }

                if(p_arg ) {
                       *((int*) p_arg) = *((int*) p_arg) - 1;
                   }
            }
           break;
        default:
            if( p_recv_data && p_recv_data->len > 0 && p_recv_data->p_data){
                m2m_log("receive data : %s\n",p_recv_data->p_data);
                m2m_bytes_dump((u8*)"recv dump : ",p_recv_data->p_data, p_recv_data->len);
                 if(p_arg ) {
                        *((int*) p_arg) = *((int*) p_arg) - 1;
                    }
            }
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
int m2m_relay_id_find(M2M_Address_T *p_addr, void *p_r_list,M2M_id_T *p_id){ return 0;}


