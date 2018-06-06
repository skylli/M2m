/***********************************************************************
** client  *****************************************************
** 作用：构建 network 跟远端建立 session 连接，发送数据.
** 使用: ./app_client <本地 ID>  <本地port> <本地 key> <服务器 host> <服务器 port> <远端 ID> <dev端 host> <dev端 port> <dev 端key> <传输的 data>
**          随后通过键盘输入 data，按下 enter 发送。
**
***********************************************************************
**********************************************************************/

#include <string.h>
#include "m2m_type.h"
#include "m2m.h"
#include "m2m_api.h"
#include "m2m_log.h"
#include "config.h"
#include "app_implement.h"

/***************** 配置****************************************************/

/********************************************************************/
int main(int argc, char **argv){
   int i, ret;
   for(i=0;i<argc;i++)
       m2m_printf("{%s}",argv[i]);

   return 0;
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

