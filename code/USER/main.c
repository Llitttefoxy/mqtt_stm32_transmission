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
/*           ����main��������ں���Դ�ļ�              */
/*                                                     */
/*-----------------------------------------------------*/

#include "stm32f10x.h"	  //������Ҫ��ͷ�ļ�
#include "main.h"		  //������Ҫ��ͷ�ļ�
#include "delay.h"		  //������Ҫ��ͷ�ļ�
#include "usart1.h"		  //������Ҫ��ͷ�ļ�
#include "timer1.h"		  //������Ҫ��ͷ�ļ�
#include "timer3.h"		  //������Ҫ��ͷ�ļ�
#include "led.h"		  //������Ҫ��ͷ�ļ�
#include "mqtt.h"		  //������Ҫ��ͷ�ļ�
#include "key.h"		  //������Ҫ��ͷ�ļ�
#include "wizchip_conf.h" //������Ҫ��ͷ�ļ�
#include "socket.h"		  //������Ҫ��ͷ�ļ�
#include "dhcp.h"		  //������Ҫ��ͷ�ļ�
#include "spi.h"		  //������Ҫ��ͷ�ļ�
#include "dns.h"		  //������Ҫ��ͷ�ļ�
#include "dma.h"		  //������Ҫ��ͷ�ļ�
#include "chip.h"		  //������Ҫ��ͷ�ļ�

void LED_PubState(unsigned char sta);
void MQTT_CheckConnectStatus(unsigned char status);

unsigned char gDATABUF[DATA_BUF_SIZE];	 // ���ݻ�����
unsigned char gDNSBUF[MAX_DNS_BUF_SIZE]; // ���ݻ�����
char DNS_flag = 0;						 // DNS������־  0:δ�ɹ�  1���ɹ�
int tcp_state;							 // ���ڱ���TCP���ص�״̬
int temp_state;							 // ������ʱ����TCP���ص�״̬
char my_dhcp_retry = 0;					 // DHCP��ǰ�������ԵĴ���
char SOCK_flag = 0;						 // �˿����ӱ�־ 0��û�����ӷ�����  1���Ѿ����ӷ��������ȴ���������Ӧ
uint8_t usart_it_flag=0;
uint32_t Chip_ID[3]={0,0,0};
wiz_NetInfo gWIZNETINFO =				 // MAC��ַ�Լ�дһ������Ҫ��·�������������豸һ������
	{
		0x00,
		0x08,
		0xdc,
		0x00,
		0xab,
		0xcd,
};
unsigned char dns2[4] = {114, 114, 114, 114}; // DNS����������IP

