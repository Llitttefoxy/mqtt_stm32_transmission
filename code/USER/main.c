/*-----------------------------------------------------*/
/*              ModuleStar STM32系列开发板                */
/*-----------------------------------------------------*/
/*                     程序结构                        */
/*-----------------------------------------------------*/
/*USER     ：包含程序的main函数，是整个程序的入口      */
/*HARDWARE ：包含开发板各种功能外设的驱动程序          */
/*CORE     ：包含STM32的核心程序，官方提供，我们不修改 */
/*STLIB    ：官方提供的库文件，我们不修改              */
/*-----------------------------------------------------*/
/*                                                     */
/*           程序main函数，入口函数源文件              */
/*                                                     */
/*-----------------------------------------------------*/

#include "stm32f10x.h"	  //包含需要的头文件
#include "main.h"		  //包含需要的头文件
#include "delay.h"		  //包含需要的头文件
#include "usart1.h"		  //包含需要的头文件
#include "timer1.h"		  //包含需要的头文件
#include "timer3.h"		  //包含需要的头文件
#include "led.h"		  //包含需要的头文件
#include "mqtt.h"		  //包含需要的头文件
#include "key.h"		  //包含需要的头文件
#include "wizchip_conf.h" //包含需要的头文件
#include "socket.h"		  //包含需要的头文件
#include "dhcp.h"		  //包含需要的头文件
#include "spi.h"		  //包含需要的头文件
#include "dns.h"		  //包含需要的头文件
#include "dma.h"		  //包含需要的头文件
#include "chip.h"		  //包含需要的头文件

void LED_PubState(unsigned char sta);
void MQTT_CheckConnectStatus(unsigned char status);

unsigned char gDATABUF[DATA_BUF_SIZE];	 // 数据缓冲区
unsigned char gDNSBUF[MAX_DNS_BUF_SIZE]; // 数据缓冲区
char DNS_flag = 0;						 // DNS解析标志  0:未成功  1：成功
int tcp_state;							 // 用于保存TCP返回的状态
int temp_state;							 // 用于临时保存TCP返回的状态
char my_dhcp_retry = 0;					 // DHCP当前共计重试的次数
char SOCK_flag = 0;						 // 端口链接标志 0：没有链接服务器  1：已经链接服务器，等待服务器响应
uint8_t usart_it_flag=0;
uint32_t Chip_ID[3]={0,0,0};
wiz_NetInfo gWIZNETINFO =				 // MAC地址自己写一个，不要和路由器下其他的设备一样即可
	{
		0x00,
		0x08,
		0xdc,
		0x00,
		0xab,
		0xcd,
};
unsigned char dns2[4] = {114, 114, 114, 114}; // DNS解析服务器IP

