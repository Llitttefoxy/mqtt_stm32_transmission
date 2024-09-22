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
/*           程序main函数，入口函数头文件              */
/*                                                     */
/*-----------------------------------------------------*/

#ifndef __MAIN_H
#define __MAIN_H

#define SOCK_DHCP		    0      //DHCP功能用的端口，W5500有8个，0-7都行
#define SOCK_TCPS	        1      //TCP连接用的端口， W5500有8个，0-7都行
#define SOCKET_DNS          2      //DNS功能用的端口， W5500有8个，0-7都行
#define MY_MAX_DHCP_RETRY	3      //DHCP重试次数
#define DATA_BUF_SIZE       2048   //缓冲区大小


void W5500_init(void);
void my_ip_assign(void);                     //获取到IP时的回调函数  
void my_ip_conflict(void);                   //获取IP的失败函数
void network_init(void);                     //初始化网络函数
void W5500_TxData(unsigned char *data);      //W5500发送数据
void W5500_DNS(unsigned char *DomainName);   //DNS域名解析 
void LED1_State(void);       //函数声明，判断开关1状态，并发布给服务器
void MQTT_UploadString(unsigned char sta); //MQTT上传字符串函数
void MQTT_UploadData(unsigned char *sta);   //MQTT上传数据函数

#endif











