/***********************************************************************
** 多 session 测试 *****************************************************
** 描述：该测试通过同时建立 TST_MSES_MAX_NET 个 network，每个 network 单独跟 
**      TST_MSES_REMOTE_HOST/TST_MSES_REMOTE_PORT 远端建立 session，
**      分别进行token 获取，传输数据， 更新token， 再度传输数据验证 token 
**      更新的是否成功测试。
** 需求： 运行远端 device， 其 ip/port 必须跟该测试中的 
**       TST_MSES_REMOTE_HOST/TST_MNET_REMOTE_POR 对应。否则无法进行测试。
** 测试结果：
**     每个 session 的单项测试接收到 device 回应的数据包为成功，否则为失败，
**     失败的可能为传输超时，数据包本身有问题，session 未成功建立。
** 注意： 该测试没有进行秘钥设置，同时该测试也没有涉及 server 端。
***********************************************************************
**********************************************************************/

#include <string.h>
#include "../include/m2m_type.h"
#include "../include/m2m.h"
#include "../include/m2m_api.h"
#include "../src/util/m2m_log.h"
#include "../config/config.h"
#include "../include/app_implement.h"
#include "../include/util.h"

#include "tst_config.h"


/***************** 测试配置****************************************************/
#define TST_MSES_MAX_NET    (200)    // 并行构建的 session 个数.

#define TST_MSES_LOCAL_PORT_START      (9510)
#define TST_MSES_LOCAL_KEY       TCONF_APP_KEY

#define TST_MSES_APP_LOCAL_ID_START    (10)
#define TST_MSES_REMOTE_ID       (2)

#define TST_MSES_SERVER_HOST     TCONF_HOST
#define TST_MSES_SERVERT_PORT    TCONF_SERVER_PORT


#define TST_MSES_HOST     TCONF_HOST
#define TST_MSES_SERVER_PORT     TCONF_SERVER_PORT


#define TST_MSES_REMOTE_HOST     TCONF_HOST
#define TST_MSES_REMOTE_PORT     TCONF_DEVICE_PORT
#define TST_MSES_HOST_SECRET_KEY TCONF_SERVER_KEY
#define TST_MSES_REMOTE_SECRET_KEY     TCONF_DEVICE_KEY

#define TST_MSES_DATA_STR  "sending test data."

/********************************************************************/

enum{
    TST_MSES_CMD_NET_CREAT,
    TST_MSES_CMD_SESSION_CREAT,
    TST_MSES_CMD_DATA,
    TST_MSES_CMD_TOKEN,
    TST_MSES_CMD_TOTAL,
    TST_MSES_CMD_DESTORY,
    TST_MSES_CMD_MAX
}TEST_MSES_FUNCTIONS;
static const char *tst_mses_subitem_name[TST_MSES_CMD_MAX] = {
    "network creat",
    "session creat"
    "data transmission",
    "token update",
    "Finaly function",
    "net destory ",
    NULL
};
typedef struct TST_MSES_ITEM_T{
    M2M_T m2m;
    int index;
    int rq_indx;
    int respon_indx;

    int result[TST_MSES_CMD_MAX];
    int final_result;
}TST_Mses_item_T;

extern u8 g_log_level;
TST_Mses_item_T msession[TST_MSES_MAX_NET];

void test_mses_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);

int test_mses_result(int *p_ret,u8 **p_name, int items);
void test_mses_onlineCheck_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);

#define WAIT_UNTIL(n,c,net)   while(n != c){ \
                                 m2m_trysync(net);}
static int g_success = 1;
int mutiple_session_report(TST_Mses_item_T *p_ml){
    int i = 0, finish_items = 0;
    if(!p_ml) return 1;
    
    for( i; i<TST_MSES_MAX_NET; i++ ){
            if( p_ml[i].result[TST_MSES_CMD_DESTORY] )
            finish_items++;
    }
    if( finish_items == TST_MSES_MAX_NET ){
        int result_count = 0;
        m2m_printf("mutiple session test have been finish. \n");
        
        // print test result.
        for( i=0; i< TST_MSES_MAX_NET; i++){
            m2m_printf("session <%d> -----------test items: \n", TST_MSES_APP_LOCAL_ID_START+i);
            p_ml[i].final_result = test_mses_result(p_ml[i].result, (u8**)tst_mses_subitem_name, TST_MSES_CMD_MAX-1 ) + 1;
            result_count += p_ml[i].final_result;
            m2m_printf("session <%d> ----------- final test result: %s \n", TST_MSES_APP_LOCAL_ID_START+i, (p_ml->final_result)?"successful.":"fault." );
        }
        if( result_count == TST_MSES_MAX_NET ){
            m2m_printf("good, mutiple session test total successful..\n");
			g_success = 0;
		}else 
            m2m_printf("sorry, there something wrong in mutiple test.\n");
        return 1;
    }
    else 
        return 0;
}

