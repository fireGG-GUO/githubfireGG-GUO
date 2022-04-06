/*
 * Copyright (c) 2020 Nanjing Xiaoxiongpai Intelligent Technology Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lwip/netif.h"
#include "lwip/inet.h"
#include "lwip/netifapi.h"
#include "lwip/ip4_addr.h"
#include "lwip/api_shell.h"
#include "lwip/dhcp.h"

#include "cmsis_os2.h"
#include "hos_types.h"
#include "wifi_device.h"
#include "wifiiot_errno.h"
#include "ohos_init.h"
#include "demo.h"

#define DEF_TIMEOUT 15
#define ONE_SECOND 1
//#define DHCP_EN

#define SELECT_WIFI_SECURITYTYPE WIFI_SEC_TYPE_PSK

static void WiFiInit(void);
static void WaitSacnResult(void);
static int WaitConnectResult(void);
static void OnWifiScanStateChangedHandler(int state, int size);
static void OnWifiConnectionChangedHandler(int state, WifiLinkedInfo *info);
static void OnHotspotStaJoinHandler(StationInfo *info);
static void OnHotspotStateChangedHandler(int state);
static void OnHotspotStaLeaveHandler(StationInfo *info);

static int g_staScanSuccess = 0;
static int g_ConnectSuccess = 0;
static int ssid_count = 0;
WifiEvent g_wifiEventHandler = {0};
WifiErrorCode error;

#define SELECT_WLAN_PORT "wlan0"

/* clear netif's ip, gateway and netmask */
void hi_sta_reset_addr(struct netif *pst_lwip_netif)
{
    ip4_addr_t st_gw;
    ip4_addr_t st_ipaddr;
    ip4_addr_t st_netmask;
    //printf("%s %d \r\n", __FILE__, __LINE__);
    if (pst_lwip_netif == NULL)
    {
        printf("hisi_reset_addr::Null param of netdev\r\n");
        return;
    }

    IP4_ADDR(&st_gw, 0, 0, 0, 0);
    IP4_ADDR(&st_ipaddr, 0, 0, 0, 0);
    IP4_ADDR(&st_netmask, 0, 0, 0, 0);

    netifapi_netif_set_addr(pst_lwip_netif, &st_ipaddr, &st_netmask, &st_gw);
}

void hi_sta_set_addr(struct netif *pst_lwip_netif)
{
    ip4_addr_t st_gw;
    ip4_addr_t st_ipaddr;
    ip4_addr_t st_netmask;
    //printf("%s %d \r\n", __FILE__, __LINE__);
    if (pst_lwip_netif == NULL)
    {
        printf("hisi_reset_addr::Null param of netdev\r\n");
        return;
    }

    IP4_ADDR(&st_gw, 192, 168, 1, 1);
    IP4_ADDR(&st_ipaddr, 192, 168, 1, 200);
    IP4_ADDR(&st_netmask, 255, 255, 255, 0);

    netifapi_netif_set_addr(pst_lwip_netif, &st_ipaddr, &st_netmask, &st_gw);
    printf("**********localIP:%s******************\r\n", ip4addr_ntoa(&st_ipaddr));
    printf("**********netmask:%s******************\r\n", ip4addr_ntoa(&st_netmask));
    printf("**********gw:%s******************\r\n", ip4addr_ntoa(&st_gw));
}