int main(void)
{
	int ret;				 // ���ڱ��溯������ֵ
	GetChipID(Chip_ID); // ��ȡоƬID
	sprintf(P_TOPIC_NAME,"%08X%08X%08X", Chip_ID[0], Chip_ID[1], Chip_ID[2]);
	Delay_Init();			 // ��ʱ���ܳ�ʼ��
	Usart1_Init(115200);	 // ����1���ܳ�ʼ����������115200
	My_DMA_Init();			 // DMA��ʼ��
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);          //�رս����ж�
	LED_LocationStaInit();	 // LED��ʼ��
	AliIoT_Parameter_Init(); // ��ʼ�����Ӱ�����IoTƽ̨MQTT�������Ĳ���
	W5500_init();			 // ��ʼ��W5500
	u1_printf("Chip ID:0x%08x%08x%08x\r\n", Chip_ID[0], Chip_ID[1], Chip_ID[2]); // ���оƬID
	while (1)				 // ��ѭ��
	{
		switch (DHCP_run()) // �ж�DHCPִ�����ĸ�����
		{
		case DHCP_IP_ASSIGN:  // ��״̬��ʾ��·���������������ip��
		case DHCP_IP_CHANGED: // ��״̬��ʾ��·�����ı��˷�����������ip
			my_ip_assign();	  // ����IP������ȡ��������¼��������
			break;			  // ����

		case DHCP_IP_LEASED: // ��״̬��ʾ��·����������Ŀ�����ip����ʽ�����ˣ���ʾ��������ͨ����
			if (DNS_flag == 0)
			{						   // ���������δ����������if
				W5500_DNS(ServerName); // ��������������
				ServerIP[0]=192;
				ServerIP[1]=168;
				ServerIP[2]=8;
				ServerIP[3]=143;
			}
			else
			{									 // ��֮������Ѿ���������IP������else��֧���ӷ�����
				tcp_state = getSn_SR(SOCK_TCPS); // ��ȡTCP���Ӷ˿ڵ�״̬
				if (tcp_state != temp_state)
				{											   // ������ϴε�״̬��һ��
					temp_state = tcp_state;					   // ���汾��״̬
					u1_printf("״̬����:0x%x\r\n", tcp_state); // ���������Ϣ
				}
				if (ctlwizchip(CW_GET_PHYLINK, (void *)&ret) == -1)
				{										 // ���if��������ʾδ֪����
					u1_printf("δ֪����׼������\r\n"); // ��ʾ��Ϣ
					NVIC_SystemReset();					 // ����
				}
				if (ret == PHY_LINK_OFF)
				{							   // �ж������Ƿ�Ͽ�
					u1_printf("���߶Ͽ�\r\n"); // �����⵽�����߶Ͽ���������ʾ��Ϣ
					Delay_Ms(1000);			   // ��ʱ1s
				}
				switch (tcp_state)
				{ // switch��䣬�жϵ�ǰTCP���ӵ�״̬
				case SOCK_INIT:
					if (Connect_flag == 0)
					{													// �����û�����ӷ�����������if
						u1_printf("׼�����ӷ�����\r\n");				// ���������Ϣ
						ret = connect(SOCK_TCPS, ServerIP, ServerPort); // ���ӷ�����
						u1_printf("���ӷ����������룺%d\r\n", ret);		// ���������Ϣ
						if (ret == SOCKERR_NOPEN)
						{				   // ������δ��������if
							Delay_Ms(200); // ��ʱ
						}
					}
					break; // ����

				case SOCK_ESTABLISHED:
					if(usart_it_flag==0)
					{
						// USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);          //���������ж�
						MYDMA_Enable();
						usart_it_flag=1;
					}
					if ((Connect_flag == 0) && (getSn_IR(SOCK_TCPS) == Sn_IR_CON))
					{								 // �ж������Ƿ���
						u1_printf("�����ѽ���\r\n"); // ���������Ϣ
						MQTT_Buff_Init();			 // ��ʼ������,����,�������ݵ� ������ �Լ���״̬����
						Connect_flag = 1;			 // ���ӱ�־=1
					}
					ret = recv(SOCK_TCPS, gDATABUF, DATA_BUF_SIZE); // ��������
					if (ret > 0)
					{												 // ���ret����0����ʾ��������
						memcpy(&MQTT_RxDataInPtr[2], gDATABUF, ret); // �������ݵ����ջ�����
						MQTT_RxDataInPtr[0] = ret / 256;			 // ��¼���ݳ���
						MQTT_RxDataInPtr[1] = ret % 256;			 // ��¼���ݳ���
						MQTT_RxDataInPtr += RBUFF_UNIT;				 // ָ������
						if (MQTT_RxDataInPtr == MQTT_RxDataEndPtr)	 // ���ָ�뵽������β����
							MQTT_RxDataInPtr = MQTT_RxDataBuf[0];	 // ָ���λ����������ͷ
						TIM_SetCounter(TIM3, 0);					 // ���㶨ʱ��3�����������¼�ʱping������ʱ��
					}
					/*--------------------------------------------------------------------*/
					/*   Connect_flag=1ͬ����������������,���ǿ��Է������ݺͽ���������    */
					/*--------------------------------------------------------------------*/
					if (Connect_flag == 1)
					{
						/*-------------------------------------------------------------*/
						/*                     �����ͻ���������                      */
						/*-------------------------------------------------------------*/
						if (MQTT_TxDataOutPtr != MQTT_TxDataInPtr)
						{ // if�����Ļ���˵�����ͻ�������������
							// 3������ɽ���if
							// ��1�֣�0x10 ���ӱ���
							// ��2�֣�0x82 ���ı��ģ���g_connectPackFlag��λ����ʾ���ӱ��ĳɹ�
							// ��3�֣�SubcribePack_flag��λ��˵�����ӺͶ��ľ��ɹ����������Ŀɷ�
							if ((MQTT_TxDataOutPtr[2] == 0x10) || ((MQTT_TxDataOutPtr[2] == 0x82) && (g_connectPackFlag == 1)) || (SubcribePack_flag == 1))
							{
								u1_printf("��������:0x%x\r\n", MQTT_TxDataOutPtr[2]); // ������ʾ��Ϣ
								MQTT_TxData(MQTT_TxDataOutPtr);						  // ��������
								MQTT_TxDataOutPtr += TBUFF_UNIT;					  // ָ������
								if (MQTT_TxDataOutPtr == MQTT_TxDataEndPtr)			  // ���ָ�뵽������β����
									MQTT_TxDataOutPtr = MQTT_TxDataBuf[0];			  // ָ���λ����������ͷ
							}
						} // �����ͻ��������ݵ�else if��֧��β

						/*-------------------------------------------------------------*/
						/*                     ������ջ���������                      */
						/*-------------------------------------------------------------*/
						if (MQTT_RxDataOutPtr != MQTT_RxDataInPtr)
						{ // if�����Ļ���˵�����ջ�������������
							u1_printf("���յ�����:");
							/*-----------------------------------------------------*/
							/*                    ����CONNACK����                  */
							/*-----------------------------------------------------*/
							// if�жϣ����һ��������4���ֽڣ���һ���ֽ���0x20����ʾ�յ�����CONNACK����
							// ��������Ҫ�жϵ�4���ֽڣ�����CONNECT�����Ƿ�ɹ�
							if (MQTT_RxDataOutPtr[2] == 0x20)
							{
								MQTT_CheckConnectStatus(MQTT_RxDataOutPtr[5]);
							}
							// if�жϣ����һ��������5���ֽڣ���һ���ֽ���0x90����ʾ�յ�����SUBACK����
							// ��������Ҫ�ж϶��Ļظ��������ǲ��ǳɹ�
							else if (MQTT_RxDataOutPtr[2] == 0x90)
							{
								switch (MQTT_RxDataOutPtr[6])
								{
								case 0x00:
								case 0x01:
									u1_printf("���ĳɹ�\r\n"); // ���������Ϣ
									SubcribePack_flag = 1;	   // SubcribePack_flag��1����ʾ���ı��ĳɹ����������Ŀɷ���
									TIM3_ENABLE_10S();		   // ����10s��PING��ʱ��
									// TODO LED1_State();                                                              			//�жϿ���״̬����������������
									break; // ������֧
								default:
									u1_printf("����ʧ�ܣ�׼������\r\n"); // ���������Ϣ
									NVIC_SystemReset();					 // ����
									break;								 // ������֧
								}
							}
							// if�жϣ����һ��������2���ֽڣ���һ���ֽ���0xD0����ʾ�յ�����PINGRESP����
							else if (MQTT_RxDataOutPtr[2] == 0xD0)
							{
								u1_printf("PING���Ļظ�\r\n"); // ���������Ϣ
							}
							// if�жϣ������һ���ֽ���0x30����ʾ�յ����Ƿ�������������������
							// ����Ҫ��ȡ��������
							else if ((MQTT_RxDataOutPtr[2] == 0x30))
							{
								u1_printf("�������ȼ�0����\r\n");		  // ���������Ϣ
								MQTT_DealPushdata_Qs0(MQTT_RxDataOutPtr); // ����ȼ�0��������
							}
							MQTT_RxDataOutPtr += RBUFF_UNIT;			// ָ������
							if (MQTT_RxDataOutPtr == MQTT_RxDataEndPtr) // ���ָ�뵽������β����
								MQTT_RxDataOutPtr = MQTT_RxDataBuf[0];	// ָ���λ����������ͷ
						}												// ������ջ��������ݵ�else if��֧��β
						/*-------------------------------------------------------------*/
						/*                     ���������������                      */
						/*-------------------------------------------------------------*/
						if (MQTT_CMDOutPtr != MQTT_CMDInPtr)
						{												  // if�����Ļ���˵�����������������
							u1_printf("����:%s\r\n", &MQTT_CMDOutPtr[2]); // ���������Ϣ
							if (strstr((char *)MQTT_CMDOutPtr + 2, "\"switcher\":1,\"flag\":\"SW1\""))
							{						 // ���������"params":{"powerstate":1}˵���������·��򿪿���1
								LED_LocationStaOn(); // ��LED1
								LED_PubState(1);	 // �жϿ���״̬����������������
							}
							else if (strstr((char *)MQTT_CMDOutPtr + 2, "\"switcher\":0,\"flag\":\"SW1\""))
							{						  // ���������"params":{"powerstate":0}˵���������·��رտ���1
								LED_LocationStaOff(); // �ر�LED1
								LED_PubState(0);	  // �жϿ���״̬����������������
							}
							MQTT_CMDOutPtr += CBUFF_UNIT;		  // ָ������
							if (MQTT_CMDOutPtr == MQTT_CMDEndPtr) // ���ָ�뵽������β����
								MQTT_CMDOutPtr = MQTT_CMDBuf[0];  // ָ���λ����������ͷ
						}
					}
					break; // ����

				case SOCK_CLOSE_WAIT:
					u1_printf("�ȴ��ر�����\r\n");				  // ���������Ϣ
					if ((ret = disconnect(SOCK_TCPS)) != SOCK_OK) // �˿ڹر�
					{
						u1_printf("���ӹر�ʧ�ܣ�׼������\r\n"); // ���������Ϣ
						NVIC_SystemReset();						 // ����
					}
					u1_printf("���ӹرճɹ�\r\n"); // ���������Ϣ
					Connect_flag = 0;			   // ���ӱ�־=0
					break;

				case SOCK_CLOSED:
					u1_printf("׼����W5500�˿�\r\n");					// ���������Ϣ
					Connect_flag = 0;									// ���ӱ�־=0
					ret = socket(SOCK_TCPS, Sn_MR_TCP, 5050, Sn_MR_ND); // ��W5500�Ķ˿ڣ����ڽ���TCP���ӣ�����TCP�˿�5050
					if (ret != SOCK_TCPS)
					{											 // �����ʧ�ܣ�����if
						u1_printf("�˿ڴ򿪴���׼������\r\n"); // ���������Ϣ
						NVIC_SystemReset();						 // ����
					}
					else
						u1_printf("��5050�˿ڳɹ�\r\n"); // ���������Ϣ
					break;								   // ����
				}
			}
			break; // ����

		case DHCP_FAILED:	 // ��״̬��ʾDHCP��ȡIPʧ��
			my_dhcp_retry++; // ʧ�ܴ���+1
			if (my_dhcp_retry > MY_MAX_DHCP_RETRY)
			{										 // ���ʧ�ܴ�������������������if
				u1_printf("DHCPʧ�ܣ�׼������\r\n"); // ������ʾ��Ϣ
				NVIC_SystemReset();					 // ����
			}
			break; // ����
		}
	}
}

