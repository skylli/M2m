/***********************************************************************
**多 network 测试********************************************************
** 描述：该测试通过同时建立 TST_MSES_MAX_NET 个 network，每个 network 单独跟 
** TST_MNET_REMOTE_HOST/TST_MNET_REMOTE_PORT 远端建立 session，分别进行token 
** 获取，传输数据， 更新token，再度传输数据验证 token 更新的是否成功测试。
** 需求： 运行远端 device， 其 ip/port 必须跟该测试中的  
**        TST_MNET_REMOTE_HOST/TST_MNET_REMOTE_POR 对应。否则无法进行测试。
** 测试结果：
**  每个 session 的单项测试接收到 device 回应的数据包为成功，否则为失败，失败的可能有，传输超时，数据包本身有问题，session 未成功建立 ...。
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

/***************** 测试配置****************************************************/
#define TST_MSES_MAX_NET    (10)    // 并行构建的 net 个数.

#define TST_MNET_LOCAL_PORT_START      (9510)
#define TST_MNET_LOCAL_KEY       DEFAULT_APP_KEY

#define TST_MNET_APP_LOCAL_ID_START    (10)
#define TST_MNET_REMOTE_ID       (2)

#define TST_MNET_SERVER_HOST     DEFAULT_HOST
#define TST_MNET_SERVERT_PORT    DEFAULT_SERVER_PORT


#define TST_MNET_HOST     DEFAULT_HOST
#define TST_MNET_SERVER_PORT     DEFAULT_SERVER_PORT


#define TST_MNET_REMOTE_HOST     DEFAULT_HOST
#define TST_MNET_REMOTE_PORT     DEFAULT_DEVICE_PORT
#define TST_MNET_HOST_SECRET_KEY DEFAULT_SERVER_KEY
#define TST_MNET_REMOTE_SECRET_KEY     DEFAULT_DEVICE_KEY

#define TST_MNET_DATA_STR  "sending test data."

/********************************************************************/

enum{
    TST_MNET_CMD_START = 0,
    TST_MNET_CMD_NET_CREAT,
    TST_MNET_CMD_LNLINE_CHECK,
    TST_MNET_CMD_SESSION_CREAT,
    TST_MNET_CMD_DATA,
    TST_MNET_CMD_TOKEN,
    TST_MNET_CMD_TOTAL,
    TST_MNET_CMD_DESTORY,
    TST_MNET_CMD_MAX
}TEST_MNET_FUNCTIONS;
static const char *tst_mnet_subitem_name[TST_MNET_CMD_MAX] = {
    "start to test",
    "network creat",
    "online check",
    "session creat"
    "data transmission",
    "token update",
    "Finaly function",
    "net destory ",
    NULL
};
typedef struct TST_MNET_ITEM_T{
    M2M_T m2m;

    int index;
    int rq_indx;
    int respon_indx;

    int result[TST_MNET_CMD_MAX];
    int final_result;
}TST_Mnet_item_T;

extern u8 g_log_level;
TST_Mnet_item_T mnet[TST_MSES_MAX_NET];

void test_mnet_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);

int test_mnet_result(int *p_ret,u8 **p_name, int items);
void test_mnet_onlineCheck_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg);

#define WAIT_UNTIL(n,c,net)   while(n != c){ \
                                 m2m_trysync(net);}
int mutiple_trysync_continue(TST_Mnet_item_T *p_ml){
    int i = 0, finish_items = 0;
    if(!p_ml) return 0;
    
    for( i; i<TST_MSES_MAX_NET; i++ ){
        if(p_ml[i].m2m.net)
            m2m_trysync( p_ml[i].m2m.net);
        else
            finish_items++;
    }
    if( finish_items == TST_MSES_MAX_NET ){
        int result_count = 0;
        m2m_printf("mutiple net test have been finish. \n");
        
        // print test result.
        for(i=0;i< TST_MSES_MAX_NET; i++){
            m2m_printf("net<%d> -----------test items: \n", TST_MNET_APP_LOCAL_ID_START+i);
            p_ml[i].final_result = test_mnet_result(p_ml[i].result, (u8**)tst_mnet_subitem_name, TST_MNET_CMD_MAX-1 ) + 1;
            result_count += p_ml[i].final_result;
            m2m_printf("net<%d> ----------- final test result: %d \n", TST_MNET_APP_LOCAL_ID_START+i, p_ml->final_result);
        }
        if( result_count == TST_MSES_MAX_NET )
            m2m_printf("good, mutiple test total successful..\n");
        else 
            m2m_printf("sorry, there something wrong in mutiple test.\n");
        return 0;
    }
    else 
        return 1;
}

