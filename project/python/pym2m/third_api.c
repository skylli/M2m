/***********************************************************************
** devices  *****************************************************
** 作用：构建 server，中转数据，不响应数据。
** 使用: ./app_server <本地 ID> <本地port> <本地 key>.
**
***********************************************************************
**********************************************************************/
#include <stdlib.h>
#include <string.h>
#include "../../../include/m2m_type.h"
#include "../../../include/m2m.h"
#include "../../../include/m2m_api.h"

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
}
void m2m_relay_list_destory(void **pp_list_hd){


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

    return 0;
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
    return 0;
}

int m2m_relay_id_find(M2M_Address_T      *p_addr, void *p,M2M_id_T *p_id){ 
    return 0;
}

