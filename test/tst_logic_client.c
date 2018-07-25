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
** It's an sample to show how to build an client to send data request to server use m2m library.
****/
/***************** 测试配置****************************************************/
#define TST_LOCAL_PORT      TCONF_APP_PORT
#define TST_LOCAL_KEY       TCONF_APP_KEY

#define TST_APP_LOCAL_ID    (1)
#define TST_REMOTE_ID       (2)

#define TST_SERVER_HOST     TCONF_HOST
#define TST_SERVERT_PORT    TCONF_SERVER_PORT
#define TST_REMOTE_HOST     TCONF_HOST
#define TST_REMOTE_PORT     TCONF_DEVICE_PORT
#define TST_SECRET_KEY1     TCONF_DEVICE_KEY
#define TST_SECRET_KEY2     "00abcdefghijklnm11"
#define TST_DATA_STR  "sending test data."

#define TES_BROADCAST_DATA  "search device"

/********************************************************************/

M2M_id_T local_id,remote_id;
extern u8 g_log_level;
enum{
    TST_NET_CREAT =0,
    TST_ONLINE_CHECK_,

#ifdef CONF_BROADCAST_ENABLE
    TST_BORADCAST,
#endif // CONF_BROADCAST_ENABLE

    TST_SESSION_CREAT,
    TST_DATA,
    TST_OBS_ON,
    TST_NOTIFY,
    TST_OBS_OFF,
    TST_KEYSET,
    TST_TOKEN,
	TST_NET_KEY_SET,
	TST_CONNT_STATUS,
	TST_TOTAL,
	TST_MAX
}TEST_FUNCTIONS;
int tst_ret[TST_MAX];
char *tst_item_name[TST_MAX] = {
    "network creat",
    "online check",

#ifdef CONF_BROADCAST_ENABLE
    "broadcast ",
#endif // CONF_BROADCAST_ENABLE
    "session creat",    
    "data transmission",
    "observer start",
    "notify push",
    "observer stop",
	"session secret key update",
    "token update",
	"Net secret key set",
    "secret key setting",
	"connection state",
    "Finaly function"
};
void test_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);

int test_result(int *p_ret,u8 **p_name, int items);
void test_onlineCheck_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);

#define SRC_DATA    "ABCD1234"
#define WAIT_UNTIL(n,c,net)   while(n != c){ \
                                 m2m_trysync(net);}