int WifiConnect(const char *ssid, const char *psk)
{
    WifiScanInfo *info = NULL;
    unsigned int size = WIFI_SCAN_HOTSPOT_LIMIT;
    static struct netif *g_lwip_netif = NULL;

    osDelay(200);
    printf("<--System Init-->\r\n");

    //初始化WIFI
    WiFiInit();

    //使能WIFI
    if (EnableWifi() != WIFI_SUCCESS)
    {
        printf("EnableWifi failed, error = %d\r\n", error);
        return -1;
    }

    //判断WIFI是否激活
    if (IsWifiActive() == 0)
    {
        printf("Wifi station is not actived.\r\n");
        return -1;
    }

    //分配空间，保存WiFi信息
    info = malloc(sizeof(WifiScanInfo) * WIFI_SCAN_HOTSPOT_LIMIT);
    if (info == NULL)
    {
        return -1;
    }
    //轮询查找WiFi列表
    do
    {
        //重置标志位
        ssid_count = 0;
        g_staScanSuccess = 0;

        //开始扫描
        Scan();

        //等待扫描结果
        WaitSacnResult();

        //获取扫描列表
        error = GetScanInfoList(info, &size);

    } while (g_staScanSuccess != 1);
    //打印WiFi列表
    printf("********************\r\n");
    for (uint8_t i = 0; i < ssid_count; i++)
    {
        printf("no:%03d, ssid:%-30s, rssi:%5d\r\n", i + 1, info[i].ssid, info[i].rssi / 100);
    }
    printf("********************\r\n");

    //连接指定的WiFi热点
    for (uint8_t i = 0; i < ssid_count; i++)
    {
        if (strcmp(ssid, info[i].ssid) == 0)
        {
            int result;

            printf("Select:%3d wireless, Waiting...\r\n", i + 1);

            //拷贝要连接的热点信息
            WifiDeviceConfig select_ap_config = {0};
            strcpy(select_ap_config.ssid, info[i].ssid);
            strcpy(select_ap_config.preSharedKey, psk);
            select_ap_config.securityType = SELECT_WIFI_SECURITYTYPE;

            if (AddDeviceConfig(&select_ap_config, &result) == WIFI_SUCCESS)
            {
                if (ConnectTo(result) == WIFI_SUCCESS && WaitConnectResult() == 1)
                {
                    printf("WiFi connect succeed!\r\n");
                    g_lwip_netif = netifapi_netif_find(SELECT_WLAN_PORT);
                    break;
                }
            }
        }

        if (i == ssid_count - 1)
        {
            printf("ERROR: No wifi as expected\r\n");
            while (1)
                osDelay(100);
        }
    }
#ifdef DHCP_EN
    //启动DHCP
    if (g_lwip_netif)
    {
        netifapi_dhcp_start(g_lwip_netif);
        printf("begain to dhcp\r\n");
    }

    //等待DHCP
    for (;;)
    {
        if (netifapi_dhcp_is_bound(g_lwip_netif) == ERR_OK)
        {
            //char addrString[20] = {0};
            printf("<-- DHCP state:OK -->\r\n");

            //打印获取到的IP信息
            netifapi_netif_common(g_lwip_netif, dhcp_clients_info_show, NULL);

            printf("**********localIP:%s******************\r\n", ip4addr_ntoa(&g_lwip_netif->ip_addr.u_addr.ip4));
            printf("**********netmask:%s******************\r\n", ip4addr_ntoa(&g_lwip_netif->netmask.u_addr.ip4));
            printf("**********gw:%s******************\r\n", ip4addr_ntoa(&g_lwip_netif->gw.u_addr.ip4));

            // memset(addrString, 0, sizeof(addrString));
            // inet_ntoa_r(g_lwip_netif->ip_addr, addrString, 20);
            // printf("ipaddr %s\n", addrString);
            // memset(addrString, 0, sizeof(addrString));
            // inet_ntoa_r(g_lwip_netif->netmask, addrString, 20);
            // printf("netmask %s\n", addrString);
            // memset(addrString, 0, sizeof(addrString));
            // inet_ntoa_r(g_lwip_netif->gw, addrString, 20);
            // printf("gw %s\n", addrString);

            break;
        }
        printf("<-- DHCP state:Inprogress -->\r\n");
        osDelay(100);
    }
#else
    //配置静态IP
    if (g_lwip_netif)
    {
        hi_sta_set_addr(g_lwip_netif);
        osEventFlagsSet(wifi_connect_evt_id, FLAGS_MSK1);
    }
#endif

    osDelay(100);

    return 0;
}

static void WiFiInit(void)
{
    printf("<--Wifi Init-->\r\n");
    g_wifiEventHandler.OnWifiScanStateChanged = OnWifiScanStateChangedHandler;
    g_wifiEventHandler.OnWifiConnectionChanged = OnWifiConnectionChangedHandler;
    g_wifiEventHandler.OnHotspotStaJoin = OnHotspotStaJoinHandler;
    g_wifiEventHandler.OnHotspotStaLeave = OnHotspotStaLeaveHandler;
    g_wifiEventHandler.OnHotspotStateChanged = OnHotspotStateChangedHandler;
    error = RegisterWifiEvent(&g_wifiEventHandler);
    if (error != WIFI_SUCCESS)
    {
        printf("register wifi event fail!\r\n");
    }
    else
    {
        printf("register wifi event succeed!\r\n");
    }
}

static void OnWifiScanStateChangedHandler(int state, int size)
{
    if (size > 0)
    {
        ssid_count = size;
        g_staScanSuccess = 1;
    }
    printf("callback function for wifi scan:%d, %d\r\n", state, size);
    return;
}

static void OnWifiConnectionChangedHandler(int state, WifiLinkedInfo *info)
{
    if (info == NULL)
    {
        printf("WifiConnectionChanged:info is null, stat is %d.\n", state);
    }
    else
    {
        if (state == WIFI_STATE_AVALIABLE)
        {
            g_ConnectSuccess = 1;
        }
        else
        {
            g_ConnectSuccess = 0;
        }
    }
}

static void OnHotspotStaJoinHandler(StationInfo *info)
{
    (void)info;
    printf("STA join AP\n");
    return;
}

static void OnHotspotStaLeaveHandler(StationInfo *info)
{
    (void)info;
    printf("HotspotStaLeave:info is null.\n");
    return;
}

static void OnHotspotStateChangedHandler(int state)
{
    printf("HotspotStateChanged:state is %d.\n", state);
    return;
}

static void WaitSacnResult(void)
{
    int scanTimeout = DEF_TIMEOUT;
    while (scanTimeout > 0)
    {
        sleep(ONE_SECOND);
        scanTimeout--;
        if (g_staScanSuccess == 1)
        {
            printf("WaitSacnResult:wait success[%d]s\n", (DEF_TIMEOUT - scanTimeout));
            break;
        }
    }
    if (scanTimeout <= 0)
    {
        printf("WaitSacnResult:timeout!\n");
    }
}

static int WaitConnectResult(void)
{
    int ConnectTimeout = DEF_TIMEOUT;
    while (ConnectTimeout > 0)
    {
        sleep(ONE_SECOND);
        ConnectTimeout--;
        if (g_ConnectSuccess == 1)
        {
            printf("WaitConnectResult:wait success[%d]s\n", (DEF_TIMEOUT - ConnectTimeout));
            break;
        }
    }
    if (ConnectTimeout <= 0)
    {
        printf("WaitConnectResult:timeout!\n");
        return 0;
    }

    return 1;
}
