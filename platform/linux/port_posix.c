/*
 * port_posix.c
 * description: socket
 *  Created on: 2018-1-30
 *      Author: skylli
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>
#include <ifaddrs.h>  


#include "m2m_type.h"
#include "m2m.h"
#include "m2m_log.h"
#include "m2m_endian.h"

typedef struct sockaddr SOCKADDR;

/*
 * Function:    m2m_gethostbyname
 * Description: it use the interface in posix.
 * Input:        host: The pointer of host string.
 * Output:      addr: The pointer of the M2M_Address_T.
 * Return:      If success, return 0; else return -1.
*/
int m2m_gethostbyname(M2M_Address_T* addr,char* host)
{

    u32 ip=0;
    struct hostent* hp;
    if((hp=gethostbyname(host))==NULL){
        ip = inet_addr(host);
        
        memcpy(addr->ip, &ip, sizeof(u32));
        addr->len = sizeof(u32);
    }else{
        memcpy(addr->ip, hp->h_addr_list[0], hp->h_length);
        addr->len = hp->h_length;
    }
    
    m2m_debug_level(M2M_LOG,"host: %s ip is %d.%d.%d.%d \n",host,addr->ip[0],addr->ip[1],addr->ip[2],addr->ip[3]);

    return 0;
}
/*
 * Function:    m2m_openSocket
 * Description: m2m_openSocket it use the interface in posix.
 * Input:        N/A
 * Output:      socketId: The pointer of socket id.
 * Return:      If success, return 0; else return -1.
*/
int m2m_openSocket(int* socketId, u16 port)
{
    int fd,ret = 0;
    struct sockaddr_in sin;
    // 创建socket
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket");
        return -1;
    }
    
    if( -1 == bind(fd, (SOCKADDR*)&sin, sizeof(SOCKADDR))){
        m2m_debug_level( M2M_LOG_ERROR, "socket bind to prot %d fail!");
        return M2M_ERR_SOCKETERR;
    }
    *socketId = fd;
    return 0;
}

/*
 * Function:    m2m_closeSocket
 * Description: it use the interface in posix.
 * Input:        socketId: The socket id.
 * Output:      N/A
 * Return:      If success, return 0; else return -1.
*/
int m2m_closeSocket(int socketId)
{
    
    return close(socketId);
}

/*
 * Function:  m2m_send
 * Description: send function, it use the interface in posix.
 * Input:        socketId: The socket id.
 *                  addr_in:  The pointer of M2M_Address_T
 *                  tosend: The pointer of the send buffer
 *                  tosendLength: The length of the send buffer.
 * Output:      N/A
 * Return:      If success, return the sended number. else return -1.
*/
int m2m_send
    (
    int socketId,
    M2M_Address_T* addr_in,
    void* tosend,
    s32 tosendLength
    )
{
    int ret;
    struct sockaddr_in servaddr;    /* server address */
    /* fill in the server's address and data */
    memset((char*)&servaddr, 0, sizeof(servaddr));
    if(addr_in->len== 4){
        servaddr.sin_family = AF_INET;
    }
    else{
        m2m_debug_level( M2M_LOG_WARN, "m2m_send unkown addr len!");
        return -1;
    }
    //servaddr.sin_port = m2m_htons(addr_in->port);
    servaddr.sin_port = ntohs(addr_in->port);
    memcpy(&servaddr.sin_addr.s_addr,addr_in->ip,addr_in->len);

    m2m_debug_level(M2M_LOG_DEBUG, \
                        "socket %d send to addr_in->port = %d, ip = %u.%u.%u.%u", \
                        socketId,addr_in->port, addr_in->ip[0], \
                        addr_in->ip[1], addr_in->ip[2], \
                        addr_in->ip[3]);
    if((ret = sendto( socketId, tosend, tosendLength, 0, 
                      ( struct sockaddr *)&servaddr,
                      sizeof(servaddr)))<0)
    {
        m2m_debug_level(M2M_LOG_ERROR, "sendto failed");
        return -1;
    }
    return ret;
}
    

