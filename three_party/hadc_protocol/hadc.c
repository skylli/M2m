#include "../../include/m2m_type.h"
#include "../../include/util.h"
#include "hadc.h"

int hadc_package_encode_alloc(u8 **pp_hadc, 
        Hadc_type_T cmd, u16 len, u8 *p_data){

    if( !pp_hadc || !len || !p_data)
        return -1;
    
    Hadc_had_T *p_hadc = (Hadc_had_T*)mmalloc(sizeof(Hadc_had_T) + len +1);
    p_hadc->version = HADC_VERSION;
    p_hadc->type = cmd;
    p_hadc->len = len;
    mcpy(p_hadc->p_paylaod, p_data, len);

    *pp_hadc = (u8*)p_hadc;
    return (int) (sizeof(Hadc_had_T) + len);
}
int hadc_package_decode(u8 **pp_dst, u8 *p_cmd, u16 slen, u8 *p_src){
    Hadc_had_T *p = NULL;

    if(slen < sizeof(Hadc_had_T) || !p_src || !pp_dst ||!p_cmd)
        return 0;
    
    p = (Hadc_had_T*) p_src;
    if( p->version != HADC_VERSION)
        return 0;

    *p_cmd = p->type;
    *pp_dst = p->p_paylaod;
    
    return (int)p->len;
}