int main(){
    M2M_T m2m, hid;
    M2M_conf_T conf;
	int connt_sta = 0;
    // configuretion.
    conf.do_relay = 0;
    conf.max_router_tm = 10*60*1000;
    conf.def_enc_type = M2M_ENC_TYPE_AES128;

    /** config m2m *****/
    M2M_Return_T ret = m2m_int(&conf);
    int current_tm = 0,tmp_tm = 0;
    local_id.id[ID_LEN -1] = TST_APP_LOCAL_ID; // 
    remote_id.id[ID_LEN -1] = TST_REMOTE_ID;
    int recv_flag = 0, recv_broadcast_flag = 0,setkey_flag = 0,token_flag = 0;
	void *p_obs_node = NULL;

	mmemset((u8*)&hid, 0, sizeof(M2M_T) );
    /**1. 建立 network ********/
    m2m.net = m2m_net_creat( &local_id, TST_LOCAL_PORT, strlen(TST_LOCAL_KEY), TST_LOCAL_KEY, NULL, TST_SERVER_HOST, TST_SERVERT_PORT, (m2m_func)test_callback,NULL);
    if( !m2m.net){
        m2m_log_error(" creat net failt !! \n");
        goto func_tst_end;
    }
    tst_ret[TST_NET_CREAT] = 1;
	
    /*2  在线设备查询 ***/
#if 1
    m2m_dev_online_check( m2m.net, TST_SERVER_HOST, TST_SERVERT_PORT, &local_id, (m2m_func)test_onlineCheck_callback,&tst_ret[TST_ONLINE_CHECK_]);
    WAIT_UNTIL(tst_ret[TST_ONLINE_CHECK_],1, m2m.net);
#endif
    /**2 发送广播包**********/
#if 1
//#ifdef CONF_BROADCAST_ENABLE
    ret = m2m_broadcast_data_start( m2m.net,TST_REMOTE_PORT,strlen(TES_BROADCAST_DATA),TES_BROADCAST_DATA,(m2m_func)test_callback,&tst_ret[TST_BORADCAST]);
    WAIT_UNTIL(tst_ret[TST_BORADCAST],1, m2m.net);
    
    m2m_log("stop broadcast ...");
    m2m_broadcast_data_stop( m2m.net);
#endif // CONF_BROADCAST_ENABLE

    /**3 创建会话*********/
    m2m.session = m2m_session_creat( m2m.net, &remote_id,TST_REMOTE_HOST, TST_REMOTE_PORT, strlen(TST_SECRET_KEY1),TST_SECRET_KEY1,(m2m_func)test_callback,&tst_ret[TST_SESSION_CREAT]);
    if( !m2m.session ){
        m2m_log_error(" creat session failt !! \n");
        goto func_tst_end;
    }
    /**4. 发送数据********/
    ret = m2m_session_data_send(&m2m, strlen(TST_DATA_STR), TST_DATA_STR, (m2m_func)test_callback ,&tst_ret[TST_DATA]);
    while(1){
        m2m_trysync(m2m.net);
		tst_ret[TST_SESSION_CREAT] = tst_ret[TST_DATA];
        if( tst_ret[TST_SESSION_CREAT])
            break;
    }
	/** 订阅***/
	p_obs_node = m2m_session_observer_start(&m2m, TYPE_ACK_MUST, strlen(TCONF_OBSERVER_DATA), TCONF_OBSERVER_DATA,(m2m_func)test_callback, &tst_ret[TST_OBS_ON]);
    WAIT_UNTIL(tst_ret[TST_OBS_ON],1, m2m.net);

	/** 推送***/	
	m2m_printf("waitting notify...\n");
	WAIT_UNTIL(tst_ret[TST_NOTIFY],1, m2m.net);
	
	m2m_printf("notify have receive...\n");
	/** 取消订阅********/
	if(p_obs_node){
		ret = m2m_session_observer_stop(&m2m, p_obs_node);		
		tst_ret[TST_OBS_OFF] = (ret == M2M_ERR_NOERR)?1:0;
	}
	
	ret = m2m_session_secret_set( &m2m, (strlen(TST_SECRET_KEY2)),(TST_SECRET_KEY2), (m2m_func)test_callback,&tst_ret[TST_KEYSET]);
    WAIT_UNTIL(tst_ret[TST_KEYSET],1, m2m.net);
	
    ret = m2m_session_token_update( &m2m, (m2m_func)test_callback,&tst_ret[TST_TOKEN]);
    WAIT_UNTIL(tst_ret[TST_TOKEN],1, m2m.net);
//	M2M_Return_T m2m_net_secretkey_set(size_t net,M2M_id_T *p_id,u8 *p_host,int port, int key_len,u8 *p_key,int newkey_len, u8 *p_newkey,m2m_func func, void *p_args){
	// set net secret key 
	ret = m2m_net_secretkey_set( m2m.net, &remote_id, TST_REMOTE_HOST, TST_REMOTE_PORT, 
			(strlen(TST_SECRET_KEY1)),(TST_SECRET_KEY1),(strlen(TST_SECRET_KEY2)),(TST_SECRET_KEY2), (m2m_func)test_callback,&tst_ret[TST_NET_KEY_SET] );
    WAIT_UNTIL(tst_ret[TST_NET_KEY_SET],1, m2m.net);
	
	// transmit data againt. 
    ret = m2m_session_data_send(&m2m, strlen(TST_DATA_STR), TST_DATA_STR, (m2m_func)test_callback ,&tst_ret[TST_TOTAL]);
    WAIT_UNTIL(tst_ret[TST_TOKEN],1, m2m.net);
	// get connection status.
	connt_sta = (int)m2m_session_connted(&m2m);
	tst_ret[TST_CONNT_STATUS] = (int)m2m_session_connted(&m2m);
func_tst_end:

    m2m_net_destory(m2m.net);
    m2m_log_error(" test end !! \n");
    ret = test_result(tst_ret, (u8**)tst_item_name, (TST_MAX -1) );
    return ret;
}
void test_onlineCheck_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg){
    m2m_log_debug(">>>>>>> online check callback:: receive code = %d\n", code);
    if( p_recv_pkt && p_recv_pkt->len > 0 && p_recv_pkt->p_data){
        m2m_log("receive data : %s\n",p_recv_pkt->p_data);
        if(p_recv_pkt->len == sizeof(M2M_Address_T)){
            M2M_Address_T *p_ip =  (M2M_Address_T*) p_recv_pkt->p_data;
            m2m_log("device was online and the ip is  %u.%u.%u.%u port: %d", p_ip->ip[0], p_ip->ip[1],p_ip->ip[2],p_ip->ip[3], p_ip->port);
        }
        m2m_bytes_dump("recv dump : ",p_recv_pkt->p_data, p_recv_pkt->len);
    
    }
    if(  p_arg){
        //*((int*) p_arg) = -1;
        if( code >= M2M_HTTP_OK )
            *((int*) p_arg) = 1;
        }
} 

