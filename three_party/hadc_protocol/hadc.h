/*
 * m2m projuct
 *
 * FileName: hadc.h
 *
 * Description: hadc api declaration.
 *
 * Author: skylli   20180615
 * 
 */
#ifndef _HADC_H_

#define _HADC_H_

#include "../../include/m2m_type.h"
/** config ****/
/* version  0x01 | wifi-mode **/
#define HADC_VERSION  0x01

/**
 * version(u8) + type(u8) + len(u16) + payload
*/
typedef struct HADC_HAD_T
{
    u8 version;
    u8 type;
    u16 len;
    u8 p_paylaod[0];
}Hadc_had_T;
/**
 * type define 
 * **/
/**
* 
type | description
-----|---
0x01 | uart
0x02 | gpio
0x03 | input
**/
typedef enum HADC_TYPE_T{
    HADC_TYPE_NONE = 0X00,
    HADC_TYPE_UART = 0X01,
    HADC_TYPE_GPIO = 0X02,
    HADC_TYPE_MAX
}Hadc_type_T;
/** api *********/
int hadc_package_encode_alloc(u8 **pp_hadc, 
        Hadc_type_T cmd, u16 len, u8 *p_data);
int hadc_package_decode(u8 **pp_dst, u8 *p_cmd, u16 slen, u8 *p_src);
#endif // _HADC_H_
