/*-----------------------------------------------------*/
/*              ModuleStar STM32ϵ�п�����                */
/*-----------------------------------------------------*/
/*                     ����ṹ                        */
/*-----------------------------------------------------*/
/*USER     �����������main��������������������      */
/*HARDWARE ��������������ֹ����������������          */
/*CORE     ������STM32�ĺ��ĳ��򣬹ٷ��ṩ�����ǲ��޸� */
/*STLIB    ���ٷ��ṩ�Ŀ��ļ������ǲ��޸�              */
/*-----------------------------------------------------*/
/*                                                     */
/*           ����main��������ں���ͷ�ļ�              */
/*                                                     */
/*-----------------------------------------------------*/

#ifndef __MAIN_H
#define __MAIN_H

#define SOCK_DHCP		    0      //DHCP�����õĶ˿ڣ�W5500��8����0-7����
#define SOCK_TCPS	        1      //TCP�����õĶ˿ڣ� W5500��8����0-7����
#define SOCKET_DNS          2      //DNS�����õĶ˿ڣ� W5500��8����0-7����
#define MY_MAX_DHCP_RETRY	3      //DHCP���Դ���
#define DATA_BUF_SIZE       2048   //��������С


void W5500_init(void);
void my_ip_assign(void);                     //��ȡ��IPʱ�Ļص�����  
void my_ip_conflict(void);                   //��ȡIP��ʧ�ܺ���
void network_init(void);                     //��ʼ�����纯��
void W5500_TxData(unsigned char *data);      //W5500��������
void W5500_DNS(unsigned char *DomainName);   //DNS�������� 
void LED1_State(void);       //�����������жϿ���1״̬����������������
void MQTT_UploadString(unsigned char sta); //MQTT�ϴ��ַ�������
void MQTT_UploadData(unsigned char *sta);   //MQTT�ϴ����ݺ���

#endif











