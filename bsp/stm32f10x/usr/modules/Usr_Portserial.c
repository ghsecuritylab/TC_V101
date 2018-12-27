#include <rtthread.h>
#include <components.h>
#include <stm32f10x_conf.h>
#include "global_var.h"
#include "sys_conf.h"
#include "Usr_Portserial.h"
#include "Usr_Porttimer.h"
#include "fifo.h"

Comm_st  Comm_PC_inst;
Comm_st* Comm_PC_Protocol;
//U8	g_ComBuff[COM_MAX][MNT_RX_LEN];
Comm_T_st  Comm_T_inst;

static fifo8_cb_td mnt_PC_fifo;
static fifo8_cb_td mnt_T_fifo;

void PortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
		ComParity eParity)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_DeInit(UART5);
	//======================ʱ�ӳ�ʼ��=======================================
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD ,	ENABLE);
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_UART5,ENABLE);
	
	//======================IO��ʼ��=========================================	
	//UART5_TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//UART5_RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	//����485���ͺͽ���ģʽ
	//TODO   ��ʱ��дB13 ��֮����������ʱ���޸�
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	//======================���ڳ�ʼ��=======================================
	USART_InitStructure.USART_BaudRate = ulBaudRate;
	//����У��ģʽ
	switch (eParity)
	{
	case COM_PAR_NONE: //��У��
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		break;
	case COM_PAR_ODD: //��У��
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		break;
	case COM_PAR_EVEN: //żУ��
		USART_InitStructure.USART_Parity = USART_Parity_Even;
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		break;
	default:
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		break;
	}

	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_HardwareFlowControl =	USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  	USART_Init(UART5, &USART_InitStructure);     //��ʼ������Ĵ���     
  	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//�ж�ʹ��
	USART_Cmd(UART5, ENABLE);                    //����ʹ��
	
	//=====================�жϳ�ʼ��======================================
	//����NVIC���ȼ�����ΪGroup2��0-3��ռʽ���ȼ���0-3����Ӧʽ���ȼ�
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	COMM_PC_RECEIVE_MODE;
	fifo8_init(&mnt_PC_fifo,1,MNT_RX_LEN);
	return;	 
}

