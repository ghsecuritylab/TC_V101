#include <stm32f10x_conf.h>
#include "sys_conf.h"
#include "Usr_Porttimer.h"
#include "Usr_Portserial.h"
#include <rtdevice.h>


void Comm_PC_PortTimersInit(USHORT usTim1Timerout50us)
{

	uint16_t PrescalerValue = 0;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	//====================================ʱ�ӳ�ʼ��===========================
	//ʹ�ܶ�ʱ��3ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	//====================================��ʱ����ʼ��===========================
	//��ʱ��ʱ�������˵��
	//HCLKΪ72MHz��APB1����2��ƵΪ36MHz
	//TIM3��ʱ�ӱ�Ƶ��Ϊ72MHz��Ӳ���Զ���Ƶ,�ﵽ���
	//TIM3�ķ�Ƶϵ��Ϊ3599��ʱ���Ƶ��Ϊ72 / (1 + Prescaler) = 20KHz,��׼Ϊ50us
	//TIM������ֵΪusTim1Timerout50u
	
	PrescalerValue = (uint16_t) (SystemCoreClock / 20000) - 1;
	//��ʱ��1��ʼ��
	TIM_TimeBaseStructure.TIM_Period = (uint16_t) usTim1Timerout50us;
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
	//Ԥװ��ʹ��
	TIM_ARRPreloadConfig(TIM5, ENABLE);
	//====================================�жϳ�ʼ��===========================
	//����NVIC���ȼ�����ΪGroup2��0-3��ռʽ���ȼ���0-3����Ӧʽ���ȼ�
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
//	//�������жϱ�־λ
//	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
//	//��ʱ��3����жϹر�
//	TIM_ITConfig(TIM5, TIM_IT_Update, DISABLE);
//	//��ʱ��3����
//	TIM_Cmd(TIM5 ,DISABLE);
	
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM5, 0);
	TIM_Cmd(TIM5, DISABLE);
	return;
}

void Comm_PC_PortTimersEnable(void)
{
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM5, 0);
	TIM_Cmd(TIM5, ENABLE);
}

void Comm_PC_PortTimersDisable(void)
{
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
	TIM_ITConfig(TIM5, TIM_IT_Update, DISABLE);
	TIM_SetCounter(TIM5, 0);
	TIM_Cmd(TIM5, DISABLE);
}



void TIM5_IRQHandler(void)
{
	rt_interrupt_enter();
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
	{
		
		TIM_ClearFlag(TIM5, TIM_FLAG_Update);	     		//���жϱ��
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);		//�����ʱ��T5����жϱ�־λ
		Comm_PC_inst.rx_timeout = 1;
	}
	rt_interrupt_leave();
}



BOOL Comm_T_PortTimersInit(USHORT usTimeOut50us)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	static USHORT usT35TimeOut50us;
	static USHORT usPrescalerValue = 0;

	//====================================ʱ�ӳ�ʼ��===========================
	//ʹ�ܶ�ʱ��2ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	//====================================��ʱ����ʼ��===========================
	//��ʱ��ʱ�������˵��
	//HCLKΪ72MHz��APB1����2��ƵΪ36MHz
	//TIM2��ʱ�ӱ�Ƶ��Ϊ72MHz��Ӳ���Զ���Ƶ,�ﵽ���
	//TIM2�ķ�Ƶϵ��Ϊ3599��ʱ���Ƶ��Ϊ72 / (1 + Prescaler) = 20KHz,��׼Ϊ50us
	//TIM������ֵΪusTim1Timerout50u	
	usPrescalerValue = (uint16_t) (SystemCoreClock / 20000) - 1;
	//����T35��ʱ������ֵ
	usT35TimeOut50us = usTimeOut50us; 

	TIM_TimeBaseStructure.TIM_Prescaler = usPrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = (uint16_t) usT35TimeOut50us;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	//Ԥװ��ʹ��
	TIM_ARRPreloadConfig(TIM2, ENABLE);
	//====================================�жϳ�ʼ��===========================
	//����NVIC���ȼ�����ΪGroup2��0-3��ռʽ���ȼ���0-3����Ӧʽ���ȼ�
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
//	//�������жϱ�־λ
//	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//	//��ʱ��3����жϹر�
//	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
//	//��ʱ��3����
//	TIM_Cmd(TIM2, DISABLE);

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM2, 0);
	TIM_Cmd(TIM2, ENABLE);

	return TRUE;
}

void Comm_T_PortTimersEnable(void)
{
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM2, 0);
	TIM_Cmd(TIM2, ENABLE);
}

void Comm_T_PortTimersDisable(void)
{
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
	TIM_SetCounter(TIM2, 0);
	TIM_Cmd(TIM2, DISABLE);
}

void TIM2_IRQHandler(void)
{
	rt_interrupt_enter();
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		
		TIM_ClearFlag(TIM2, TIM_FLAG_Update);	     //���жϱ��
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	 //�����ʱ��TIM2����жϱ�־λ
//		prvvTIMERExpiredISR();
	}
	rt_interrupt_leave();
}