int main(void)
{
	int ret;				 // 用于保存函数返回值
	GetChipID(Chip_ID); // 获取芯片ID
	sprintf(P_TOPIC_NAME,"%08X%08X%08X", Chip_ID[0], Chip_ID[1], Chip_ID[2]);
	Delay_Init();			 // 延时功能初始化
	Usart1_Init(115200);	 // 串口1功能初始化，波特率115200
	My_DMA_Init();			 // DMA初始化
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);          //关闭接收中断
	LED_LocationStaInit();	 // LED初始化
	AliIoT_Parameter_Init(); // 初始化连接阿里云IoT平台MQTT服务器的参数
	W5500_init();			 // 初始化W5500
	u1_printf("Chip ID:0x%08x%08x%08x\r\n", Chip_ID[0], Chip_ID[1], Chip_ID[2]); // 输出芯片ID
	while (1)				 // 主循环
	{
		switch (DHCP_run()) // 判断DHCP执行在哪个过程
		{
		case DHCP_IP_ASSIGN:  // 该状态表示，路由器分配给开发板ip了
		case DHCP_IP_CHANGED: // 该状态表示，路由器改变了分配给开发板的ip
			my_ip_assign();	  // 调用IP参数获取函数，记录各个参数
			break;			  // 跳出

		case DHCP_IP_LEASED: // 该状态表示，路由器分配给的开发板ip，正式租用了，表示可以联网通信了
			if (DNS_flag == 0)
			{						   // 如果域名还未解析，进入if
				W5500_DNS(ServerName); // 解析服务器域名
				ServerIP[0]=192;
				ServerIP[1]=168;
				ServerIP[2]=8;
				ServerIP[3]=143;
			}
			else
			{									 // 反之，如果已经解析到了IP，进入else分支链接服务器
				tcp_state = getSn_SR(SOCK_TCPS); // 获取TCP链接端口的状态
				if (tcp_state != temp_state)
				{											   // 如果和上次的状态不一样
					temp_state = tcp_state;					   // 保存本次状态
					u1_printf("状态编码:0x%x\r\n", tcp_state); // 串口输出信息
				}
				if (ctlwizchip(CW_GET_PHYLINK, (void *)&ret) == -1)
				{										 // 如果if成立，表示未知错误
					u1_printf("未知错误，准备重启\r\n"); // 提示信息
					NVIC_SystemReset();					 // 重启
				}
				if (ret == PHY_LINK_OFF)
				{							   // 判断网线是否断开
					u1_printf("网线断开\r\n"); // 如果检测到，网线断开，串口提示信息
					Delay_Ms(1000);			   // 延时1s
				}
				switch (tcp_state)
				{ // switch语句，判断当前TCP链接的状态
				case SOCK_INIT:
					if (Connect_flag == 0)
					{													// 如果还没有链接服务器，进入if
						u1_printf("准备连接服务器\r\n");				// 串口输出信息
						ret = connect(SOCK_TCPS, ServerIP, ServerPort); // 链接服务器
						u1_printf("连接服务器返回码：%d\r\n", ret);		// 串口输出信息
						if (ret == SOCKERR_NOPEN)
						{				   // 服务器未开启进入if
							Delay_Ms(200); // 延时
						}
					}
					break; // 跳出

				case SOCK_ESTABLISHED:
					if(usart_it_flag==0)
					{
						// USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);          //开启接收中断
						MYDMA_Enable();
						usart_it_flag=1;
					}
					if ((Connect_flag == 0) && (getSn_IR(SOCK_TCPS) == Sn_IR_CON))
					{								 // 判断链接是否建立
						u1_printf("连接已建立\r\n"); // 串口输出信息
						MQTT_Buff_Init();			 // 初始化接收,发送,命令数据的 缓冲区 以及各状态参数
						Connect_flag = 1;			 // 链接标志=1
					}
					ret = recv(SOCK_TCPS, gDATABUF, DATA_BUF_SIZE); // 接收数据
					if (ret > 0)
					{												 // 如果ret大于0，表示有数据来
						memcpy(&MQTT_RxDataInPtr[2], gDATABUF, ret); // 拷贝数据到接收缓冲区
						MQTT_RxDataInPtr[0] = ret / 256;			 // 记录数据长度
						MQTT_RxDataInPtr[1] = ret % 256;			 // 记录数据长度
						MQTT_RxDataInPtr += RBUFF_UNIT;				 // 指针下移
						if (MQTT_RxDataInPtr == MQTT_RxDataEndPtr)	 // 如果指针到缓冲区尾部了
							MQTT_RxDataInPtr = MQTT_RxDataBuf[0];	 // 指针归位到缓冲区开头
						TIM_SetCounter(TIM3, 0);					 // 清零定时器3计数器，重新计时ping包发送时间
					}
					/*--------------------------------------------------------------------*/
					/*   Connect_flag=1同服务器建立了连接,我们可以发布数据和接收推送了    */
					/*--------------------------------------------------------------------*/
					if (Connect_flag == 1)
					{
						/*-------------------------------------------------------------*/
						/*                     处理发送缓冲区数据                      */
						/*-------------------------------------------------------------*/
						if (MQTT_TxDataOutPtr != MQTT_TxDataInPtr)
						{ // if成立的话，说明发送缓冲区有数据了
							// 3种情况可进入if
							// 第1种：0x10 连接报文
							// 第2种：0x82 订阅报文，且g_connectPackFlag置位，表示连接报文成功
							// 第3种：SubcribePack_flag置位，说明连接和订阅均成功，其他报文可发
							if ((MQTT_TxDataOutPtr[2] == 0x10) || ((MQTT_TxDataOutPtr[2] == 0x82) && (g_connectPackFlag == 1)) || (SubcribePack_flag == 1))
							{
								u1_printf("发送数据:0x%x\r\n", MQTT_TxDataOutPtr[2]); // 串口提示信息
								MQTT_TxData(MQTT_TxDataOutPtr);						  // 发送数据
								MQTT_TxDataOutPtr += TBUFF_UNIT;					  // 指针下移
								if (MQTT_TxDataOutPtr == MQTT_TxDataEndPtr)			  // 如果指针到缓冲区尾部了
									MQTT_TxDataOutPtr = MQTT_TxDataBuf[0];			  // 指针归位到缓冲区开头
							}
						} // 处理发送缓冲区数据的else if分支结尾

						/*-------------------------------------------------------------*/
						/*                     处理接收缓冲区数据                      */
						/*-------------------------------------------------------------*/
						if (MQTT_RxDataOutPtr != MQTT_RxDataInPtr)
						{ // if成立的话，说明接收缓冲区有数据了
							u1_printf("接收到数据:");
							/*-----------------------------------------------------*/
							/*                    处理CONNACK报文                  */
							/*-----------------------------------------------------*/
							// if判断，如果一共接收了4个字节，第一个字节是0x20，表示收到的是CONNACK报文
							// 接着我们要判断第4个字节，看看CONNECT报文是否成功
							if (MQTT_RxDataOutPtr[2] == 0x20)
							{
								MQTT_CheckConnectStatus(MQTT_RxDataOutPtr[5]);
							}
							// if判断，如果一共接收了5个字节，第一个字节是0x90，表示收到的是SUBACK报文
							// 接着我们要判断订阅回复，看看是不是成功
							else if (MQTT_RxDataOutPtr[2] == 0x90)
							{
								switch (MQTT_RxDataOutPtr[6])
								{
								case 0x00:
								case 0x01:
									u1_printf("订阅成功\r\n"); // 串口输出信息
									SubcribePack_flag = 1;	   // SubcribePack_flag置1，表示订阅报文成功，其他报文可发送
									TIM3_ENABLE_10S();		   // 启动10s的PING定时器
									// TODO LED1_State();                                                              			//判断开关状态，并发布给服务器
									break; // 跳出分支
								default:
									u1_printf("订阅失败，准备重启\r\n"); // 串口输出信息
									NVIC_SystemReset();					 // 重启
									break;								 // 跳出分支
								}
							}
							// if判断，如果一共接收了2个字节，第一个字节是0xD0，表示收到的是PINGRESP报文
							else if (MQTT_RxDataOutPtr[2] == 0xD0)
							{
								u1_printf("PING报文回复\r\n"); // 串口输出信息
							}
							// if判断，如果第一个字节是0x30，表示收到的是服务器发来的推送数据
							// 我们要提取控制命令
							else if ((MQTT_RxDataOutPtr[2] == 0x30))
							{
								u1_printf("服务器等级0推送\r\n");		  // 串口输出信息
								MQTT_DealPushdata_Qs0(MQTT_RxDataOutPtr); // 处理等级0推送数据
							}
							MQTT_RxDataOutPtr += RBUFF_UNIT;			// 指针下移
							if (MQTT_RxDataOutPtr == MQTT_RxDataEndPtr) // 如果指针到缓冲区尾部了
								MQTT_RxDataOutPtr = MQTT_RxDataBuf[0];	// 指针归位到缓冲区开头
						}												// 处理接收缓冲区数据的else if分支结尾
						/*-------------------------------------------------------------*/
						/*                     处理命令缓冲区数据                      */
						/*-------------------------------------------------------------*/
						if (MQTT_CMDOutPtr != MQTT_CMDInPtr)
						{												  // if成立的话，说明命令缓冲区有数据了
							u1_printf("命令:%s\r\n", &MQTT_CMDOutPtr[2]); // 串口输出信息
							if (strstr((char *)MQTT_CMDOutPtr + 2, "\"switcher\":1,\"flag\":\"SW1\""))
							{						 // 如果搜索到"params":{"powerstate":1}说明服务器下发打开开关1
								LED_LocationStaOn(); // 打开LED1
								LED_PubState(1);	 // 判断开关状态，并发布给服务器
							}
							else if (strstr((char *)MQTT_CMDOutPtr + 2, "\"switcher\":0,\"flag\":\"SW1\""))
							{						  // 如果搜索到"params":{"powerstate":0}说明服务器下发关闭开关1
								LED_LocationStaOff(); // 关闭LED1
								LED_PubState(0);	  // 判断开关状态，并发布给服务器
							}
							MQTT_CMDOutPtr += CBUFF_UNIT;		  // 指针下移
							if (MQTT_CMDOutPtr == MQTT_CMDEndPtr) // 如果指针到缓冲区尾部了
								MQTT_CMDOutPtr = MQTT_CMDBuf[0];  // 指针归位到缓冲区开头
						}
					}
					break; // 跳出

				case SOCK_CLOSE_WAIT:
					u1_printf("等待关闭连接\r\n");				  // 串口输出信息
					if ((ret = disconnect(SOCK_TCPS)) != SOCK_OK) // 端口关闭
					{
						u1_printf("连接关闭失败，准备重启\r\n"); // 串口输出信息
						NVIC_SystemReset();						 // 重启
					}
					u1_printf("连接关闭成功\r\n"); // 串口输出信息
					Connect_flag = 0;			   // 链接标志=0
					break;

				case SOCK_CLOSED:
					u1_printf("准备打开W5500端口\r\n");					// 串口输出信息
					Connect_flag = 0;									// 链接标志=0
					ret = socket(SOCK_TCPS, Sn_MR_TCP, 5050, Sn_MR_ND); // 打开W5500的端口，用于建立TCP链接，本地TCP端口5050
					if (ret != SOCK_TCPS)
					{											 // 如果打开失败，进入if
						u1_printf("端口打开错误，准备重启\r\n"); // 串口输出信息
						NVIC_SystemReset();						 // 重启
					}
					else
						u1_printf("打开5050端口成功\r\n"); // 串口输出信息
					break;								   // 跳出
				}
			}
			break; // 跳出

		case DHCP_FAILED:	 // 该状态表示DHCP获取IP失败
			my_dhcp_retry++; // 失败次数+1
			if (my_dhcp_retry > MY_MAX_DHCP_RETRY)
			{										 // 如果失败次数大于最大次数，进入if
				u1_printf("DHCP失败，准备重启\r\n"); // 串口提示信息
				NVIC_SystemReset();					 // 重启
			}
			break; // 跳出
		}
	}
}