void MQTT_CheckConnectStatus(unsigned char status)
{
	switch (status)
	{
	case 0x00:
		u1_printf("CONNECT���ĳɹ�\r\n");
		g_connectPackFlag = 1; // CONNECT���ĳɹ�
		break;
	case 0x01:
		u1_printf("�����Ѿܾ�����֧�ֵ�Э��汾��׼������\r\n");
		NVIC_SystemReset();
		break;
	case 0x02:
		u1_printf("�����Ѿܾ������ϸ�Ŀͻ��˱�ʶ����׼������\r\n");
		NVIC_SystemReset();
		break;
	case 0x03:
		u1_printf("�����Ѿܾ�������˲����ã�׼������\r\n");
		NVIC_SystemReset();
		break;
	case 0x04:
		u1_printf("�����Ѿܾ�����Ч���û��������룬׼������\r\n");
		NVIC_SystemReset();
		break;
	case 0x05:
		u1_printf("�����Ѿܾ���δ��Ȩ��׼������\r\n");
		NVIC_SystemReset();
		break;
	default:
		u1_printf("�����Ѿܾ���δ֪״̬��׼������\r\n");
		NVIC_SystemReset();
		break;
	}
	return;
}

/*-------------------------------------------------*/
/*���������ϴ�һ���ַ���				            */
/*��  �����ַ�                                     */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void MQTT_UploadString(unsigned char sta)
{
	char temp[TBUFF_UNIT];
	sprintf(temp,"{\"sensorDatas\":[{\"flag\":\"gas_data\",\"str\":\"%c\"}]}", sta);    //��������
	MQTT_PublishQs0(P_TOPIC_NAME, temp, strlen(temp));
}