int mutiple_cmd_jump_rq(size_t p_net,TST_Mses_item_T *p_ml, int index){

    M2M_id_T local_id, remote_id;
    int ret  =0;

    mmemset( (u8*)&local_id, 0, sizeof( M2M_id_T));
    mmemset( (u8*)&remote_id, 0, sizeof( M2M_id_T));
    
    local_id.id[ ID_LEN-1] = TST_MSES_APP_LOCAL_ID_START + index;
    remote_id.id[ ID_LEN-1 ] = TST_MSES_REMOTE_ID;

    if( !p_net || !p_ml || p_ml->rq_indx >= TST_MSES_CMD_MAX  || (p_ml->rq_indx == p_ml->respon_indx && p_ml->rq_indx !=0) )
        return 0;
    
    switch( p_ml->rq_indx ){

        case TST_MSES_CMD_NET_CREAT:
            p_ml->m2m.net = p_net;
            p_ml->index = index;
            p_ml->respon_indx = p_ml->rq_indx;
            p_ml->result[TST_MSES_CMD_NET_CREAT] = 1;
            p_ml->rq_indx++;
            
       case TST_MSES_CMD_SESSION_CREAT:
            p_ml->m2m.session = m2m_session_creat( p_ml->m2m.net, &remote_id, TST_MSES_REMOTE_HOST, TST_MSES_REMOTE_PORT,\
                                                   strlen(TST_MSES_REMOTE_SECRET_KEY), TST_MSES_REMOTE_SECRET_KEY, (m2m_func)test_mses_callback, p_ml);
            if( p_ml->m2m.session){
                p_ml->respon_indx = p_ml->rq_indx;
            }
            break;
        case TST_MSES_CMD_DATA:
            ret = m2m_session_data_send( &p_ml->m2m, strlen(TST_MSES_DATA_STR), TST_MSES_DATA_STR, (m2m_func)test_mses_callback, p_ml );
            p_ml->respon_indx = p_ml->rq_indx;
            break;
        case TST_MSES_CMD_TOKEN:
            ret = m2m_session_token_update( &p_ml->m2m, (m2m_func)test_mses_callback, p_ml);
            p_ml->respon_indx = p_ml->rq_indx;
            break;
        case TST_MSES_CMD_TOTAL:
            ret = m2m_session_data_send( &p_ml->m2m, strlen(TST_MSES_DATA_STR), TST_MSES_DATA_STR, (m2m_func)test_mses_callback, p_ml);
            p_ml->respon_indx = p_ml->rq_indx;
            break;
        case TST_MSES_CMD_DESTORY:
            ret = m2m_session_destory( &p_ml->m2m);
            p_ml->m2m.session = 0;
            p_ml->rq_indx++;
            p_ml->respon_indx = p_ml->rq_indx;
            p_ml->result[TST_MSES_CMD_DESTORY] = 1;
            break;
    }
}
int main(void){
    int i=0, ret=0;
    size_t net = 0;
    M2M_id_T local_id, h_id;

    mmemset( (u8*)&local_id, 0, sizeof( M2M_id_T));
    mmemset( (u8*)&h_id, 0, sizeof( M2M_id_T));
    
    local_id.id[ ID_LEN-1] = TST_MSES_APP_LOCAL_ID_START;
    mmemset( (u8*)&msession, 0, sizeof(TST_Mses_item_T));

    
    m2m_int(NULL);
    net =  m2m_net_creat(&local_id,  TST_MSES_LOCAL_PORT_START, strlen(TST_MSES_LOCAL_KEY), TST_MSES_LOCAL_KEY, \
                         &h_id,(u8*)TST_MSES_SERVER_HOST, TST_MSES_SERVERT_PORT,NULL, NULL);
    while(1){
        // send reqeust .
        for(i=0; i<TST_MSES_MAX_NET;i++ )
            ret = mutiple_cmd_jump_rq( net, &msession[i], i);
        if(mutiple_session_report((TST_Mses_item_T*)&msession))
            break;
        m2m_trysync(net);
    }
    
    ret = m2m_net_destory( net );
    m2m_deint();
    return g_success;

}

void test_mses_onlineCheck_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg){
    m2m_log_debug(">>>>>>> online check callback:: receive code = %d\n", code);
    if( p_recv_pkt && p_recv_pkt->len > 0 && p_recv_pkt->p_data){
        m2m_log("receive data : %s\n",p_recv_pkt->p_data);
        if(p_recv_pkt->len == sizeof(M2M_Address_T)){
            M2M_Address_T *p_ip =  (M2M_Address_T*) p_recv_pkt->p_data;
            m2m_log("device was online and the ip is  %u.%u.%u.%u port: %d", p_ip->ip[0], p_ip->ip[1],p_ip->ip[2],p_ip->ip[3], p_ip->port);
        }
        m2m_bytes_dump((u8*)"recv dump : ",p_recv_pkt->p_data, p_recv_pkt->len);
    }
    if( p_arg ){
        TST_Mses_item_T *p_ml = (TST_Mses_item_T*)p_arg;
        if( code >= M2M_HTTP_OK )
            p_ml->result[p_ml->rq_indx] = 1;

        p_ml->respon_indx = p_ml->rq_indx;
        p_ml->rq_indx++;
    }
} 

void test_mses_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg){
    m2m_log_debug(">>>>>>> net callback:: receive code = %d\n", code);
    if( p_recv_pkt && p_recv_pkt->len > 0 && p_recv_pkt->p_data){
        m2m_log("receive data : %s\n",p_recv_pkt->p_data);
        m2m_bytes_dump((u8*)"recv dump : ",p_recv_pkt->p_data, p_recv_pkt->len);
    }
	
    if(  p_arg && code > 0 ){
        TST_Mses_item_T *p_ml = (TST_Mses_item_T*)p_arg;
        if( code >= M2M_HTTP_OK )
            p_ml->result[p_ml->rq_indx] = 1;
    
        p_ml->respon_indx = p_ml->rq_indx;
        p_ml->rq_indx++;
    }

} 
int test_mses_result(int *p_ret,u8 **p_name, int items){
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
        return -1;
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

