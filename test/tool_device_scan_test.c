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
#include <stdlib.h>

#include "tst_config.h"

/*******************
** It's an tool for testing .
****/
/***************** 测试配置****************************************************/
#define TOOL_LOCAL_PORT      (9510)
#define TOOL_LOCAL_KEY       TCONF_APP_KEY

#define TOOL_APP_LOCAL_ID    (1)
#define TOOL_REMOTE_ID       (2)


#define TOOL_REMOTE_HOST     TCONF_HOST
#define TOOL_REMOTE_PORT     (9529)
#define TOOL_SECRET_KEY1     TCONF_DEVICE_KEY
#define TOOL_DATA_STR  "sending test data."

#define TOOL_BROADCAST_DATA  "search device"

/********************************************************************/
typedef struct _DEV_INFO_T{
    M2M_id_T id;
    u8 f_get_dev;
    u8 f_build_session;
    u8 f_transmit;
    u8 p_host[32];
}DEV_info_T;

static M2M_id_T local_id;

void tool_net_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);
void tool_scan_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);
void tool_func_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);


int tool_result(int *p_ret,u8 **p_name, int items);
void tool_onlineCheck_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);

#define WAIT_UNTIL(n,c,net)   while(n != c){ \
                                 m2m_trysync(net);}
int main(){
    DEV_info_T dev;
    M2M_T m2m;
    M2M_conf_T conf;
    // configuretion.
    conf.do_relay = 0;
    conf.max_router_tm = 10*60*1000;
    conf.def_enc_type = M2M_ENC_TYPE_AES128;

    /** config m2m *****/
    m2m_record_level_set(M2M_LOG_WARN);
    M2M_Return_T ret = m2m_int(&conf);
    int current_tm = 0,tmp_tm = 0;
    local_id.id[ID_LEN -1] = TOOL_APP_LOCAL_ID; // 
    mmemset((u8*)&dev, 0, sizeof(DEV_info_T));
    
    /**1. 建立 network ********/
    m2m.net = m2m_net_creat( &local_id, TOOL_LOCAL_PORT, strlen(TOOL_LOCAL_KEY), TOOL_LOCAL_KEY, NULL,NULL, 0, NULL,NULL);
    if( !m2m.net){
        m2m_log_error(" creat net failt !! \n");
        return -1;
    }
    /*2  在线设备查询 ***/
#if 0
    mmemset((u8*)&remote_id, 0 , sizeof(M2M_id_T));
    m2m_dev_online_check((Net_T*) m2m.net, TST_SERVER_HOST, TST_SERVERT_PORT, &local_id, (m2m_func)test_onlineCheck_callback,&getdeviceId);
    WAIT_UNTIL(getdeviceId,1, m2m.net);
#endif
    while(1){
    /**2 发送广播包**********/
#ifdef CONF_BROADCAST_ENABLE
        mmemset((u8*)&dev, 0 , sizeof(DEV_info_T));
        dev.f_get_dev = 0;
        ret = m2m_broadcast_data_start((size_t)m2m.net, TOOL_REMOTE_PORT,strlen( TOOL_BROADCAST_DATA), TOOL_BROADCAST_DATA,(m2m_func)tool_scan_callback,&dev);
        m2m_printf("scanding devices ...\n");
        WAIT_UNTIL(dev.f_get_dev,1, m2m.net);
        
        m2m_printf("stop broadcast ...\n");
        m2m_broadcast_data_stop( m2m.net);
#endif // CONF_BROADCAST_ENABLE
        /**3 创建会话*********/
        dev.f_build_session = 0;
        m2m.session = m2m_session_creat( m2m.net, &dev.id, dev.p_host, TOOL_REMOTE_PORT, strlen(TOOL_SECRET_KEY1),TOOL_SECRET_KEY1,(m2m_func)tool_net_callback,&dev);
        if( !m2m.session ){
            m2m_log_error(" creat session failt !! \n");
            return -1;
        }
        /**4. 发送数据********/
        dev.f_transmit = 0;
        m2m_printf("sending data to device ...\n");
        ret = m2m_session_data_send(&m2m, strlen(TOOL_DATA_STR), TOOL_DATA_STR, (m2m_func)tool_func_callback ,&dev);
        while( 0 == dev.f_transmit){
            m2m_trysync(m2m.net);
        }
        m2m_session_destory(&m2m);
    }
    m2m_net_destory(m2m.net);
    m2m_deint();
    return ret;
}
void tool_net_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);
void tool_scan_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);
void tool_func_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);

void tool_net_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg){
    m2m_log_debug(">>>>>>> net callback:: receive code = %d\n", code);
    if( p_recv_pkt && p_recv_pkt->len > 0 && p_recv_pkt->p_data){
        m2m_log("receive data : %s\n",p_recv_pkt->p_data);
        m2m_bytes_dump("recv dump : ",p_recv_pkt->p_data, p_recv_pkt->len);
    }
} 
void tool_scan_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg){
    m2m_log_debug(">>>>>>> net callback:: receive code = %d\n", code);
    
    if( code == M2M_REQUEST_BROADCAST_ACK  && p_recv_pkt \
        && p_recv_pkt->len >= sizeof(M2M_id_T) && p_recv_pkt->p_data){
        m2m_bytes_dump("receive device id : ",p_recv_pkt->p_data, p_recv_pkt->len);
        if(p_arg){
            DEV_info_T *p_dev = (DEV_info_T*)p_arg;
            mcpy((u8*)&p_dev->id, p_recv_pkt->p_data, sizeof(M2M_id_T));
            mcpy((u8*)p_dev->p_host, &p_recv_pkt->p_data[sizeof(M2M_id_T)], (p_recv_pkt->len - sizeof(M2M_id_T))  );
            m2m_printf("\tremote device ip: %s\n",p_dev->p_host);
            p_dev->f_get_dev = 1;
        }
    }
} 
void tool_func_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg){
    m2m_log_debug(">>>>>>> net callback:: receive code = %d\n", code);
    if(  p_recv_pkt && p_recv_pkt->len > 0 && p_recv_pkt->p_data){
        m2m_log("receive data : %s\n",p_recv_pkt->p_data);
        m2m_bytes_dump("recv dump : ",p_recv_pkt->p_data, p_recv_pkt->len);
    }
    if( p_arg && code >= M2M_HTTP_OK){
        DEV_info_T *p_dev = (DEV_info_T*)p_arg;
        p_dev->f_transmit = 1;
        m2m_printf("test successfully.\n");
    }
} 


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

M2M_Address_T *m2m_relay_id_find( void *p_r_list,M2M_id_T *p_id){ return 0; }