/*-------------------------------------------------*/
/*���������ϴ�һ������  				            */
/*��  �����ַ�                                     */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void MQTT_UploadData(unsigned char *sta)
{
	char temp[TBUFF_UNIT];
	sprintf(temp,"{\"sensorDatas\":[{\"flag\":\"data_test\",\"value\":\"%d\"}]}", sta[0]*256+sta[1]);    //��������
	MQTT_PublishQs0(P_TOPIC_NAME, temp, strlen(temp));
}

/*-------------------------------------------------*/
/*���������жϿ���״̬����������������            */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void LED_PubState(unsigned char sta)
{
	char temp[TBUFF_UNIT];
	sprintf(temp,"{\"sensorDatas\":[{\"flag\":\"SWSTA1\",\"switcher\":%d},{\"flag\":\"SW1\",\"switcher\":%d}]}", sta,sta);    //��������
	MQTT_PublishQs0(P_TOPIC_NAME, temp, strlen(temp));
}

/*-------------------------------------------------*/
/*����������ȡ��IPʱ�Ļص�����                     */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void my_ip_assign(void)
{
	getIPfromDHCP(gWIZNETINFO.ip);	 // �ѻ�ȡ����ip��������¼����������
	getGWfromDHCP(gWIZNETINFO.gw);	 // �ѻ�ȡ�������ز�������¼����������
	getSNfromDHCP(gWIZNETINFO.sn);	 // �ѻ�ȡ�������������������¼����������
	getDNSfromDHCP(gWIZNETINFO.dns); // �ѻ�ȡ����DNS��������������¼����������
	gWIZNETINFO.dhcp = NETINFO_DHCP; // ���ʹ�õ���DHCP��ʽ
	network_init();					 // ��ʼ������
	u1_printf("DHCP���� : %d ��\r\n", getDHCPLeasetime());
}
/*-------------------------------------------------*/
/*����������ȡIP��ʧ�ܺ���                         */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void my_ip_conflict(void)
{
	u1_printf("��ȡIPʧ�ܣ�׼������\r\n"); // ��ʾ��ȡIPʧ��
	NVIC_SystemReset();					   // ����
}
/*-------------------------------------------------*/
/*����������ʼ�����纯��                           */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void network_init(void)
{
	char tmpstr[6] = {0};
	wiz_NetInfo netinfo;

	ctlnetwork(CN_SET_NETINFO, (void *)&gWIZNETINFO); // �����������
	ctlnetwork(CN_GET_NETINFO, (void *)&netinfo);	  // ��ȡ�������
	ctlwizchip(CW_GET_ID, (void *)tmpstr);			  // ��ȡоƬID

	// ��ӡ�������
	if (netinfo.dhcp == NETINFO_DHCP)
		u1_printf("\r\n=== %s NET CONF : DHCP ===\r\n", (char *)tmpstr);
	else
		u1_printf("\r\n=== %s NET CONF : Static ===\r\n", (char *)tmpstr);
	u1_printf("===========================\r\n");
	u1_printf("MAC��ַ: %02X:%02X:%02X:%02X:%02X:%02X\r\n", netinfo.mac[0], netinfo.mac[1], netinfo.mac[2], netinfo.mac[3], netinfo.mac[4], netinfo.mac[5]);
	u1_printf("IP��ַ: %d.%d.%d.%d\r\n", netinfo.ip[0], netinfo.ip[1], netinfo.ip[2], netinfo.ip[3]);
	u1_printf("���ص�ַ: %d.%d.%d.%d\r\n", netinfo.gw[0], netinfo.gw[1], netinfo.gw[2], netinfo.gw[3]);
	u1_printf("��������: %d.%d.%d.%d\r\n", netinfo.sn[0], netinfo.sn[1], netinfo.sn[2], netinfo.sn[3]);
	u1_printf("DNS������: %d.%d.%d.%d\r\n", netinfo.dns[0], netinfo.dns[1], netinfo.dns[2], netinfo.dns[3]);
	u1_printf("===========================\r\n");
}
/*-------------------------------------------------*/
/*����������ʼ��W5500                              */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void W5500_init(void)
{
	// W5500�շ��ڴ�������շ������������ܵĿռ���16K����0-7��ÿ���˿ڵ��շ����������Ƿ��� 2K
	char memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
	char tmp;

	SPI_Configuration();								   // ��ʼ��SPI�ӿ�
	reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);  // ע���ٽ�������
	reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect); // ע��SPIƬѡ�źź���
	reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte);   // ע���д����
	if (ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1)
	{												  // ���if��������ʾ�շ��ڴ����ʧ��
		u1_printf("��ʼ���շ�����ʧ��,׼������\r\n"); // ��ʾ��Ϣ
		NVIC_SystemReset();							  // ����
	}
	do
	{ // �������״̬
		if (ctlwizchip(CW_GET_PHYLINK, (void *)&tmp) == -1)
		{										 // ���if��������ʾδ֪����
			u1_printf("δ֪����׼������\r\n"); // ��ʾ��Ϣ
			NVIC_SystemReset();					 // ����
		}
		if (tmp == PHY_LINK_OFF)
		{
			u1_printf("����δ����\r\n"); // �����⵽������û���ӣ���ʾ��������
			Delay_Ms(2000);				 // ��ʱ2s
		}
	} while (tmp == PHY_LINK_OFF); // ѭ��ִ�У�ֱ������������

	setSHAR(gWIZNETINFO.mac);									 // ����MAC��ַ
	DHCP_init(SOCK_DHCP, gDATABUF);								 // ��ʼ��DHCP
	reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict); // ע��DHCP�ص�����
	DNS_flag = 0;												 // DNS������־=0
	my_dhcp_retry = 0;											 // DHCP���Դ���=0
	tcp_state = 0;												 // TCP״̬=0
	temp_state = -1;											 // ��һ��TCP״̬=-1
}
/*-------------------------------------------------*/
/*��������W5500��������                            */
/*��  ����data:����                                */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void W5500_TxData(unsigned char *data)
{
	int len;
	len = send(SOCK_TCPS, &data[2], data[0] * 256 + data[1]); // W5500��������
	if (len == (data[0] * 256 + data[1]))
		u1_printf("�������ݳɹ�\r\n");
}
/*-------------------------------------------------*/
/*��������DNS��������                              */
/*��  ����DomainName:����                          */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void W5500_DNS(unsigned char *DomainName)
{
	char ret;

	u1_printf("��ʼ��������\r\n");			   // ���������Ϣ
	DNS_init(SOCKET_DNS, gDNSBUF);			   // DNS��ʼ��
	ret = DNS_run(dns2, DomainName, ServerIP); // ��������
	u1_printf("DNS��������ֵ��%d\r\n", ret);   // �����������
	if (ret == 1)
	{																								   // ����ֵ����1����ʾ������ȷ
		u1_printf("DNS�����ɹ���%d.%d.%d.%d\r\n", ServerIP[0], ServerIP[1], ServerIP[2], ServerIP[3]); // ���������Ϣ
		DNS_flag = 1;																				   // DNS�����ɹ�����־=1
		close(SOCKET_DNS);																			   // �ر�DNS�˿�
	}
}
