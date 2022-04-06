/*********************************************************************
 * 头文件 INCLUDES
 */
#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "wifiiot_errno.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_i2c.h"
#include "wifiiot_i2c_ex.h"

#include "lwip/sockets.h"
#include "demo.h"
#include "wifi_connect.h"
#include "nfc.h"
#include "i2c_nfc.h"



/********************************************************************* 
 * 宏定义  *#define 
 */
#define WIFI_SSID "GG"
#define WIFI_PSK "15105081376"

#define _PROT_ 80     //webserver基于TCPserver建立的用的端口号默认是80
#define TCP_BACKLOG 1 //TCP客户端最大连接数量

/*******************************************************************************
 * EXTERNAL VARIABLES
 */
//在sock_fd 进行监听，在 new_fd 接收新的链接
int sock_fd, new_fd;
char recvbuf[512];
unsigned int LED_Per = 0;
unsigned int LED_BLN_flag = 0;

//web HTML网站信息
unsigned char LED_htmldataIP[] = "119.75.217.109"//"192.168.1.200";

const unsigned char LED_htmldata1[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n<center><p>LED is now [";
unsigned char LED_htmldata2[] = "BLN OFF";
const unsigned char LED_htmldata3[] = "]! Pre[";
unsigned char LED_htmldata4[] = "100";
const unsigned char LED_htmldata5[] = "]!</center><br><br>Click <a href='http://";
const unsigned char LED_htmldata6[] = "/gpio/000'>here</a> to switch LED GPIO OFF Per:000, or <a href='http://";
const unsigned char LED_htmldata7[] = "/gpio/100'>here</a> to switch LED GPIO ON Per:100,or <a href='http://";
const unsigned char LED_htmldata8[] = "/gpio/F'>here</a> to switch LED VAL,or <a href='http://";
const unsigned char LED_htmldata9[] = "/gpio/B'>here</a> to LED breathing light MODE</html>";

const unsigned char htmldata[] = "\
      <html>\
          <head><title> LWIP</title></head>\
           <center><p>A WebServer Based on LwIP Hello world!</center>\
     </html>";

const unsigned char errhtml[] = "\
        <html>\
            <head>\
               <title>Error!</title>\
            </head>\
            <body>\
               <h1>404 - Page not found</h1>\
            </body>\
        </html>";

osEventFlagsId_t wifi_connect_evt_id; // event flags id
/*******************************************************************************/
/**
  * @brief 呼吸灯模式下打开网页内容
  * @param conn: connection socket
  * @retval None
  */
void BLN_htmldata_join_write(int conn)
{
  lwip_write(conn, LED_htmldata1, sizeof(LED_htmldata1) - 1);
  memset(LED_htmldata2, 0, sizeof(LED_htmldata2));
  memcpy(LED_htmldata2, ((LED_BLN_flag != 0) ? "BLN  ON" : "BLN OFF"), 7);
  lwip_write(conn, LED_htmldata2, sizeof(LED_htmldata2) - 1);
  lwip_write(conn, LED_htmldata5, sizeof(LED_htmldata5) - 1);
  lwip_write(conn, LED_htmldataIP, sizeof(LED_htmldataIP) - 1);
  lwip_write(conn, LED_htmldata6, sizeof(LED_htmldata6) - 1);
  lwip_write(conn, LED_htmldataIP, sizeof(LED_htmldataIP) - 1);
  lwip_write(conn, LED_htmldata7, sizeof(LED_htmldata7) - 1);
  lwip_write(conn, LED_htmldataIP, sizeof(LED_htmldataIP) - 1);
  lwip_write(conn, LED_htmldata8, sizeof(LED_htmldata8) - 1);
  lwip_write(conn, LED_htmldataIP, sizeof(LED_htmldataIP) - 1);
  lwip_write(conn, LED_htmldata9, sizeof(LED_htmldata9) - 1);
}
/**
  * @brief LED控制模式下打开网页内容
  * @param conn: connection socket
  * @retval None
  */
void LED_htmldata_join_write(int conn)
{
  char send_Per[3];
  lwip_write(conn, LED_htmldata1, sizeof(LED_htmldata1) - 1);
  memset(LED_htmldata2, 0, sizeof(LED_htmldata2));
  memcpy(LED_htmldata2, ((LED_Per != 0) ? " ON" : "OFF"), 3);
  lwip_write(conn, LED_htmldata2, sizeof(LED_htmldata2) - 1);
  lwip_write(conn, LED_htmldata3, sizeof(LED_htmldata3) - 1);
  memset(LED_htmldata4, 0, sizeof(LED_htmldata4));
  sprintf(send_Per, "%03d", LED_Per);
  memcpy(LED_htmldata4, send_Per, 3);
  lwip_write(conn, LED_htmldata4, sizeof(LED_htmldata4) - 1);
  lwip_write(conn, LED_htmldata5, sizeof(LED_htmldata5) - 1);
  lwip_write(conn, LED_htmldataIP, sizeof(LED_htmldataIP) - 1);
  lwip_write(conn, LED_htmldata6, sizeof(LED_htmldata6) - 1);
  lwip_write(conn, LED_htmldataIP, sizeof(LED_htmldataIP) - 1);
  lwip_write(conn, LED_htmldata7, sizeof(LED_htmldata7) - 1);
  lwip_write(conn, LED_htmldataIP, sizeof(LED_htmldataIP) - 1);
  lwip_write(conn, LED_htmldata8, sizeof(LED_htmldata8) - 1);
  lwip_write(conn, LED_htmldataIP, sizeof(LED_htmldataIP) - 1);
  lwip_write(conn, LED_htmldata9, sizeof(LED_htmldata9) - 1);
}

/**
  * @brief serve tcp connection
  * @param conn: connection socket
  * @retval None
  */
void http_server(int conn)
{
  //处理目标
  ssize_t ret;

  while (1)
  {
    if ((ret = lwip_recv(conn, recvbuf, sizeof(recvbuf), 0)) == -1) //等待tcp消息
    {
      printf("recv error \r\n");
      lwip_close(conn);
      return;
    }
    printf("recv :%s\r\n", recvbuf);

    printf("http server response!\r\n");
    // 截取“value”之后的内容
    if (strncmp((char *)recvbuf, "GET /gpio", 9) == 0)
    {
      static int get_Pre = 0, F_Pre = 0;
      if (recvbuf[10] == '0' || recvbuf[10] == '1') //调节亮度百分比 000 001 010 100（000~100）
      {
        get_Pre = (recvbuf[10] - 0x30) * 100 + (recvbuf[11] - 0x30) * 10 + (recvbuf[12] - 0x30);
        printf("LED get_Pre[%d]\r\n", get_Pre);
        if (get_Pre >= 0 && get_Pre <= 100)
        {
          LED_BLN_flag = 0;
          LED_Per = F_Pre = get_Pre;
          LED_DUTY_con(LED_Per);
          printf("LED SET Pre[%d]\r\n", LED_Per);
          LED_htmldata_join_write(conn);
        }
        else
          goto gotoerr;
      }
      else if (recvbuf[10] == 'F') //切换亮灭
      {
        LED_BLN_flag = 0;
        if (LED_Per == 0)
        {
          if (F_Pre == 0)
            LED_Per = 100;
          else
            LED_Per = F_Pre;
        }
        else if (LED_Per > 0)
        {
          F_Pre = LED_Per;
          LED_Per = 0;
        }
        LED_DUTY_con(LED_Per);
        printf("LED SET Pre[%d]\r\n", LED_Per);
        LED_htmldata_join_write(conn);
      }
      else if (recvbuf[10] == 'B') //开关呼吸灯模式d
      {
        if (LED_BLN_flag == 0)
          LED_BLN_flag = 1;
        else if (LED_BLN_flag == 1)
          LED_BLN_flag = 0;
        printf("LED BLN MOD[%d]\r\n", LED_BLN_flag);
        BLN_htmldata_join_write(conn);
      }
      else
        goto gotoerr;
    }
    else if (strncmp((char *)recvbuf, "GET /lwip", 9) == 0)
    {
      lwip_write(conn, htmldata, sizeof(htmldata) - 1);
    }
    else
    {
    gotoerr:
      lwip_write(conn, errhtml, sizeof(errhtml) - 1);
    }
    /* Close connection socket */
    lwip_close(conn);
    memset(recvbuf, 0, sizeof(recvbuf));
    printf("http server close!\r\n");
    return;
    //sleep(1);
  }
}

/**
  * @brief  http_task
  * @param  None
  * @retval None
  */
static void http_task(void)
{
  //服务端地址信息
  struct sockaddr_in server_sock;

  //客户端地址信息
  struct sockaddr_in client_sock;
  int sin_size;

  struct sockaddr_in *cli_addr;

  //连接Wifi
  WifiConnect(WIFI_SSID, WIFI_PSK);

  /* create a TCP socket */
  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("can not create socket");
    exit(1);
  }

  /* bind to port 80 at any interface */
  bzero(&server_sock, sizeof(server_sock));
  server_sock.sin_family = AF_INET;
  server_sock.sin_addr.s_addr = htonl(INADDR_ANY); //设置成ANY就相当于设置使用自动分配IP地址
  server_sock.sin_port = htons(_PROT_);

  //调用bind函数绑定socket和地址
  if (bind(sock_fd, (struct sockaddr *)&server_sock, sizeof(struct sockaddr)) == -1)
  {
    perror("bind is error\r\n");
    exit(1);
  }

  //调用listen函数监听(指定port监听)
  /* listen for connections (TCP listen backlog = TCP_BACKLOG = 1) */
  if (listen(sock_fd, TCP_BACKLOG) == -1)
  {
    perror("listen is error\r\n");
    exit(1);
  }
  printf("start accept\n");

  while (1)
  {
    sin_size = sizeof(struct sockaddr_in);
    printf("wait accept newaddr\r\n");
    if ((new_fd = accept(sock_fd, (struct sockaddr *)&client_sock, (socklen_t *)&sin_size)) == -1)
    {
      perror("accept");
      continue;
    }
    cli_addr = malloc(sizeof(struct sockaddr));
    usleep(500000);
    printf("accept addr\r\n");
    if (cli_addr != NULL)
    {
      memcpy(cli_addr, &client_sock, sizeof(struct sockaddr));
      printf("IP:%s\r\n", inet_ntoa(cli_addr->sin_addr));
      http_server(new_fd);
    }
  }
}

/*****LED PWM 初始化*****/
void LED_CON_init(void)
{
  //设置GPIO_2引脚复用功能为PWM
  IoSetFunc(WIFI_IOT_IO_NAME_GPIO_2, WIFI_IOT_IO_FUNC_GPIO_2_PWM2_OUT);

  //设置GPIO_2引脚为输出模式
  GpioSetDir(WIFI_IOT_IO_NAME_GPIO_2, WIFI_IOT_GPIO_DIR_OUT);

  //初始化PWM2端口
  PwmInit(WIFI_IOT_PWM_PORT_PWM2);
  //输出不同占空比的PWM波
  PwmStop(WIFI_IOT_PWM_PORT_PWM2);
}

/*****LED PWM 控制*****/
void LED_DUTY_con(unsigned int per)
{
  unsigned int duty = per * 40000 / 100;
  if (duty == 0)
    PwmStop(WIFI_IOT_PWM_PORT_PWM2);
  else
    //输出不同占空比的PWM波
    PwmStart(WIFI_IOT_PWM_PORT_PWM2, duty, 40000); //0时LED灭，40000的分频160Mhz/40000 = 4Khz
}

/*****LED控制任务*****/
void LED_CON_TASK(void)
{
  unsigned int i;
  static unsigned int old_LED_Per = 100;
  LED_CON_init();
  while (1)
  {
    if (old_LED_Per != LED_Per && LED_BLN_flag == 0)
    {
      printf("LED NOW Per [%d]\r\n", LED_Per);
      old_LED_Per = LED_Per;
    }
    if (LED_BLN_flag == 1)
    {
      for (i = 0; i < 40000; i += 100)
      {
        if (LED_BLN_flag == 0)
          break;
        //输出不同占空比的PWM波
        PwmStart(WIFI_IOT_PWM_PORT_PWM2, i, 40000); //0时LED灭，40000的分频160Mhz/40000 = 4Khz

        usleep(10);
      }
    }
    usleep(10000); //延迟10ms
  }
}

/**************************************************************
 * void WebServerDemo(void)
 *
 * This function initializes the  service.
 **************************************************************/
static void WebServerDemo(void)
{
  wifi_connect_evt_id = osEventFlagsNew(NULL);
  if (wifi_connect_evt_id == NULL)
  {
    printf("Falied to create EventFlags!\n");
  }

  osThreadAttr_t attr;

  attr.name = "http_task";
  attr.attr_bits = 0U;
  attr.cb_mem = NULL;
  attr.cb_size = 0U;
  attr.stack_mem = NULL;
  attr.stack_size = 10240;
  attr.priority = osPriorityNormal;

  if (osThreadNew((osThreadFunc_t)http_task, NULL, &attr) == NULL)
  {
    printf("[WebServerDemo] Falied to create http_task!\n");
  }

  attr.name = "LED_CON_TASK";
  attr.priority = 23;

  if (osThreadNew((osThreadFunc_t)LED_CON_TASK, NULL, &attr) == NULL)
  {
    printf("[WebServerDemo] Falied to create LED_CON_TASK!\n");
  }

  attr.name = "LED_CON_TASK";
  attr.priority = 22;

  if (osThreadNew((osThreadFunc_t)I2CTask, NULL, &attr) == NULL)
  {
    printf("Falied to create I2CTask!\n");
  }
}

APP_FEATURE_INIT(WebServerDemo);