int mutiple_cmd_jump_rq( TST_Mnet_item_T *p_ml, int index){

    M2M_id_T local_id, remote_id, h_id;
    int ret  =0;

    mmemset( (u8*)&local_id, 0, sizeof( M2M_id_T));
    mmemset( (u8*)&remote_id, 0, sizeof( M2M_id_T));
    mmemset( (u8*)&h_id, 0, sizeof( M2M_id_T));
    
    local_id.id[ID_LEN-1] = TST_MNET_APP_LOCAL_ID_START + index;
    remote_id.id[ ID_LEN-1 ] = TST_MNET_REMOTE_ID;
    if( !p_ml || p_ml->rq_indx >= TST_MNET_CMD_MAX  || (p_ml->rq_indx == p_ml->respon_indx && p_ml->rq_indx !=0) )
        return 0;

    
    switch( p_ml->rq_indx ){

        case TST_MNET_CMD_START:
            p_ml->index = index;
            p_ml->respon_indx = p_ml->rq_indx;
            p_ml->result[TST_MNET_CMD_START] = 1;
            p_ml->rq_indx++;
            //break;
            
        case TST_MNET_CMD_NET_CREAT:
            p_ml->m2m.net = m2m_net_creat( &local_id, (TST_MNET_LOCAL_PORT_START + index), strlen(TST_MNET_LOCAL_KEY), TST_MNET_LOCAL_KEY, \
                                           &h_id,TST_MNET_SERVER_HOST, TST_MNET_SERVERT_PORT,NULL, NULL);
            if( p_ml->m2m.net ){
                
                p_ml->result[TST_MNET_CMD_NET_CREAT] = 1;
                p_ml->respon_indx = p_ml->rq_indx;
                p_ml->rq_indx++;
            }
            break;
        case TST_MNET_CMD_LNLINE_CHECK:
            
            //p_ml->m2m.session = m2m_dev_online_check( p_ml->m2m.net,TST_MNET_REMOTE_HOST, TST_MNET_SERVER_PORT, &remote_id,test_mnet_onlineCheck_callback, p_ml);
            p_ml->respon_indx = p_ml->rq_indx;
            // todo 
            
            p_ml->result[TST_MNET_CMD_LNLINE_CHECK] = 1;
            p_ml->rq_indx++;
            p_ml->respon_indx = p_ml->rq_indx;
            //break;
       case TST_MNET_CMD_SESSION_CREAT:
            p_ml->m2m.session = m2m_session_creat( p_ml->m2m.net, &remote_id, TST_MNET_REMOTE_HOST, TST_MNET_REMOTE_PORT,\
                                                   strlen(TST_MNET_REMOTE_SECRET_KEY), TST_MNET_REMOTE_SECRET_KEY,(m2m_func)test_mnet_callback, p_ml);
            if( p_ml->m2m.session){
                p_ml->respon_indx = p_ml->rq_indx;
            }
            break;
        case TST_MNET_CMD_DATA:
            ret = m2m_session_data_send( &p_ml->m2m, strlen(TST_MNET_DATA_STR), TST_MNET_DATA_STR,(m2m_func)test_mnet_callback, p_ml );
            p_ml->respon_indx = p_ml->rq_indx;
            break;
        case TST_MNET_CMD_TOKEN:
            ret = m2m_session_token_update( &p_ml->m2m, (m2m_func)test_mnet_callback, p_ml);
            p_ml->respon_indx = p_ml->rq_indx;
            break;
        case TST_MNET_CMD_TOTAL:
            ret = m2m_session_data_send( &p_ml->m2m, strlen(TST_MNET_DATA_STR), TST_MNET_DATA_STR,(m2m_func)test_mnet_callback, p_ml);
            p_ml->respon_indx = p_ml->rq_indx;
            break;
        case TST_MNET_CMD_DESTORY:
            ret = m2m_net_destory( p_ml->m2m.net);
            p_ml->m2m.session = 0;
            p_ml->m2m.net = 0;
            p_ml->rq_indx++;
            p_ml->respon_indx = p_ml->rq_indx;
            p_ml->result[TST_MNET_CMD_DESTORY] = 1;
            break;
    }
}
int main(void){
    int i=0, ret=0;
    mmemset( (u8*)&mnet, 0, sizeof(TST_Mnet_item_T));

    
    m2m_int(NULL);
    while(1){
        // send reqeust .
        for(i=0; i<TST_MSES_MAX_NET;i++ )
            ret = mutiple_cmd_jump_rq(&mnet[i], i);
        if(!mutiple_trysync_continue((TST_Mnet_item_T*)&mnet))
            break;
    }
    m2m_deint();
    return 0;

}

void test_mnet_onlineCheck_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg){
    m2m_log_debug(">>>>>>> online check callback:: receive code = %d\n", code);
    if( p_recv_pkt && p_recv_pkt->len > 0 && p_recv_pkt->p_data){
        m2m_log("receive data : %s\n",p_recv_pkt->p_data);
        if(p_recv_pkt->len == sizeof(M2M_Address_T)){
            M2M_Address_T *p_ip =  (M2M_Address_T*) p_recv_pkt->p_data;
            m2m_log("device was online and the ip is  %u.%u.%u.%u port: %d", p_ip->ip[0], p_ip->ip[1],p_ip->ip[2],p_ip->ip[3], p_ip->port);
        }
        m2m_bytes_dump("recv dump : ",p_recv_pkt->p_data, p_recv_pkt->len);
    }
    if( p_arg ){
        TST_Mnet_item_T *p_ml = (TST_Mnet_item_T*)p_arg;
        if( code >= M2M_HTTP_OK )
            p_ml->result[p_ml->rq_indx] = 1;

        p_ml->respon_indx = p_ml->rq_indx;
        p_ml->rq_indx++;
    }
} 

void test_mnet_callback(int code,M2M_packet_T **pp_ack_pkt, M2M_packet_T *p_recv_pkt,void *p_arg){
    m2m_log_debug(">>>>>>> net callback:: receive code = %d\n", code);
    if( p_recv_pkt && p_recv_pkt->len > 0 && p_recv_pkt->p_data){
        m2m_log("receive data : %s\n",p_recv_pkt->p_data);
        m2m_bytes_dump("recv dump : ",p_recv_pkt->p_data, p_recv_pkt->len);
    }
    if(  p_arg){
        TST_Mnet_item_T *p_ml = (TST_Mnet_item_T*)p_arg;
        if( code >= M2M_HTTP_OK )
            p_ml->result[p_ml->rq_indx] = 1;
    
        p_ml->respon_indx = p_ml->rq_indx;
        p_ml->rq_indx++;
    }

} 
int test_mnet_result(int *p_ret,u8 **p_name, int items){
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

