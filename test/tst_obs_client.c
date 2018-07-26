/*********************************************************
** 功能测试：observer 测试 Device 端.
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
#define TST_CLI_OBS_HOST      TCONF_HOST
#define TST_CLI_OBS_LOCAL_ID    (3)
#define TST_CLI_OBS_LOCAL_PORT  (9595)//TCONF_APP_PORT
#define TST_CLI_OBS_LOCAL_KEY   TCONF_APP_KEY

#define TST_CLI_OBS_SERVER_HOST TCONF_HOST
#define TST_CLI_OBS_SERVER_PORT TCONF_SERVER_PORT

#define TST_CLI_OBS_REMOTE_ID    (1)
#define TST_CLI_OBS_REMOTE_HOST  "127.0.0.1"//("ec2-54-153-105-41.us-west-1.compute.amazonaws.com")
#define TST_CLI_OBS_REMOTE_PORT   (9627)//TCONF_DEVICE_PORT
#define TST_CLI_OBS_REMOTE_KEY1   "111"//TCONF_DEVICE_KEY

#define TST_CLI_OBS_DATA1	("OBSERVER DATA1")
/*************************************************************/

// todo
int notify_destory = 0;
int loop_count = 0;
M2M_id_T device_id,server_id;
void obs_cli_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);
void obs_cli_notify_callback(int code,M2M_packet_T **pp_ack_data,M2M_packet_T *p_recv_data,void *p_arg);

int main(void){
    // 创建 net ， 谅解到远端服务器。
    // M2M_Return_T m2m_int(M2M_conf_T *p_conf);
    
    M2M_T m2m;
    M2M_conf_T conf;
    int ret;
	M2M_id_T rid;
	void *p_node = NULL;
	int notify_break = 0;
	mmemset(&rid, 0, sizeof(M2M_id_T) );
    device_id.id[ID_LEN -1] = TST_CLI_OBS_LOCAL_ID; // 
	rid.id[0] = TST_CLI_OBS_REMOTE_ID;
	
    conf.def_enc_type = M2M_ENC_TYPE_AES128;
    conf.max_router_tm = 10*60*1000;
    conf.do_relay = 0;
    ret = m2m_int(&conf);
	
	m2m_printf("version %s \n",m2m_version());

    m2m.net = m2m_net_creat( &device_id,TST_CLI_OBS_LOCAL_PORT, strlen(TST_CLI_OBS_LOCAL_KEY), TST_CLI_OBS_LOCAL_KEY,\
                             NULL,NULL, NULL,(m2m_func)obs_cli_callback, NULL);
	if( m2m.net == 0 ){
        m2m_printf(" creat network failt !!\n");
        return -1;
    }
 
	// Creat session
	//m2m.session = m2m_session_creat(size_t net, M2M_id_T * p_id, u8 * p_host, int port, int key_len, u8 * p_key, m2m_func func, void * p_args);
	
	m2m.session = m2m_session_creat( m2m.net, &rid, &TST_CLI_OBS_REMOTE_HOST, TST_CLI_OBS_REMOTE_PORT, 
									strlen(TST_CLI_OBS_REMOTE_KEY1), TST_CLI_OBS_REMOTE_KEY1, (m2m_func)obs_cli_callback,NULL);
	_RETURN_EQUAL_0(m2m.session, -1);
	// start to observer 
	p_node = m2m_session_observer_start(&m2m, TYPE_ACK_MUST, strlen(TST_CLI_OBS_DATA1), TST_CLI_OBS_DATA1, obs_cli_notify_callback, &notify_break);
#if 1
   	//while(loop_count < 3){
	while(1){
		m2m_trysync( m2m.net );
		//if(notify_break > 0)
		//	break;
		if(notify_destory){
			m2m_printf("re observer new ............");
			p_node = m2m_session_observer_start(&m2m, TYPE_ACK_MUST, strlen(TST_CLI_OBS_DATA1), TST_CLI_OBS_DATA1, obs_cli_notify_callback, &notify_break);
			notify_destory = 0;
		}
		if( loop_count >  3)
			break;
    }
#endif
	if(p_node)
		ret = m2m_session_observer_stop(&m2m, p_node);

	m2m_session_destory(&m2m);
    m2m_net_destory(m2m.net);
    m2m.net = 0;
	
	return 0;
}
void obs_cli_callback(int code,M2M_packet_T **pp_ack_data,M2M_packet_T *p_recv_data,void *p_arg){

    switch(code){
        case M2M_REQUEST_BROADCAST: 
           {
                 M2M_packet_T *p_ack = (M2M_packet_T*)mmalloc(sizeof(M2M_packet_T));
                 p_ack->p_data = (u8*)mmalloc( sizeof( M2M_id_T) + strlen(TST_CLI_OBS_HOST) + 1 );
                 p_ack->len = sizeof( M2M_id_T) + strlen(TST_CLI_OBS_HOST);
                 mcpy( (u8*)p_ack->p_data, (u8*)device_id.id, sizeof(M2M_id_T) );
                 mcpy((u8*)&p_ack->p_data[sizeof(M2M_id_T)], TST_CLI_OBS_HOST, strlen(TST_CLI_OBS_HOST));
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
void obs_cli_notify_callback(int code,M2M_packet_T **pp_ack_data, M2M_packet_T *p_recv_data,void *p_arg){

	m2m_log_debug("client receive code = %d\n", code);
	
	switch(code){
		case M2M_HTTP_OK:
			if(p_arg)
				*((int*)p_arg) = 1;
			m2m_log(" observer creat successfully.");
			break;
		case M2M_REQUEST_NOTIFY_PUSH:
			{
				  M2M_packet_T *p_ack = (M2M_packet_T*)mmalloc(sizeof(M2M_packet_T));
				  p_ack->p_data = (u8*)mmalloc( sizeof( M2M_id_T) + strlen(TST_CLI_OBS_HOST) + 1 );
				  p_ack->len = sizeof( M2M_id_T) + strlen(TST_CLI_OBS_HOST);
				  mcpy( (u8*)p_ack->p_data, (u8*)device_id.id, sizeof(M2M_id_T) );
				  mcpy((u8*)&p_ack->p_data[sizeof(M2M_id_T)], TST_CLI_OBS_HOST, strlen(TST_CLI_OBS_HOST));
				  
				  m2m_log("receive an notify.");
				  if( p_recv_data->len > 0 && p_recv_data->p_data){
					   m2m_log("notify data : %s\n",p_recv_data->p_data);
				 }
				  if(pp_ack_data)
				 		*pp_ack_data = p_ack;
				  (*((int*)p_arg))++;
				  loop_count++;
			 }
			break;
		case M2M_ERR_OBSERVER_DISCARD:
			m2m_log(">>>>>>>>>>>>>>>>>>>> observer have been destory.");
			if(!p_arg  )
				break;
			notify_destory = 1;

			break;

	
		case M2M_ERR_REQUEST_DESTORY:
			notify_destory =1;
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
int m2m_relay_id_find( M2M_Address_T       *p_addr,void *p_r_list,M2M_id_T *p_id){ return 0;}


