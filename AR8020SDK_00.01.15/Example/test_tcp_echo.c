///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////

#include "lwip/opt.h"

#if LWIP_SOCKET // don't build if not configured for use in lwipopts.h
// Standard C Included Files
#include <stdio.h>
// lwip Included Files
#include "lwip/mem.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/dhcp.h"
#include "lwip/pbuf.h"
#include "lwip/netbuf.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/inet_chksum.h"
#include "lwip/init.h"
#include "netif/etharp.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
//#include "etharp.h"
// SDK Included Files
#include "hal_usb_host.h"
#include "hal.h"

#include "test_tcp_echo.h"

///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////

// UDPECHO_DEBUG: Enable debugging for UDPECHO.
#ifndef TCPECHO_DBG
#define TCPECHO_DBG                 LWIP_DBG_ON
#endif

#ifndef TCPECHO_STACKSIZE
#define TCPECHO_STACKSIZE           3000
#endif

#ifndef TCPECHO_PRIORITY
#define TCPECHO_PRIORITY            osPriorityNormal
#endif

///////////////////////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////////////////////

static struct netif netif0;

///////////////////////////////////////////////////////////////////////////////
// Code
///////////////////////////////////////////////////////////////////////////////

typedef struct ip_addr ip_addr_t;

#define ip4_addr_get_u32(src_ipaddr) ((src_ipaddr)->addr)
#define inet_addr_from_ipaddr(target_inaddr, source_ipaddr) ((target_inaddr)->s_addr = ip4_addr_get_u32(source_ipaddr))

#define ip4_addr_set_u32(dest_ipaddr, src_u32) ((dest_ipaddr)->addr = (src_u32))
#define inet_addr_to_ipaddr(target_ipaddr, source_inaddr)   (ip4_addr_set_u32(target_ipaddr, (source_inaddr)->s_addr))

static void tcpecho_thread(void *arg)
{
    struct netconn *conn, *newconn;
    err_t err;
    uint32_t * pint = NULL;
    
    LWIP_UNUSED_ARG(arg);
    netif_set_up(&netif0);
    // Create a new connection identifier
    conn = netconn_new(NETCONN_TCP);

    // Bind connection to well known port number 7
    err = netconn_bind(conn, NULL, 7);
    if(err != ERR_OK)
    {
        DLOG_Error("bind failed !\n"); 
    }

    // Tell connection to go into listening mode
    netconn_listen(conn);

    while (1) 
    {
        /* Grab new connection. */
        err = netconn_accept(conn, &newconn);
        /* Process the new connection. */
        if (err == ERR_OK) 
        {
            struct netbuf *buf;
            void *data;
            u16_t len;

            while ((err = netconn_recv(newconn, &buf)) == ERR_OK) 
            {
                pint = (uint32_t *)(buf->p->payload);
                LWIP_DEBUGF(TCPECHO_DBG, ("len: %u got: %u", buf->p->tot_len, ntohl(*pint)));
                
                do 
                {
                    netbuf_data(buf, &data, &len);
                    err = netconn_write(newconn, data, len, NETCONN_COPY);
                    if(err != ERR_OK) 
                    {
                        DLOG_Error("netconn_send failed: %d\r\n", (int)err);
                    } 
                    else 
                    {                    
                        LWIP_DEBUGF(TCPECHO_DBG, ("netconn_send success\r\n"));
                    }
                } while (netbuf_next(buf) >= 0);
                netbuf_delete(buf);
            }
            /* Close connection and discard connection identifier. */
            netconn_close(newconn);
            netconn_delete(newconn);
        }
    }
}

static void tcp_echo_init(void)
{
    LWIP_DEBUGF(TCPECHO_DBG, ("TCP thread building !\n"));
    sys_thread_new("tcpecho_thread", tcpecho_thread, NULL, TCPECHO_STACKSIZE, TCPECHO_PRIORITY);
}

extern err_t ethernetif_init(struct netif *netif);
extern void HAL_USB_NetDeviceRecv(void (*net_device_recv_handler)(void *, uint32_t));
extern void ethernetif_input(void *data, uint32_t size);

void command_TestTcpEcho( void )
{
    
    ip_addr_t netif0_ipaddr, netif0_netmask, netif0_gw;

    HAL_USB_NetDeviceUp(ENUM_USB_NETCARD_NORNAL_MODE);
    HAL_USB_NetDeviceRecv(ethernetif_input);

    tcpip_init(NULL,NULL);

    IP4_ADDR(&netif0_ipaddr, 192,168,1,110);
    IP4_ADDR(&netif0_netmask, 255,255,255,0);
    IP4_ADDR(&netif0_gw, 192,168,1,1);

    netif_add(&netif0, &netif0_ipaddr, &netif0_netmask, &netif0_gw, NULL, ethernetif_init, tcpip_input);
    netif_set_default(&netif0);
    netif_set_up(&netif0);

    tcp_echo_init();
    
}

#endif // LWIP_SOCKET