void Comm_PC_PortSerialPutByte(uint8_t *ucbyte, uint16_t len)
{
		uint8_t tx_data;
		uint16_t i;
	
		for(i=0;i<len;i++)
		{
			fifo8_push(&mnt_PC_fifo,ucbyte);
			ucbyte++;
		}

		USART_ClearFlag(UART5, USART_FLAG_TC);
		COMM_PC_SEND_MODE;
		USART_ITConfig(UART5, USART_IT_RXNE, DISABLE);
		fifo8_pop(&mnt_PC_fifo,&tx_data);
		USART_SendData(UART5, tx_data);
		USART_ITConfig(UART5, USART_IT_TC, ENABLE);
		
}
/***************************************************************************
 * Function Name  : UART5_IRQHandler
 * Description    : This function handles UART5 global interrupt request.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/

void UART5_IRQHandler(void)
{
	uint8_t rec_data;
	uint8_t tx_data;

	rt_interrupt_enter();
	//�������
	if (USART_GetFlagStatus(UART5, USART_FLAG_ORE) == SET)
	{
		rec_data = USART_ReceiveData(UART5);
		//USART_ClearFlag(UART5 USART_FLAG_ORE);
	}
	//�����ж�
	if (USART_GetITStatus(UART5, USART_IT_RXNE) == SET)
	{
			USART_ClearITPendingBit( UART5, USART_IT_RXNE);
			rec_data = USART_ReceiveData(UART5);
			Comm_PC_inst.rx_flag =1;
	  		g_Var_inst.Test[1]=0x5A;
			g_Var_inst.Buff[0]=Comm_PC_inst.rx_flag;
			g_Var_inst.Buff[1]=Comm_PC_inst.rx_ok;
			g_Var_inst.Buff[2]=Comm_PC_inst.rec_state;
    
			if(Comm_PC_inst.rx_ok == 0)
			{
					switch(Comm_PC_inst.rec_state)
					{
						case IDLE_STATE:
							 if(rec_data == FRAME_HEAD)//��ʼ��
							 {
							 	g_Var_inst.Buff[3]=FRAME_HEAD;
							 	Comm_PC_inst.Rec_Checksum=FRAME_HEAD;
								Comm_PC_inst.DataCount = 0;
								Comm_PC_inst.Buffer[Comm_PC_inst.DataCount++] = rec_data;	
								Comm_PC_inst.rec_state ++;
							 }
							 break;
						case ADDRESS_STATE:	//��ַ
							 if(rec_data == Comm_PC_inst.addr)
							 {
								Comm_PC_inst.Rec_Checksum +=rec_data;
								Comm_PC_inst.Buffer[Comm_PC_inst.DataCount++] = rec_data;	
								Comm_PC_inst.rec_state ++;
							 }
							 break;
						case CMD_STATE:	//������
								Comm_PC_inst.Rec_Checksum +=rec_data;
								Comm_PC_inst.Buffer[Comm_PC_inst.DataCount++] = rec_data;	
								Comm_PC_inst.rec_state ++;
							 	break;
						case LENGTH_STATE://�����򳤶�
								Comm_PC_inst.Rec_DataLength =rec_data;
								Comm_PC_inst.Rec_Checksum +=rec_data;
								Comm_PC_inst.Buffer[Comm_PC_inst.DataCount++] = rec_data;	
								if(!Comm_PC_inst.Rec_DataLength)
								{
									Comm_PC_inst.rec_state ++;
								}
								Comm_PC_inst.rec_state ++;
							 	break;
						case DATA_STATE://����
								Comm_PC_inst.Rec_Checksum +=rec_data;
								Comm_PC_inst.Buffer[Comm_PC_inst.DataCount++] = rec_data;	
								Comm_PC_inst.Rec_DataLength--;
								//��������
								if(!Comm_PC_inst.Rec_DataLength)
								{
									Comm_PC_inst.rec_state ++;
								}
							 	break;
						case CS_STATE://У���
								Comm_PC_inst.Buffer[Comm_PC_inst.DataCount++] = rec_data;	
								if(Comm_PC_inst.Rec_Checksum!=rec_data)
								{
									Comm_PC_inst.rec_state =IDLE_STATE;
									return;
								}
								Comm_PC_inst.rec_state ++;
							 	break;
						case END_STATE://������
								Comm_PC_inst.Buffer[Comm_PC_inst.DataCount++] = rec_data;	
								if(FRAME_END == rec_data)
								{
									Comm_PC_inst.rx_ok = 1;
									Comm_PC_inst.rec_state =IDLE_STATE;
								}
							 	break;
						default:
								Comm_PC_inst.rec_state = IDLE_STATE;
								break;
				}
		}
	}
	//�����ж�
	if (USART_GetITStatus(UART5, USART_IT_TC) == SET)
	{
			USART_ClearFlag(UART5, USART_FLAG_TC);
			if(is_fifo8_empty(&mnt_PC_fifo) == 0)
			{
					fifo8_pop(&mnt_PC_fifo,&tx_data);
					USART_SendData(UART5, tx_data);
			}				
			else
			{
					USART_ITConfig(UART5, USART_IT_TC, DISABLE);
					USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
					COMM_PC_RECEIVE_MODE;
			}		
			
	}
	
	rt_interrupt_leave();
}


void Comm_PC_Init( UCHAR ucAddress, UCHAR ucPort, ULONG ulBaudRate, ComParity eParity )
{

    ULONG    usTimerT35_50us;

//	Comm_PC_inst.rec_state = IDLE_STATE;
//	Comm_PC_inst.rx_flag = 0x00;
//	Comm_PC_inst.rx_ok = 0x00;
//	Comm_PC_inst.rx_timeout =0x00;
//	Comm_PC_inst.update_timer_flag = 0x00;
//	Comm_PC_inst.addr = ucAddress;
//	Comm_PC_inst.DataCount = 0x00;
//	Comm_PC_inst.Rec_Checksum =0x00;
//	Comm_PC_inst.Rec_DataLength =0x00;
//	memset(Comm_PC_inst.Buffer,0x00,MNT_RX_LEN);

	memset(&Comm_PC_inst,0x00,sizeof(Comm_PC_inst));
	Comm_PC_inst.rec_state = IDLE_STATE;
	Comm_PC_inst.addr = ucAddress;
	
	Comm_PC_Protocol =&Comm_PC_inst;
	Comm_PC_Protocol->pFrame = (ProtocolFrame*)Comm_PC_inst.Buffer;

	g_Var_inst.Test[1]=0x00;
	memset(g_Var_inst.Buff,0x00,VAR_BUFLEN);
    /* Modbus RTU uses 8 Databits. */
    PortSerialInit( ucPort, ulBaudRate, 8, eParity );
   
	/* If baudrate > 19200 then we should use the fixed timer values
	 * t35 = 1750us. Otherwise t35 must be 3.5 times the character time.
	 */


	if( ulBaudRate > 19200 )
	{
		usTimerT35_50us = 35;       /* 1800us. */
	}
	else
	{
			/* The timer reload value for a character is given by:
			 *
			 * ChTimeValue = Ticks_per_1s / ( Baudrate / 11 )
			 *             = 11 * Ticks_per_1s / Baudrate
			 *             = 220000 / Baudrate
			 * The reload for t3.5 is 1.5 times this value and similary
			 * for t3.5.
			 */
			usTimerT35_50us = ( 7UL * 220000UL ) / ( 2UL * ulBaudRate );
	}

	Comm_PC_PortTimersInit((USHORT)usTimerT35_50us );     
}

