/***********************************************************************
** client  *****************************************************
** 作用：构建 network 跟远端建立 session 连接，发送数据.
** 使用: ./app_client <本地 ID>  <本地port> <本地 key> <远端 ID> <dev端 host> <dev端 port> <dev 端key> <传输的 data>
**              <服务器 ID> <服务器 host> <服务器 port> 
**          随后通过键盘输入 data，按下 enter 发送。
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


void app_callback(int code,M2M_packet_T **pp_ack_data,M2M_packet_T *p_recv_data,void *p_arg);
void transmit_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);

/***************** 配置****************************************************/

/********************************************************************/
int main(int argc, char **argv){
    int i = 0, ret = 0;
    
    M2M_T m2m;
    M2M_id_T l_id,r_id, h_id;
    int l_port,s_port,r_port;
    u8 l_key[20],r_key[20];
    int l_keylen = 0,r_keylen = 0;
    u8 *p_send_data = NULL,*p_remote_host = NULL, *p_server_host = NULL;
    int recv_flag = 0, slen = 0;

    mmemset((u8*)&l_id, 0, sizeof(M2M_id_T));
    mmemset((u8*)&r_id, 0, sizeof(M2M_id_T));
    mmemset((u8*)&h_id, 0, sizeof(M2M_id_T));
	
    mmemset((u8*) l_key,0, 20);
    mmemset((u8*) r_key,0, 20);
        
    
    if(argc < 8){
        m2m_printf("Need more parameter/n please input : %s <local id> <local port> < loacl secret key>",argv[0]);
        m2m_printf("< remote id > < remote host > <remote port> <remote key> < transmit data>");    
        m2m_printf(" <server id > < server host> < server port> \n");
        return -1;
    }

    
    /******** net config *************/
    M2M_conf_T dconf;
    dconf.def_enc_type = M2M_ENC_TYPE_AES128;
    dconf.max_router_tm = 10*60*1000;
    dconf.do_relay = 0;

    for(i=0;i<argc;i++)
        m2m_printf("<%s>",argv[i]);
    
    m2m_printf("\t\n");
// get local parameter.
    //get local id
    STR_2_INT_ARRAY( l_id.id, argv[1], strlen(argv[1]));
    // get local port 
    l_port = atoi(argv[2]);
    // get local secret key
    l_keylen = strlen(argv[3]);
    l_keylen = (l_keylen>16)?16:l_keylen;
    mcpy( (u8*)l_key, (u8*)argv[3], l_keylen);
    l_keylen = 16;

    // 3 m2m_printf("< remote id > < remote host > <remote port> <remote key> < transmit data>");    
// get remote parameter
    // get remote id 
    STR_2_INT_ARRAY( r_id.id, argv[4], strlen(argv[4]));
    // get remote host 
    p_remote_host = (u8*)argv[5];
    // get local port 
    r_port = atoi(argv[6]);
    // get local secret key
    r_keylen = strlen(argv[7]);
    r_keylen = (r_keylen>16)?16:r_keylen;
    mcpy( (u8*)r_key, (u8*)argv[7], r_keylen);
    r_keylen = 16;
    // get transmit data 
    slen = hadc_package_encode_alloc(&p_send_data,HADC_TYPE_UART,strlen(argv[8]), argv[8]);

// get server parameter
// 8  m2m_printf(" <server id > < server host> < server port> \n");
    if( argc >= 11){
        // get server id;
        STR_2_INT_ARRAY( h_id.id, argv[9], strlen(argv[9]));
        // get server host
        p_server_host = (u8*)argv[10];
        // get server port 
        s_port = atoi(argv[11]);
    }

    // printf all config 
    m2m_printf("local port = %d\n", l_port);
    m2m_bytes_dump((u8*)"local id: ", l_id.id, sizeof(M2M_id_T));
    m2m_bytes_dump((u8*)"local secret key : ", l_key, l_keylen);
    
    m2m_bytes_dump((u8*)"remote id: ", r_id.id, sizeof(M2M_id_T));
    m2m_printf((u8*)"remote host %s port %d\n", p_remote_host, r_port);
    m2m_bytes_dump("remote secret key : ", r_key, r_keylen);
    m2m_printf((u8*)"send data %s to remote \n", p_send_data);

    if(argc >= 11){
        m2m_printf("server port = %d\n", s_port);
        m2m_bytes_dump((u8*)"server id: ", &h_id, sizeof(M2M_id_T));
        m2m_printf("server host %s port %d\n", p_server_host, s_port);
    }
    
    // build net 
    m2m_int(&dconf);
    if(argc < 11)
        m2m.net = m2m_net_creat(&l_id,l_port,l_keylen, l_key, NULL,NULL, 0, (m2m_func)app_callback, NULL);
    else 
        m2m.net = m2m_net_creat(&l_id,l_port,l_keylen, l_key, &h_id,p_server_host, s_port, (m2m_func)app_callback, NULL);

    if( !m2m.net ){
        m2m_printf("creat net failt !!\n");
        return -1;
    }

    // creat session 
    m2m.session = m2m_session_creat(m2m.net, &r_id, p_remote_host, r_port, r_keylen, r_key, (m2m_func)app_callback, NULL);
    if( !m2m.session){
        m2m_printf("session creat failt !!\n");
        m2m_net_destory(m2m.net);
        return -1;
    }
    // sen out 
    ret = m2m_session_data_send(&m2m, slen, p_send_data, (m2m_func)transmit_callback, &recv_flag);
    while(recv_flag == 0){
        m2m_trysync(m2m.net);
    }

    mfree(p_send_data);
    m2m_session_destory(&m2m);
    m2m_net_destory(m2m.net);
    
    return 0;

}

void app_callback(int code,M2M_packet_T **pp_ack_data,M2M_packet_T *p_recv_data,void *p_arg){
    
     m2m_log_debug("app_callback:: receive code = %d\n", code);
 }
void transmit_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg){
    m2m_log_debug("callback:: receive code = %d\n", code);
    if( p_recv_pkt && p_recv_pkt->len > 0 && p_recv_pkt->p_data){
        m2m_log("receive data : %s\n",p_recv_pkt->p_data);
        m2m_bytes_dump("recv dump : ",p_recv_pkt->p_data, p_recv_pkt->len);
    }
    if(p_arg){
        *((int*) p_arg) = -1;
        if( code >= M2M_HTTP_OK )
            *((int*) p_arg) = 1;
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
void *m2m_relay_list_creat(){ return NULL;}
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
int m2m_relay_list_add( void **PP,M2M_id_T *p_id,M2M_Address_T *p_addr){ return 0;}
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

int m2m_relay_id_find(M2M_Address_T *p_addr, void *p_r_list,M2M_id_T *p_id){ return 0; }

