 * 宏定义  *#define 
 */
#define WIFI_SSID "TP"
#define WIFI_PSK "1356630466"


//web HTML网站信息
unsigned char LED_htmldataIP[] = "119.75.217.109"//"192.168.1.200";

 //连接Wifi
  WifiConnect(WIFI_SSID, WIFI_PSK);
  进入静态分配IP的函数

接收tcp消息：

      if ((ret = lwip_recv(conn, recvbuf, sizeof(recvbuf), 0)) == -1) //等待tcp消息
  
接收信息后进行数据判断，正确后发送相应网页信息的代码：
    else if (strncmp((char *)recvbuf, "GET /lwip", 9) == 0)
    {
      lwip_write(conn, htmldata, sizeof(htmldata) - 1);
    }
否则发送http错误
    else
    {
    gotoerr:
      lwip_write(conn, errhtml, sizeof(errhtml) - 1);
    }

创建TCPsocket

  /* create a TCP socket */
  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)  //domain, AF_INET;
                                                          //type, SOCK_STREAM;
                                                          //protocol, 0;
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


  //调用listen函数监听(指定port监听)
  /* listen for connections (TCP listen backlog = TCP_BACKLOG = 1) */
  if (listen(sock_fd, TCP_BACKLOG) == -1)
