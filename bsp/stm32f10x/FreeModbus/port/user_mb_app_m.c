/*
 * FreeModbus Libary: user callback functions and buffer define in master mode
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: user_mb_app_m.c,v 1.60 2013/11/23 11:49:05 Armink $
 */
#include "user_mb_app.h"

#define MBM_RESPONSE_DELAY	30
#define	MBM_QUEST_DELAY			30

static uint16_t mbm_dev_poll(uint16_t mb_comp_mask, mbm_dev_st* mbm_dev_inst);
static uint16_t mbm_dev_init(mbm_dev_st* mbm_dev_inst);
static uint16_t mbm_reg_update(mbm_dev_st* mbm_dev_inst);
static void mbm_fsm_init(mbm_dev_st* mbm_dev_inst);
static void mbm_fsm_update(sys_reg_st*	gds_ptr,mbm_dev_st* mbm_dev_inst);


/*-----------------------Master mode use these variables----------------------*/
//Master mode:HoldingRegister variables
static uint16_t   usMRegHoldStart                         = M_REG_HOLDING_START;
static uint16_t   usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS];
static mbm_dev_st mbm_dev_inst;


/**
  * @brief  modbus master poll thread
  * @param  none
  * @retval none
**/
void modbus_master_thread_entry(void* parameter)
{
		eMBErrorCode    eStatus = MB_ENOERR;
		rt_thread_delay(MODBUS_MASTER_THREAD_DELAY);
		eStatus = eMBMasterInit(MB_RTU,2,4800,MB_PAR_NONE);
		if(eStatus != MB_ENOERR)
		{
				rt_kprintf("MBM init fail\n");
		}
		eStatus = eMBMasterEnable();
		if(eStatus != MB_ENOERR)
		{
				rt_kprintf("MBM enable fail\n");
		}
		while(1)
		{ 
				eStatus = eMBMasterPoll();	
				if(eStatus != MB_ENOERR)
				{
						rt_kprintf("MBM poll err!\n");
				}
				rt_thread_delay(10);
		}
}

void mbm_fsm_thread_entry(void* parameter)
{	
		extern sys_reg_st	g_sys; 
		rt_thread_delay(MBM_FSM_THREAD_DELAY);
		mbm_fsm_init(&mbm_dev_inst);								//initialize local modbus master register set
		while(1)
		{
				mbm_fsm_update(&g_sys,&mbm_dev_inst);		//update modbus slave components into local modbus master register
				rt_thread_delay(1000);
		}
}