/* ----------------------- Start implementation -----------------------------*/
void EnterCriticalSection(void)
{
	//�ر�ȫ���ж�
	__disable_irq();
}

void ExitCriticalSection(void)
{
	//����ȫ���ж�
	__enable_irq();
}




/****************************Comm_T�����԰�ͨ��****************************/

//Ĭ��һ������ ����1 �����ʿ�����  ��ż���������
BOOL PortSerial1_Init(ULONG ulBaudRate, UCHAR ucDataBits,
		ComParity eParity)
{
	GPIO_InitTypeDef GPIO_InitStructure;			
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	//======================ʱ�ӳ�ʼ��=======================================
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//����ʱ��
	//======================IO��ʼ��=======================================	
	//USART1_TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//USART1_RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//����485���ͺͽ���ģʽ
//    TODO   ��ʱ��дA0 ��֮����������ʱ���޸�
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//ʹ����ӳ�书��
	GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE);
	//======================���ڳ�ʼ��=======================================
	USART_DeInit(USART1);
	USART_InitStructure.USART_BaudRate = ulBaudRate;
	//����У��ģʽ
	switch (eParity)
	{
		case COM_PAR_NONE: //��У��
			USART_InitStructure.USART_Parity = USART_Parity_No;
			USART_InitStructure.USART_WordLength = USART_WordLength_8b;
			break;
		case COM_PAR_ODD: //��У��
			USART_InitStructure.USART_Parity = USART_Parity_Odd;
			USART_InitStructure.USART_WordLength = USART_WordLength_9b;
			break;
		case COM_PAR_EVEN: //żУ��
			USART_InitStructure.USART_Parity = USART_Parity_Even;
			USART_InitStructure.USART_WordLength = USART_WordLength_9b;
			break;
		default:
			return FALSE;
	}

	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_HardwareFlowControl =	USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART1, ENABLE);

	//=====================�жϳ�ʼ��======================================
	//����NVIC���ȼ�����ΪGroup2��0-3��ռʽ���ȼ���0-3����Ӧʽ���ȼ�
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

//	EXIT_CRITICAL_SECTION(); //��ȫ���ж�
	COMM_T_RECEIVE_MODE;
	fifo8_init(&mnt_T_fifo,1,MNT_RX_LEN);
	return TRUE;
}



void Comm_T_Init( UCHAR ucAddress, UCHAR ucPort, ULONG ulBaudRate, ComParity eParity )
{

    ULONG    usTimerT35_50us;

		memset(&Comm_T_inst,0x00,sizeof(Comm_T_inst));
		Comm_T_inst.Rec_state = REC_ADDR_STATE;
		Comm_T_inst.Addr = ucAddress;	

    /* Modbus RTU uses 8 Databits. */
    PortSerial1_Init(ulBaudRate, 8, eParity );
   
	/* If baudrate > 19200 then we should use the fixed timer values
	 * t35 = 1750us. Otherwise t35 must be 3.5 times the character time.
	 */

	if( ulBaudRate > 19200 )
	{
		usTimerT35_50us = 35;       /* 1800us. */
	}
	else
	{
			/* The timer reload value for a character is given by:
			 *
			 * ChTimeValue = Ticks_per_1s / ( Baudrate / 11 )
			 *             = 11 * Ticks_per_1s / Baudrate
			 *             = 220000 / Baudrate
			 * The reload for t3.5 is 1.5 times this value and similary
			 * for t3.5.
			 */
			usTimerT35_50us = ( 7UL * 220000UL ) / ( 2UL * ulBaudRate );
	}

	Comm_T_PortTimersInit((USHORT)usTimerT35_50us );     
}