void MQTT_CheckConnectStatus(unsigned char status)
{
	switch (status)
	{
	case 0x00:
		u1_printf("CONNECT报文成功\r\n");
		g_connectPackFlag = 1; // CONNECT报文成功
		break;
	case 0x01:
		u1_printf("连接已拒绝，不支持的协议版本，准备重启\r\n");
		NVIC_SystemReset();
		break;
	case 0x02:
		u1_printf("连接已拒绝，不合格的客户端标识符，准备重启\r\n");
		NVIC_SystemReset();
		break;
	case 0x03:
		u1_printf("连接已拒绝，服务端不可用，准备重启\r\n");
		NVIC_SystemReset();
		break;
	case 0x04:
		u1_printf("连接已拒绝，无效的用户名或密码，准备重启\r\n");
		NVIC_SystemReset();
		break;
	case 0x05:
		u1_printf("连接已拒绝，未授权，准备重启\r\n");
		NVIC_SystemReset();
		break;
	default:
		u1_printf("连接已拒绝，未知状态，准备重启\r\n");
		NVIC_SystemReset();
		break;
	}
	return;
}

/*-------------------------------------------------*/
/*函数名：上传一段字符串				            */
/*参  数：字符                                     */
/*返回值：无                                       */
/*-------------------------------------------------*/
void MQTT_UploadString(unsigned char sta)
{
	char temp[TBUFF_UNIT];
	sprintf(temp,"{\"sensorDatas\":[{\"flag\":\"gas_data\",\"str\":\"%c\"}]}", sta);    //构建数据
	MQTT_PublishQs0(P_TOPIC_NAME, temp, strlen(temp));
}