//******************************���ּĴ����ص�����**********************************
//��������: eMBErrorCode eMBMasterRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
//��    �������ּĴ�����صĹ��ܣ�������������д������д��
//��ڲ�����pucRegBuffer : �����Ҫ�����û��Ĵ�����ֵ���������������ָ���µļĴ�����ֵ��
//                         ���Э��ջ��֪����ǰ����ֵ���ص��������뽫��ǰֵд�����������
//					usAddress    : �Ĵ�������ʼ��ַ��
//					usNRegs      : �Ĵ�������
//          eMode        : ����ò���ΪeMBRegisterMode::MB_REG_WRITE���û���Ӧ����ֵ����pucRegBuffer�еõ����¡�
//                         ����ò���ΪeMBRegisterMode::MB_REG_READ���û���Ҫ����ǰ��Ӧ�����ݴ洢��pucRegBuffer��
//���ڲ�����eMBErrorCode : ������������صĴ�����
//��    ע��Editor��Armink 2013-11-25    Company: BXXJS
//**********************************************************************************
eMBErrorCode
eMBMasterRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    uint16_t          iRegIndex;
    uint16_t *        pusRegHoldingBuf;
    uint16_t          REG_HOLDING_START;
    uint16_t          REG_HOLDING_NREGS;
    uint16_t          usRegHoldStart;

	pusRegHoldingBuf = usMRegHoldBuf[ucMBMasterGetDestAddress() - 1];
	REG_HOLDING_START = M_REG_HOLDING_START;
	REG_HOLDING_NREGS = M_REG_HOLDING_NREGS;
	usRegHoldStart = usMRegHoldStart;
	//If mode is read,the master will wirte the received date to bufffer.
	eMode = MB_REG_WRITE;	
	usAddress--;//FreeModbus���ܺ������Ѿ���1��Ϊ��֤�뻺�����׵�ַһ�£��ʼ�1
    if( ( usAddress >= REG_HOLDING_START ) &&
        ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {
        iRegIndex = usAddress - usRegHoldStart;
        switch ( eMode )
        {
            /* Pass current register values to the protocol stack. */
        case MB_REG_READ:
            while( usNRegs > 0 )
            {
								*pucRegBuffer++ = ( unsigned char )( pusRegHoldingBuf[iRegIndex] >> 8 );
								*pucRegBuffer++ = ( unsigned char )( pusRegHoldingBuf[iRegIndex] & 0xFF );
                iRegIndex++;
                usNRegs--;
            }
            break;

            /* Update current register values with new values from the
             * protocol stack. */
        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
            break;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/**
  * @brief  poll modbus master slave device, if exist, set dev_mask bitmap accordingly
  * @param  
			@mb_comp_mask: system modbus slave device bitmap configuration.
			@mbm_dev_inst: modbus master device data struct.
  * @retval 
			@arg 1: all device online
			@arg 0: not all device online
  */
static uint16_t mbm_dev_poll(uint16_t mb_comp_mask, mbm_dev_st* mbm_dev_inst)
{
		eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
		uint16_t 	i;
		uint16_t	dev_poll_bitmap_reg;
		uint16_t 	xor_bitmap;
	
		dev_poll_bitmap_reg = mbm_dev_inst->bitmap.poll;
		xor_bitmap = mb_comp_mask^dev_poll_bitmap_reg;
		dev_poll_bitmap_reg = 0;
		if(xor_bitmap == 0)		//if default bitmap equals to online bitmap, means all device are online, return ture
		{
				return 1;
		}
		else
		{
				for(i=0;i<MB_MASTER_TOTAL_SLAVE_NUM;i++)
				{
						if(((xor_bitmap>>i)&0x0001) == 1)
						{
								errorCode = eMBMasterReqReadHoldingRegister((i+1),0,1,MBM_RESPONSE_DELAY);
								if(errorCode == MB_MRE_NO_ERR)
								{
										dev_poll_bitmap_reg |= (0x0001<<i);		//set online flag
								}
								rt_thread_delay(MBM_QUEST_DELAY);
						}						
				}
		
				mbm_dev_inst->bitmap.poll = dev_poll_bitmap_reg;
				if(dev_poll_bitmap_reg == mb_comp_mask)
				{
						mbm_dev_inst->timeout.poll = 0;
						return 1;
				}
				else
				{
						mbm_dev_inst->errcnt.poll++;
						mbm_dev_inst->timeout.poll++;						
						return 0;
				}
		}
}

/**
  * @brief  initialize modbus master slave device registers
  * @param  
			@mbm_dev_inst: modbus master device data struct.
  * @retval 
			@arg 1: all device online
			@arg 0: not all device online
  */
static uint16_t mbm_dev_init(mbm_dev_st* mbm_dev_inst)
{
		eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
		uint16_t dev_poll_bitmap_reg;
		uint16_t dev_init_bitmap_reg;
		uint16_t dev_reg_cnt;
		uint16_t xor_bitmap;
		uint16_t i;		

		dev_poll_bitmap_reg = mbm_dev_inst->bitmap.poll;
		dev_init_bitmap_reg = mbm_dev_inst->bitmap.init;	
		xor_bitmap = dev_poll_bitmap_reg^dev_init_bitmap_reg;

		if(xor_bitmap == 0)		//if default bitmap equals to online bitmap, means all device are online, return ture
		{
				return 1;
		}
		else
		{
				for(i=0;i<MB_MASTER_TOTAL_SLAVE_NUM;i++)
				{
						if(((xor_bitmap>>i)&0x0001) == 1)
						{
								dev_reg_cnt = usMRegHoldBuf[i][MBM_REG_ADDR_CNT_ADDR];
								errorCode = eMBMasterReqReadHoldingRegister((i+1),0,dev_reg_cnt,MBM_RESPONSE_DELAY);
								if(errorCode == MB_MRE_NO_ERR)
								{
										dev_init_bitmap_reg |= (0x0001<<i);
								}
								rt_thread_delay(MBM_QUEST_DELAY);
						}						
				}
				
				mbm_dev_inst->bitmap.init = dev_init_bitmap_reg;
				if(dev_init_bitmap_reg == dev_poll_bitmap_reg)
				{
						mbm_dev_inst->timeout.init = 0;
						return 1;
				}
				else
				{
						mbm_dev_inst->errcnt.init++;
						mbm_dev_inst->timeout.init++;
						if(mbm_dev_inst->timeout.init >= MBM_INIT_TIMEOUT_THRESHOLD)
						{
								mbm_dev_inst->bitmap.poll = dev_init_bitmap_reg;
						}					
						return 0;
				}
		}
}

/**
  * @brief  update local modbus master register map with only variable device reg values
  * @param  
			@mbm_dev_inst: modbus master device data struct.
  * @retval 
			@arg 1: all device online
			@arg 0: not all device online
  */
static uint16_t mbm_reg_update(mbm_dev_st* mbm_dev_inst)
{
		eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
		uint16_t dev_init_bitmap_reg;
		uint16_t dev_update_bitmap_reg;
		uint16_t dev_reg_cnt;
		uint16_t i;
		
		dev_init_bitmap_reg = mbm_dev_inst->bitmap.init;
		dev_update_bitmap_reg = 0;		
		
		for(i=0;i<MB_MASTER_TOTAL_SLAVE_NUM;i++)
		{
				if(((dev_init_bitmap_reg>>i)&0x0001) == 1)
				{
						dev_reg_cnt = usMRegHoldBuf[i][MBM_REG_ADDR_CNT_ADDR] - M_REG_HOLDING_USR_START;
						errorCode = eMBMasterReqReadHoldingRegister((i+1),M_REG_HOLDING_USR_START,dev_reg_cnt,MBM_RESPONSE_DELAY);
						if(errorCode == MB_MRE_NO_ERR)
						{
								dev_update_bitmap_reg |= (0x0001<<i);
						}
						rt_thread_delay(MBM_QUEST_DELAY);
				}				
		}		
		mbm_dev_inst->bitmap.update = dev_update_bitmap_reg;
		if(dev_update_bitmap_reg == mbm_dev_inst->bitmap.init)
		{
				mbm_dev_inst->timeout.update = 0;
				return 1;
		}
		else
		{
				mbm_dev_inst->timeout.update++;
				mbm_dev_inst->errcnt.update++;
				if(mbm_dev_inst->timeout.update >= MBM_UPDATE_TIMEOUT_THRESHOLD)
				{
						mbm_dev_inst->bitmap.init = dev_update_bitmap_reg;
				}
				return 0;
		}		
}

/**
  * @brief  update local modbus master register map with only variable device reg values(ie. reg addr after 20)
  * @param  mbm_dev_inst: modbus master device data struct.
  * @retval none
  */
static void mbm_fsm_update(sys_reg_st*	gds_ptr,mbm_dev_st* mbm_dev_inst)
{
		uint16_t mbm_fsm_cstate;
		mbm_fsm_cstate = mbm_dev_inst->mbm_fsm;
		switch (mbm_fsm_cstate)
		{
				case (MBM_FSM_IDLE):
				{
						mbm_dev_poll(gds_ptr->config.dev_mask.mb_comp, mbm_dev_inst);
						mbm_dev_init(mbm_dev_inst);
						mbm_dev_inst->mbm_fsm = MBM_FSM_SYNC;
						break;
				}
				case (MBM_FSM_SYNC):
				{
						mbm_dev_poll(gds_ptr->config.dev_mask.mb_comp, mbm_dev_inst);
						mbm_dev_init(mbm_dev_inst);
						mbm_reg_update(mbm_dev_inst);
						if(((mbm_dev_inst->bitmap.update)^(gds_ptr->config.dev_mask.mb_comp)) == 0)	//if init succeeded, go into update state, otherwise remain sync state
						{
								mbm_dev_inst->mbm_fsm = MBM_FSM_UPDATE;					
						}
						else
						{
								mbm_dev_inst->mbm_fsm = MBM_FSM_SYNC;
						}
						break;
				}
				case (MBM_FSM_UPDATE):
				{
						mbm_reg_update(mbm_dev_inst);						
						if((mbm_dev_inst->timeout.update >= MBM_UPDATE_TIMEOUT_THRESHOLD)||(gds_ptr->config.dev_mask.mb_comp != mbm_dev_inst->bitmap.update))	//if update err count timeout, swich to sync state
						{
								mbm_dev_inst->bitmap.poll = mbm_dev_inst->bitmap.update;
								mbm_dev_inst->mbm_fsm = MBM_FSM_SYNC;
						}
						else
						{
								mbm_dev_inst->mbm_fsm = MBM_FSM_UPDATE;
						}
						break;
				}			
				default:
				{
						mbm_dev_inst->mbm_fsm = MBM_FSM_IDLE;
						break;
				}						
		}
		gds_ptr->status.status_remap[MBM_COM_STS_REG_NO] = mbm_dev_inst->bitmap.update;			
}

/**
  * @brief  modbus local data structure initialization
  * @param  mbm_dev_inst: modbus master device data struct.
  * @retval none
  */
static void mbm_fsm_init(mbm_dev_st* mbm_dev_inst)
{
		mbm_dev_inst->bitmap.poll = 0;
		mbm_dev_inst->bitmap.init = 0;
		mbm_dev_inst->bitmap.update = 0;
		mbm_dev_inst->timeout.poll = 0;
		mbm_dev_inst->timeout.init = 0;
		mbm_dev_inst->timeout.update = 0;
		mbm_dev_inst->errcnt.poll = 0;
		mbm_dev_inst->errcnt.init = 0;
		mbm_dev_inst->errcnt.update = 0;
		mbm_dev_inst->mbm_fsm = MBM_FSM_IDLE;
}

/**
  * @brief  modbus module interface, update global register with designated local modbus register values
  * @param  gds_ptr		global register struct pointer
  * @retval none
**/	
void mbm_sts_update(sys_reg_st* gds_ptr)
{
		uint16_t mbm_dev_update_bitmap;
		uint16_t mbm_dev_init_bitmap;
		uint16_t i;
		mbm_dev_init_bitmap = mbm_dev_inst.bitmap.init;

		mbm_dev_update_bitmap = mbm_dev_inst.bitmap.update;	//get modbus update bitmap which could be used to determin which glabal regsiter to update 
		mbm_dev_init_bitmap = mbm_dev_inst.bitmap.init;	//get modbus init bitmap which could be used to determin which glabal regsiter to update 
		
		if(((mbm_dev_init_bitmap >> MBM_DEV_H_ADDR)&0x0001) != 0)
		{
				if(((mbm_dev_update_bitmap >> MBM_DEV_H_ADDR)&0x0001) != 0)	//copy modbus master device register to global register, humidifier daq
				{	
					  //�м�ʪ������עˮ������²�����
						if((gds_ptr->status.dout_bitmap&(0x01<<DO_FILL_BPOS))&&(gds_ptr->status.dout_bitmap&(0x01<<DO_HUM_BPOS)))
						{
								;
						}
						else
						{
								
								gds_ptr->status.mbm.hum.conductivity = usMRegHoldBuf[MBM_DEV_H_ADDR][MBM_DEV_H_REG_CONDUCT_ADDR];
								
						}
						gds_ptr->status.mbm.hum.dev_sts = usMRegHoldBuf[MBM_DEV_H_ADDR][MBM_DEV_H_REG_STATUS_ADDR];
						gds_ptr->status.mbm.hum.hum_current = usMRegHoldBuf[MBM_DEV_H_ADDR][MBM_DEV_H_REG_HUMCUR_ADDR];
						gds_ptr->status.mbm.hum.water_level = usMRegHoldBuf[MBM_DEV_H_ADDR][MBM_DEV_H_REG_WT_LV_ADDR];
				}
		}
		else	//if device is not initialized, all date reset to 0
		{
				gds_ptr->status.mbm.hum.dev_sts = 0;
				gds_ptr->status.mbm.hum.conductivity = 0;
				gds_ptr->status.mbm.hum.hum_current = 0;
				gds_ptr->status.mbm.hum.water_level = 0;
		}

		if(((mbm_dev_init_bitmap >> MBM_DEV_P_ADDR)&0x0001) != 0)
		{		
				if(((mbm_dev_update_bitmap >> MBM_DEV_P_ADDR)&0x0001) != 0)	//copy modbus master device register to global register, power daq
				{		
						gds_ptr->status.mbm.pwr.dev_sts = usMRegHoldBuf[MBM_DEV_P_ADDR][MBM_DEV_H_REG_P0WER_STATUS_ADDR];
						gds_ptr->status.mbm.pwr.pa_volt = usMRegHoldBuf[MBM_DEV_P_ADDR][MBM_DEV_H_REG_PA_VOLT_ADDR];
						gds_ptr->status.mbm.pwr.pb_volt =	usMRegHoldBuf[MBM_DEV_P_ADDR][MBM_DEV_H_REG_PB_VOLT_ADDR];
						gds_ptr->status.mbm.pwr.pc_volt =	usMRegHoldBuf[MBM_DEV_P_ADDR][MBM_DEV_H_REG_PC_VOLT_ADDR];
						gds_ptr->status.mbm.pwr.freq = usMRegHoldBuf[MBM_DEV_P_ADDR][MBM_DEV_H_REG_FREQ_ADDR];
						gds_ptr->status.mbm.pwr.pe_bitmap =	usMRegHoldBuf[MBM_DEV_P_ADDR][MBM_DEV_H_REG_PE_ADDR];	
				}
		}
		else	//if device is not initialized, all date reset to 0
		{
				gds_ptr->status.mbm.pwr.dev_sts = 0;
				gds_ptr->status.mbm.pwr.pa_volt = 0;
				gds_ptr->status.mbm.pwr.pb_volt =	0;
				gds_ptr->status.mbm.pwr.pc_volt =	0;
				gds_ptr->status.mbm.pwr.freq = 0;
				gds_ptr->status.mbm.pwr.pe_bitmap =	0;
		}
				
		for(i=0;i<TEMP_HUM_SENSOR_NUM;i++)	//copy modbus master device register to global register, temp and hum sensor daq
		{
				if(((mbm_dev_init_bitmap >> i)&0x0001) != 0)
				{
						if(((mbm_dev_update_bitmap >> i)&0x0001) != 0)
						{	
								gds_ptr->status.mbm.tnh[i].dev_sts = usMRegHoldBuf[i][MBM_DEV_H_REG_HT_STATUS_ADDR];
								gds_ptr->status.mbm.tnh[i].temp = usMRegHoldBuf[i][MBM_DEV_H_REG_TEMP_ADDR]+(int16_t)(gds_ptr->config.general.temp_sensor_cali[i].temp);
								gds_ptr->status.mbm.tnh[i].hum = usMRegHoldBuf[i][MBM_DEV_H_REG_HUM_ADDR]+(int16_t)(gds_ptr->config.general.temp_sensor_cali[i].hum);
						}
					
				}
				else //if device is not initialized, all date reset to 0
				{
						gds_ptr->status.mbm.tnh[i].dev_sts = 0;
						gds_ptr->status.mbm.tnh[i].temp = 0;
						gds_ptr->status.mbm.tnh[i].hum = 0;
				}
		}
}