//�����������fifo
void Comm_T_PortSerialPutByte(uint8_t *ucbyte, uint16_t len)
{
		uint8_t tx_data;
		uint16_t i;

		for(i=0;i<len;i++)
		{	
			fifo8_push(&mnt_T_fifo,ucbyte);
			ucbyte++;
		}
		USART_ClearFlag(USART1, USART_FLAG_TC);
		COMM_T_SEND_MODE;
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
		fifo8_pop(&mnt_T_fifo,&tx_data);
		USART_SendData(USART1, tx_data);
		USART_ITConfig(USART1, USART_IT_TC, ENABLE);
		
}

/***************************************************************************
 * Function Name  : UART1_IRQHandler
 * Description    : This function handles UART5 global interrupt request.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/

void USART1_IRQHandler(void)
{
	uint8_t rec_data;
	uint8_t tx_data;

	rt_interrupt_enter();
	//�������
	if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) == SET)
	{
		rec_data = USART_ReceiveData(USART1);
		//USART_ClearFlag(UART5 USART_FLAG_ORE);
	}
	//�����ж�
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
			USART_ClearITPendingBit( USART1, USART_IT_RXNE);
			rec_data = USART_ReceiveData(USART1);
			if(Comm_T_inst.Rx_ok == 0)
			{
					switch(Comm_T_inst.Rec_state)
					{
						case REC_ADDR_STATE:	//��ַ
								g_Var_inst.Test[0]=0x8000;
							 if(rec_data == Comm_T_inst.Addr)
							 {
								Comm_T_inst.DataCount=0x00;
								Comm_T_inst.Buffer[Comm_T_inst.DataCount++] = rec_data;	
								Comm_T_inst.Rec_state ++;
							 }
							 break;
						case REC_CMD_STATE:	//������
								g_Var_inst.Test[0]|=0x01;
								Comm_T_inst.Buffer[Comm_T_inst.DataCount++] = rec_data;
								switch(rec_data)
								{
									case MB_READ:
										Comm_T_inst.Rec_state ++;	
										break;
									case MB_READ_ERR:
									case MB_WRITE_SINGLE_ERR:
										Comm_T_inst.DataLength =1;
										Comm_T_inst.Rec_state =REC_DATA_STATE;	
										break;
									case MB_WRITE_SINGLE:
										Comm_T_inst.DataLength =4;
										Comm_T_inst.Rec_state =REC_DATA_STATE;	
										break;
									default:
										Comm_PC_inst.DataCount=0x00;
										Comm_T_inst.Rec_state =REC_ADDR_STATE;
										break;
								}
							 	break;
						case REC_LENGTH_STATE://�����򳤶�
								g_Var_inst.Test[0]|=0x02;
								Comm_T_inst.DataLength =rec_data;
								Comm_T_inst.Buffer[Comm_T_inst.DataCount++] = rec_data;	
								Comm_T_inst.Rec_state ++;
							 	break;
						case REC_DATA_STATE://����
								g_Var_inst.Test[0]|=0x04;
								Comm_T_inst.Buffer[Comm_T_inst.DataCount++] = rec_data;	
								Comm_T_inst.DataLength--;
								//��������
								if(!Comm_T_inst.DataLength)
								{
									Comm_T_inst.DataLength=2;  //2�ֽ�CRCУ��
									Comm_T_inst.Rec_state ++;
								}
							 	break;
						case REC_CHECK_STATE://У��
								g_Var_inst.Test[0]|=0x08;
								Comm_T_inst.Buffer[Comm_T_inst.DataCount++] = rec_data;	
								Comm_T_inst.DataLength--;
								//��������
								if(!Comm_T_inst.DataLength)
								{
									g_Var_inst.Test[0]|=0x10;
									Comm_T_inst.Rx_ok = 1;
									Comm_T_inst.Rec_state =REC_ADDR_STATE;
								}
							 	break;
						default:
								g_Var_inst.Test[0]|=0x100;
								Comm_T_inst.Rec_state = REC_ADDR_STATE;
								break;
				}
		}
	}
	//�����ж�
	if (USART_GetITStatus(USART1, USART_IT_TC) == SET)
	{
			USART_ClearFlag(USART1, USART_FLAG_TC);
			if(is_fifo8_empty(&mnt_T_fifo) == 0)
			{
					fifo8_pop(&mnt_T_fifo,&tx_data);
					USART_SendData(USART1, tx_data);
			}				
			else
			{
					USART_ITConfig(USART1, USART_IT_TC, DISABLE);
					USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
					COMM_T_RECEIVE_MODE;
			}		
			
	}
	
	rt_interrupt_leave();
}