void test_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg){
    m2m_log_debug(">>>>>>> net callback:: receive code = %d\n", code);
    if( p_recv_pkt && p_recv_pkt->len > 0 && p_recv_pkt->p_data){
        m2m_log("receive data : %s\n",p_recv_pkt->p_data);
        m2m_bytes_dump("recv dump : ",p_recv_pkt->p_data, p_recv_pkt->len);
    }
	switch (code){
		case M2M_REQUEST_NOTIFY_PUSH:
			 if( p_recv_pkt->len == strlen(TCONF_NOTIFY_DATA1) && p_recv_pkt->p_data \
			 	&& !memcmp(p_recv_pkt->p_data,TCONF_NOTIFY_DATA1 ,strlen(TCONF_NOTIFY_DATA1))){
					   m2m_log("notify data : %s\n",p_recv_pkt->p_data);
					   tst_ret[TST_NOTIFY] = 1;
				 }
			break;
		case M2M_REQUEST_BROADCAST_ACK:
			if(p_recv_pkt->len == sizeof(M2M_Address_T)){
	            M2M_Address_T *p_ip =  (M2M_Address_T*) p_recv_pkt->p_data;
	            m2m_log("device was online and the ip is  %u.%u.%u.%u port: %d", p_ip->ip[0], p_ip->ip[1],p_ip->ip[2],p_ip->ip[3], p_ip->port);
        	}
			if(p_arg){
		            *((int*) p_arg) = 1;
	        }	
		default:
		    if( code > 0 && p_arg){
		        if( code >= M2M_HTTP_OK )
		            *((int*) p_arg) = 1;
	        }	
			break;
	}

	
} 
int test_result(int *p_ret,u8 **p_name, int items){
    int i =0,test_result = 0;
    for(i=0;i<items;i++){
        if( p_ret[i] == 1){
            test_result++;
            m2m_printf(">>>>\t %s test successfully !\n",p_name[i]);
            }
        else
            m2m_printf(">>>>\t %s test have failt \n",p_name[i]);
    }
    if(test_result == items){
        
        m2m_printf(">> function test is totaly success \n");
        return 0;
        }
    else{
        m2m_printf(">> function test is failt. plasese have a check. \n");
        return 1;
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

M2M_Address_T *m2m_relay_id_find( void *p_r_list,M2M_id_T *p_id){ return 0; }