/*-------------------------------------------------*/
/*函数名：上传一段数据  				            */
/*参  数：字符                                     */
/*返回值：无                                       */
/*-------------------------------------------------*/
void MQTT_UploadData(unsigned char *sta)
{
	char temp[TBUFF_UNIT];
	sprintf(temp,"{\"sensorDatas\":[{\"flag\":\"data_test\",\"value\":\"%d\"}]}", sta[0]*256+sta[1]);    //构建数据
	MQTT_PublishQs0(P_TOPIC_NAME, temp, strlen(temp));
}

/*-------------------------------------------------*/
/*函数名：判断开关状态，并发布给服务器            */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void LED_PubState(unsigned char sta)
{
	char temp[TBUFF_UNIT];
	sprintf(temp,"{\"sensorDatas\":[{\"flag\":\"SWSTA1\",\"switcher\":%d},{\"flag\":\"SW1\",\"switcher\":%d}]}", sta,sta);    //构建数据
	MQTT_PublishQs0(P_TOPIC_NAME, temp, strlen(temp));
}

/*-------------------------------------------------*/
/*函数名：获取到IP时的回调函数                     */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void my_ip_assign(void)
{
	getIPfromDHCP(gWIZNETINFO.ip);	 // 把获取到的ip参数，记录到机构体中
	getGWfromDHCP(gWIZNETINFO.gw);	 // 把获取到的网关参数，记录到机构体中
	getSNfromDHCP(gWIZNETINFO.sn);	 // 把获取到的子网掩码参数，记录到机构体中
	getDNSfromDHCP(gWIZNETINFO.dns); // 把获取到的DNS服务器参数，记录到机构体中
	gWIZNETINFO.dhcp = NETINFO_DHCP; // 标记使用的是DHCP方式
	network_init();					 // 初始化网络
	u1_printf("DHCP租期 : %d 秒\r\n", getDHCPLeasetime());
}
/*-------------------------------------------------*/
/*函数名：获取IP的失败函数                         */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void my_ip_conflict(void)
{
	u1_printf("获取IP失败，准备重启\r\n"); // 提示获取IP失败
	NVIC_SystemReset();					   // 重启
}
/*-------------------------------------------------*/
/*函数名：初始化网络函数                           */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void network_init(void)
{
	char tmpstr[6] = {0};
	wiz_NetInfo netinfo;

	ctlnetwork(CN_SET_NETINFO, (void *)&gWIZNETINFO); // 设置网络参数
	ctlnetwork(CN_GET_NETINFO, (void *)&netinfo);	  // 读取网络参数
	ctlwizchip(CW_GET_ID, (void *)tmpstr);			  // 读取芯片ID

	// 打印网络参数
	if (netinfo.dhcp == NETINFO_DHCP)
		u1_printf("\r\n=== %s NET CONF : DHCP ===\r\n", (char *)tmpstr);
	else
		u1_printf("\r\n=== %s NET CONF : Static ===\r\n", (char *)tmpstr);
	u1_printf("===========================\r\n");
	u1_printf("MAC地址: %02X:%02X:%02X:%02X:%02X:%02X\r\n", netinfo.mac[0], netinfo.mac[1], netinfo.mac[2], netinfo.mac[3], netinfo.mac[4], netinfo.mac[5]);
	u1_printf("IP地址: %d.%d.%d.%d\r\n", netinfo.ip[0], netinfo.ip[1], netinfo.ip[2], netinfo.ip[3]);
	u1_printf("网关地址: %d.%d.%d.%d\r\n", netinfo.gw[0], netinfo.gw[1], netinfo.gw[2], netinfo.gw[3]);
	u1_printf("子网掩码: %d.%d.%d.%d\r\n", netinfo.sn[0], netinfo.sn[1], netinfo.sn[2], netinfo.sn[3]);
	u1_printf("DNS服务器: %d.%d.%d.%d\r\n", netinfo.dns[0], netinfo.dns[1], netinfo.dns[2], netinfo.dns[3]);
	u1_printf("===========================\r\n");
}
/*-------------------------------------------------*/
/*函数名：初始化W5500                              */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void W5500_init(void)
{
	// W5500收发内存分区，收发缓冲区各自总的空间是16K，（0-7）每个端口的收发缓冲区我们分配 2K
	char memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
	char tmp;

	SPI_Configuration();								   // 初始化SPI接口
	reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);  // 注册临界区函数
	reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect); // 注册SPI片选信号函数
	reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte);   // 注册读写函数
	if (ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1)
	{												  // 如果if成立，表示收发内存分区失败
		u1_printf("初始化收发分区失败,准备重启\r\n"); // 提示信息
		NVIC_SystemReset();							  // 重启
	}
	do
	{ // 检查连接状态
		if (ctlwizchip(CW_GET_PHYLINK, (void *)&tmp) == -1)
		{										 // 如果if成立，表示未知错误
			u1_printf("未知错误，准备重启\r\n"); // 提示信息
			NVIC_SystemReset();					 // 重启
		}
		if (tmp == PHY_LINK_OFF)
		{
			u1_printf("网线未连接\r\n"); // 如果检测到，网线没连接，提示连接网线
			Delay_Ms(2000);				 // 延时2s
		}
	} while (tmp == PHY_LINK_OFF); // 循环执行，直到连接上网线

	setSHAR(gWIZNETINFO.mac);									 // 设置MAC地址
	DHCP_init(SOCK_DHCP, gDATABUF);								 // 初始化DHCP
	reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict); // 注册DHCP回调函数
	DNS_flag = 0;												 // DNS解析标志=0
	my_dhcp_retry = 0;											 // DHCP重试次数=0
	tcp_state = 0;												 // TCP状态=0
	temp_state = -1;											 // 上一次TCP状态=-1
}
/*-------------------------------------------------*/
/*函数名：W5500发送数据                            */
/*参  数：data:数据                                */
/*返回值：无                                       */
/*-------------------------------------------------*/
void W5500_TxData(unsigned char *data)
{
	int len;
	len = send(SOCK_TCPS, &data[2], data[0] * 256 + data[1]); // W5500发送数据
	if (len == (data[0] * 256 + data[1]))
		u1_printf("发送数据成功\r\n");
}
/*-------------------------------------------------*/
/*函数名：DNS域名解析                              */
/*参  数：DomainName:域名                          */
/*返回值：无                                       */
/*-------------------------------------------------*/
void W5500_DNS(unsigned char *DomainName)
{
	char ret;

	u1_printf("开始解析域名\r\n");			   // 串口输出信息
	DNS_init(SOCKET_DNS, gDNSBUF);			   // DNS初始化
	ret = DNS_run(dns2, DomainName, ServerIP); // 解析域名
	u1_printf("DNS解析返回值：%d\r\n", ret);   // 串口输出数据
	if (ret == 1)
	{																								   // 返回值等于1，表示解析正确
		u1_printf("DNS解析成功：%d.%d.%d.%d\r\n", ServerIP[0], ServerIP[1], ServerIP[2], ServerIP[3]); // 串口输出信息
		DNS_flag = 1;																				   // DNS解析成功，标志=1
		close(SOCKET_DNS);																			   // 关闭DNS端口
	}
}