/*
 * Function:    m2m_receive_
 * Description:  receive function, it use the interface in posix.
 * Input:        socketId: The socket id.
 *                  addr:  The pointer of M2M_Address_T
 *                  buf: The pointer of the send buffer
 *                  bufLen: The length of the send buffer.
 *                  timeout: The max timeout in recv process.
 * Output:      N/A
 * Return:      If success, return the number of bytes received; else return -1.
*/
int m2m_receive
    (
    int socketId,
    M2M_Address_T* p_src_addr,
    void* buf,
    s32 bufLen, 
    s32 timeout
    )
{
    struct sockaddr_in remaddr;
    socklen_t addrlen = sizeof(remaddr);
    int recvlen;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = timeout*1000;
    u8 *p = buf;
    setsockopt(socketId, SOL_SOCKET, SO_RCVTIMEO, \
               (char *)&tv,sizeof(struct timeval));

    recvlen = recvfrom(socketId, buf, bufLen, 0, \
                       (struct sockaddr *)&remaddr, &addrlen);
    if(recvlen < 0 )
    {
        if( recvlen == -1)  
            return M2M_ERR_NOERR;
        perror("recvfrom");
        m2m_debug_level(M2M_LOG_ERROR,"return code %d",recvlen);
        return M2M_ERR_SOCKETERR;
    }
    // todo len better be unknows.
    memcpy(p_src_addr->ip, &remaddr.sin_addr.s_addr, 4);
    p_src_addr->len = 4;
    p_src_addr->port = htons(remaddr.sin_port);
    m2m_debug_level_noend(M2M_LOG_DEBUG, "socket %d received %d bytes from  %d.%d.%d.%d  prot %d\n", socketId,recvlen,\
            p_src_addr->ip[0],p_src_addr->ip[1],p_src_addr->ip[2],p_src_addr->ip[3],p_src_addr->port);
    
    return recvlen;
}

/*
 * Function:    m2m_receive
 * Description:  receive function, it use the interface in posix.
 * Input:        socketId: The socket id.
 *                  addr:  The pointer of M2M_Address_T
 *                  buf: The pointer of the send buffer
 *                  bufLen: The length of the send buffer.
 *                  timeout: The max timeout in recv process.
 * Output:      N/A
 * Return:      If success, return the number of bytes received; else return -1.
*/
int m2m_receive_filt_addr
    (
    int socketId,
    M2M_Address_T* addr,
    void* buf,
    s32 bufLen, 
    s32 timeout
    )
{
    struct sockaddr_in remaddr;
    socklen_t addrlen = sizeof(remaddr);
    int recvlen;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = timeout*1000;

    setsockopt(socketId, SOL_SOCKET, SO_RCVTIMEO, \
               (char *)&tv,sizeof(struct timeval));

    recvlen = recvfrom(socketId, buf, bufLen, 0, \
                       (struct sockaddr *)&remaddr, &addrlen);
    if(recvlen < 0)
    {
        return -1;
    }
    if(memcmp(addr->ip, &remaddr.sin_addr.s_addr, addr->len) || \
        m2m_ntohs(remaddr.sin_port) != addr->port)
    {
        m2m_debug_level(M2M_LOG_WARN,"ip or port not match!");
        return -1;
    }
    else
    {
        m2m_debug_level(M2M_LOG_WARN, "received %d bytes",recvlen);
    }
    return recvlen;
}

// 获取系统时间
u32 m2m_current_time_get(void)
{
    struct timeval tv;  
    gettimeofday(&tv,NULL);
    return ( tv.tv_sec*1000 + tv.tv_usec/1000 );
}
u32 m2m_get_random(){
    srand(time(NULL));
    return (u32)rand();
}
int broadcast_enable(int socket_fd){
    int opt= 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
}
int get_bcast_list(u32 *list, int maxlen)
{
    int num;
    struct ifaddrs *ifa, *oifa;
    
    if(getifaddrs(&ifa) < 0)
    {
        m2m_debug_level(M2M_LOG_ERROR,"get_bcast_list err");
        return 0;
    }
    // enable breacast
        // 使能广播
    oifa = ifa;
    num = 0;
    while(ifa)
    {
        struct sockaddr_in* saddr = (struct sockaddr_in*)ifa->ifa_addr;
        struct sockaddr_in* smask = (struct sockaddr_in*)ifa->ifa_netmask;
        
        if(saddr && smask)
        {
            u32 ip = saddr->sin_addr.s_addr;
            u32 mask = smask->sin_addr.s_addr;
            
            if(ip != 0 && ip != 0x100007f)
            {
                list[num] = ip | (~mask);
                //debug_log("bcast %x\n", list[num]);
                num++;
            }
        }
        
        ifa = ifa->ifa_next;
    }
    
    freeifaddrs(oifa);
    return num;
}


