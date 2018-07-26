/*********************************************************
** 功能测试： token, key 设置,广播测试.
*********************************************************/
#include <string.h>
#include "../include/m2m_type.h"
#include "../include/m2m.h"
#include "../include/m2m_api.h"
#include "../include/m2m_log.h"
#include "../config/config.h"
#include "../include/app_implement.h"
#include "../include/util.h"

#include "tst_config.h"

/** 设备端 配置 ***********************************************************/
#define TST_DEVOBS_LOCAL_HOST  TCONF_HOST
#define TST_DEVOBS_LOCAL_ID    (2)
#define TST_DEVOBS_LOCAL_PORT  TCONF_DEVICE_PORT
#define TST_DEVOBS_LOCAL_KEY   TCONF_DEVICE_KEY

#define TST_DEVOBS_SERVER_HOST	TCONF_HOST
#define TST_DEVOBS_SERVER_PORT  TCONF_SERVER_PORT

#define TST_DEVOBS_NOTIFY_PUS1  ("abcd123")

// 开启了 键盘输入之后就不可以定时推送
//#define USE_KEYBOARD_INPUT	        // 开启键盘可输入 string 并推送
#define NOTIFY_INTERVAL_TM 	(15000)  // 定时发送 notify 的时间间隔
/*************************************************************/
typedef struct DEV_OBS_T
{
	void *p_node;
	int obs_rq_cnt;
	int notify_cnt;
	int reobserver_cnt;
	BOOL reobserver_en;
	BOOL notify_push_en;
	BOOL exit;
} Dev_obs_T;
// todo
int loop_count = 0;
M2M_id_T device_id,server_id;
void dev_callback(int code,M2M_packet_T **pp_ack_pkt, void *p_r,void *p_arg);

void main(void){
    // 创建 net ， 谅解到远端服务器。
    // M2M_Return_T m2m_int(M2M_conf_T *p_conf);
    
    M2M_T m2m, h_id;
    M2M_conf_T conf;
    int ret;
	Dev_obs_T obs;
	u32 old_tm = 0;
	char arr_input[512];
    device_id.id[ID_LEN -1] = TST_DEVOBS_LOCAL_ID; // 
    mmemset((u8*)&h_id, 0, sizeof(M2M_T));
	mmemset(&obs, 0, sizeof(Dev_obs_T));
	
    conf.def_enc_type = M2M_ENC_TYPE_AES128;
    conf.max_router_tm = 10*60*1000;
    conf.do_relay = 0;
    ret = m2m_int(&conf);

    m2m.net = m2m_net_creat( &device_id, TST_DEVOBS_LOCAL_PORT, strlen(TST_DEVOBS_LOCAL_KEY),TST_DEVOBS_LOCAL_KEY,\
                             &h_id,NULL, NULL,(m2m_func)dev_callback,&obs);
	
    if( m2m.net == 0 ){
        m2m_printf(" creat network failt !!\n");
        return ;
    }
    //while(loop_count < 3){
	while(1){
		m2m_trysync( m2m.net );
#if 1
		if(obs.p_node ){
#ifdef  USE_KEYBOARD_INPUT
			mmemset(arr_input, 0, 512);
			m2m_printf("input notify: ");
			scanf("%s", arr_input);
			m2m_printf("\npushing an new notify: %s", arr_input);
			m2m_session_notify_push( &m2m, obs.p_node, strlen(arr_input),arr_input, dev_callback, &obs);
			obs.p_node = NULL;
#else
			if(DIFF_(old_tm, m2m_current_time_get()) > NOTIFY_INTERVAL_TM){
				
				m2m_session_notify_push( &m2m, obs.p_node, strlen(TCONF_NOTIFY_DATA1),TCONF_NOTIFY_DATA1, dev_callback, &obs);
				old_tm = m2m_current_time_get();
				loop_count++;
			}
#endif
		}
//		if( obs.exit )
//			break;
#endif		
    }
    
    m2m_net_destory(m2m.net);
    m2m.net = 0;
}
void dev_callback(int code,M2M_packet_T **pp_ack_data,void *p_r, void *p_arg){
	M2M_obs_payload_T *p_robs = NULL;
	Dev_obs_T *p_devobs = NULL;
	M2M_packet_T *p_recv_data = (M2M_packet_T*)p_r;

	m2m_log("recv pointer %p", p_r);
	m2m_log("receive code = %d", code);
    switch(code){
        case M2M_REQUEST_BROADCAST: 
            {
                 M2M_packet_T *p_ack = (M2M_packet_T*)mmalloc(sizeof(M2M_packet_T));
                 p_ack->p_data = (u8*)mmalloc( sizeof( M2M_id_T) + strlen(TST_DEVOBS_SERVER_HOST) + 1 );
                 p_ack->len = sizeof( M2M_id_T) + strlen(TST_DEVOBS_SERVER_HOST);
                 mcpy( (u8*)p_ack->p_data, (u8*)device_id.id, sizeof(M2M_id_T) );
                 mcpy((u8*)&p_ack->p_data[sizeof(M2M_id_T)], TST_DEVOBS_SERVER_HOST, strlen(TST_DEVOBS_SERVER_HOST));
                 m2m_log_debug("server receive code = %d\n", code);
                 if( p_recv_data && p_recv_data->len > 0 && p_recv_data->p_data){
                      m2m_log("server receive data : %s\n",p_recv_data->p_data);
                }
                *pp_ack_data = p_ack;
            }
            break;
		case M2M_REQUEST_OBSERVER_RQ:

			if(!p_arg || !p_r)
				break;
			
			p_devobs = (Dev_obs_T*) p_arg;  
			p_robs = (M2M_obs_payload_T*) p_r;
			p_devobs->p_node = p_robs->p_obs_node;
			p_devobs->notify_push_en = 1;
			p_devobs->obs_rq_cnt++;
			m2m_log("receive an observer request.");
			if(p_robs->p_payload->len && p_robs->p_payload->p_data){
				m2m_log("request data: %s", p_robs->p_payload->p_data);
			}
			break;
		case M2M_ERR_OBSERVER_DISCARD:
			m2m_log("observer have been destory.");
			if(!p_arg || !p_r )
				break;
			p_devobs = (Dev_obs_T*) p_arg;  
			p_robs = (M2M_obs_payload_T*) p_r;
			p_devobs->p_node = p_robs->p_obs_node;			
			p_devobs->exit = 1;
			break;
		
		case M2M_REQUEST_NOTIFY_PUSH:
			if(!p_arg || !p_r)
				break;
			

			m2m_log("receive an notify request.");
			if(p_robs->p_payload->len && p_robs->p_payload->p_data){
				m2m_log("request data: %s", p_robs->p_payload->p_data);
			}
			break;			
		case M2M_REQUEST_NOTIFY_ACK:
			if(!p_arg || !p_r )
				break;

			p_devobs = (Dev_obs_T*) p_arg;  
			p_robs = (M2M_obs_payload_T*) p_r;
			p_devobs->p_node = p_robs->p_obs_node;
			p_devobs->notify_push_en = 1;

			
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
int m2m_relay_id_find(M2M_Address_T       *p_addr, void *p_r_list,M2M_id_T *p_id){ return 0;}


