#include <rtthread.h>
#include <components.h>
#include <stddef.h>
#include "global_var.h"
#include "sys_conf.h"
#include "Usr_Protocol.h"
#include "Kits_Memory.h"

/******************PCͨ��**************************************/

Tab_DI  DI_Variable[]=//�������ݱ�ʶ
{
//	{0x01,0x01,0x00,2,OFFSET(Var_Info,HardWare)},//
	{0x01,0x01,0x00,2,offsetof(Var_Info,HardWare)},//
	{0x01,0x01,0x01,2,offsetof(Var_Info,SoftWare)},//
	{0x11,0x02,0x00,6,offsetof(Var_Info,DO)},//
	{0x11,0x03,0x00,1,offsetof(Var_Info,T_Address)},//
	{0x11,0x03,0x01,2,offsetof(Var_Info,T_Baudrate)},//
	{0x11,0x04,0x00,1,offsetof(Var_Info,PC_Address)},//
	{0x11,0x04,0x01,2,offsetof(Var_Info,PC_Baudrate)},//

	//��ֹ���������ѭ��
	{0x00,0x00,0x00,0x00,0x00},//��ֹ��ѭ��

};

//����DI
BOOL CheckDI(ProtocolDIInfo* pDI)
{
    const Tab_DI *DI_VAR;
	// ָ�����ݴ��λ��
	pDI->MomeryType = MEMORY_RAM;
//	pDI->pfnWriteSuccessed = NULL;

	DI_VAR=DI_Variable;

	while(DI_VAR->LEN)
	{
		if((pDI->DI.Bytes[1]==DI_VAR->DI1)&&(pDI->DI.Bytes[0]==DI_VAR->DI0))//����DI1��DI0��
		{			
			// ��������
			if(pDI->Type != TYPE_R)	  //������		 
			{
				if(DI_VAR->DIType==0x01)//ֻ��
				{
					return FALSE;
				}
			}
			pDI->Length = DI_VAR->LEN;
			pDI->Address = DI_VAR->DataAddr;
			return TRUE;
		}
		DI_VAR++;
	}
	return FALSE;
}
//��������
BOOL Checkout(ProtocolDIInfo* pDI,U8* pBuffer)
{
	U32 Address;
	U8 Length;
	U32 Ptr;
 	
	Address =pDI->Address;
	Length =pDI->Length;

	if(pDI->OpsType == OPS_READ)
	{
			rt_kprintf("pBuffer: 0x%02x 0x%02x 0x%02x 0x%02x\n", pBuffer[0],pBuffer[1],pBuffer[2],pBuffer[3]);

		// ������
		if(pDI->MomeryType == MEMORY_RAM)
		{
			Ptr=(U32)&g_Var_inst;	//����������ʼ��ַ
			Ptr+=(U8)Address;
			memcpy(pBuffer,(U8*)Ptr,Length);
			return TRUE;
		}
	}
	else if(pDI->OpsType == OPS_WRITE)
	{
		// д����
		if(pDI->MomeryType == MEMORY_RAM)
		{
			Ptr=(U32)&g_Var_inst;	//����������ʼ��ַ
			Ptr+=(U8)Address;
			memcpy((U8*)Ptr,pBuffer,Length);
			g_Var_inst.DO[6]|=DO_UPDATE;
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL Handle_CMD_READ(Comm_st* ProtocolFrame)
{
	ProtocolDIInfo pDIInfo;

				g_Var_inst.Test[1]=4;
	    	rt_kprintf("g_Var_inst.Test[1]: 0x%02x \n", g_Var_inst.Test[1]);
	// ����Ĭ��Ϊδ֪����
	ProtocolFrame->ProtocolHandleStatus = 0;

	// ������ݳ����Ƿ����������ֵ����ݳ���(�����������ָ�ʽ,���ȷֱ�Ϊ1,2,4)
	if(!((ProtocolFrame->pFrame->Length == 0x01) || \
			(ProtocolFrame->pFrame->Length == 0x02) ||\
			(ProtocolFrame->pFrame->Length == 0x04)))
	{
		// ���ݳ��Ȳ���ȷ,���ز���ʶ�������
		ProtocolFrame->ProtocolHandleStatus |= PROTOCOL_HANDLE_UNDEFINE;
		return TRUE;
	}
					g_Var_inst.Test[1]=5;
	    	rt_kprintf("g_Var_inst.Test[1]: 0x%02x \n", g_Var_inst.Test[1]);
	// �������ݱ�ʶ
	ReverseCopy(pDIInfo.DI.Bytes,ProtocolFrame->pFrame->Data,2);
		    	rt_kprintf("pDIInfo.DI.Bytes: 0x%02x 0x%02x \n", pDIInfo.DI.Bytes[0],pDIInfo.DI.Bytes[1]);
	pDIInfo.Type = TYPE_R;	 //������
	// Check ���ݱ�ʶ
	if(!CheckDI(&pDIInfo))
	{
		// ���ز�֧�ֵ����ݱ�ʶ
		ProtocolFrame->ProtocolHandleStatus |= PROTOCOL_HANDLE_UNDEFINE;
		return TRUE;
	}
					g_Var_inst.Test[1]=6;
	    	rt_kprintf("g_Var_inst.Test[1]: 0x%02x \n", g_Var_inst.Test[1]);
	// ����Check�������ݱ�ʶ��Ӧ��������Ϣ,�齨���ݰ�(���ݳ��ȳ���)
	ProtocolFrame->pFrame->Length += pDIInfo.Length;

	// ����������Ϣ��ȡ����
	pDIInfo.OpsType = OPS_READ;
	if(!Checkout(&pDIInfo,(ProtocolFrame->pFrame->Data)+2))
	{
		// ��ȡ����ʧ��,����δ֪����
		ProtocolFrame->ProtocolHandleStatus |= PROTOCOL_HANDLE_OTHER_ERROR;
		return TRUE;
	}
					g_Var_inst.Test[1]=7;
	    	rt_kprintf("g_Var_inst.Test[1]: 0x%02x \n", g_Var_inst.Test[1]);
	// ���������ɹ�
	ProtocolFrame->ProtocolHandleStatus |= PROTOCOL_HANDLE_SUCCEED;
    return TRUE;

}

// д��������
static void Handle_CMD_Write(Comm_st* ProtocolFrame)
{
	ProtocolDIInfo pDIInfo;

	// �����״̬��λ,�����������,���ö�Ӧ��λ��־
	ProtocolFrame->ProtocolHandleStatus = 0;

	// ������ݳ����Ƿ����������ֵ����ݳ���(д���ݳ�������Ӧ�ô���12�ֽ�)
	if(ProtocolFrame->pFrame->Length < 3)
	{
		// ���ݳ��Ȳ���ȷ,���ز���ʶ�������
		ProtocolFrame->ProtocolHandleStatus |= PROTOCOL_HANDLE_UNDEFINE;
		return;
	}
	// �������ݱ�ʶ
	ReverseCopy(pDIInfo.DI.Bytes,ProtocolFrame->pFrame->Data,2);

	pDIInfo.Type = TYPE_W;	 //д����
	// Check ���ݱ�ʶ
	if(!CheckDI(&pDIInfo))
	{
		// ���ز�֧�ֵ����ݱ�ʶ
		ProtocolFrame->ProtocolHandleStatus |= PROTOCOL_HANDLE_NO_SUPPORT_ERROR;
		return;
	}	

	// ����Check�������ݱ�ʶ��Ӧ��������Ϣ,�齨���ݰ�(���ݳ��ȳ���)
	ProtocolFrame->pFrame->Length = 0;
	
	// ����������Ϣ��д����
	pDIInfo.OpsType = OPS_WRITE;
	if(!Checkout(&pDIInfo,(ProtocolFrame->pFrame->Data)+2))
	{
		// ��ȡ����ʧ��,����δ֪����
		ProtocolFrame->ProtocolHandleStatus |= PROTOCOL_HANDLE_OTHER_ERROR;
		return;
	}
	g_Var_inst.Test[1]=9;
	rt_kprintf("g_Var_inst.Test[1]: 0x%02x \n", g_Var_inst.Test[1]);	
	rt_kprintf("g_Var_inst.DO: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", 
	g_Var_inst.DO[0],g_Var_inst.DO[1],g_Var_inst.DO[2],g_Var_inst.DO[3],
	g_Var_inst.DO[4],g_Var_inst.DO[5],g_Var_inst.DO[6]);
	// ���������ɹ�
	ProtocolFrame->ProtocolHandleStatus |= PROTOCOL_HANDLE_SUCCEED;

	return;
}

// д��ַ
static void Handle_CMD_T_ADDRESS_BAUDRATE(Comm_st* ProtocolFrame)
{
	ProtocolDIInfo pDIInfo;

	// �����״̬��λ,�����������,���ö�Ӧ��λ��־
	ProtocolFrame->ProtocolHandleStatus = 0;

//	// ������ݳ����Ƿ����������ֵ����ݳ���(д���ݳ�������Ӧ�ô���12�ֽ�)
//	if(ProtocolFrame->pFrame->Length < 3)
//	{
//		// ���ݳ��Ȳ���ȷ,���ز���ʶ�������
//		ProtocolFrame->ProtocolHandleStatus |= PROTOCOL_HANDLE_UNDEFINE;
//		return;
//	}
	// �������ݱ�ʶ
	ReverseCopy(pDIInfo.DI.Bytes,ProtocolFrame->pFrame->Data,2);

	if((pDIInfo.DI.Bytes[1]==0x03)&&(pDIInfo.DI.Bytes[0]==0x00))
	{
		//��������
		ReverseCopy(&g_Var_inst.PC_Address ,(ProtocolFrame->pFrame->Data)+2,1);
	}
	else 	if((pDIInfo.DI.Bytes[1]==0x03)&&(pDIInfo.DI.Bytes[0]==0x01))
	{
		//��������
		ReverseCopy(g_Var_inst.T_Baudrate.Bytes ,(ProtocolFrame->pFrame->Data)+2,2);
	}
	
	Comm_T_Init(g_Var_inst.PC_Address,1,g_Var_inst.PC_Baudrate.Value,COM_PAR_NONE);
	// ����Check�������ݱ�ʶ��Ӧ��������Ϣ,�齨���ݰ�(���ݳ��ȳ���)
	ProtocolFrame->pFrame->Length = 0;
	
	g_Var_inst.Test[1]=9;
	rt_kprintf("g_Var_inst.Test[1]: 0x%02x \n", g_Var_inst.Test[1]);	
	rt_kprintf("g_Var_inst.T_Address: 0x%02x\n", 
	g_Var_inst.T_Address);
	// ���������ɹ�
	ProtocolFrame->ProtocolHandleStatus |= PROTOCOL_HANDLE_SUCCEED;

	return;
}

// ͸������
static BOOL  Handle_CMD_Tansparent(Comm_st* ProtocolFrame)
{
	U8 DI[2];

	// �����״̬��λ,�����������,���ö�Ӧ��λ��־
	ProtocolFrame->ProtocolHandleStatus = 0;
	switch(Comm_PC_inst.State)
	{
		case 0x00:
			// ������ݳ����Ƿ����������ֵ����ݳ���(д���ݳ�������Ӧ�ô���8�ֽ�)
			if(ProtocolFrame->pFrame->Length < 8)
			{
				// ���ݳ��Ȳ���ȷ,���ز���ʶ�������
				ProtocolFrame->ProtocolHandleStatus |= PROTOCOL_HANDLE_UNDEFINE;
				return TRUE;
			}
			// �������ݱ�ʶ
			ReverseCopy(DI,ProtocolFrame->pFrame->Data,2);
			if((DI[0]==0x00)&&(DI[1]==0x05))	//͸������
			{
				Comm_T_inst.State=COM_T_RCV;//���յ�PC��ͨ������
				//��������
				Comm_T_inst.DataCount=ProtocolFrame->pFrame->Length;
				memcpy(Comm_T_inst.Buffer,ProtocolFrame->pFrame->Data,Comm_T_inst.DataCount);
			}
	    	rt_kprintf("DI[0]: 0x%02x,DI[1]: 0x%02x,DataCount: 0x%02x \n", DI[0],DI[1],Comm_T_inst.DataCount);			
			return FALSE;
		case COM_PC_RCV:
			Comm_PC_inst.State=0x00;//���յ�PC��ͨ����
			ProtocolFrame->pFrame->Length=Comm_T_inst.DataCount+2;//��2���ֽ�DI
			memcpy((ProtocolFrame->pFrame->Data)+2,Comm_T_inst.Buffer,Comm_T_inst.DataCount);			
			// ���������ɹ�
			ProtocolFrame->ProtocolHandleStatus |= PROTOCOL_HANDLE_SUCCEED;
				g_Var_inst.Test[1]=11;
	    	rt_kprintf("g_Var_inst.Test[1]: 0x%02x \n", g_Var_inst.Test[1]);
			return TRUE;		
		default:
			// ���ݳ��Ȳ���ȷ,���ز���ʶ�������
			ProtocolFrame->ProtocolHandleStatus |= PROTOCOL_HANDLE_UNDEFINE;
			break;

	}
			return FALSE;
}

// ���Э��,��������������
static BOOL Packet(Comm_st* ProtocolFrame)
{
	U8  i,Sum;
	U8*  pPacket;

	// �����������Ƿ�ɹ�
	if(ProtocolFrame->ProtocolHandleStatus & PROTOCOL_HANDLE_SUCCEED)
	{
		// ����ɹ���ִ��
		ProtocolFrame->pFrame->CMD |= PROTOCOL_CMD_RESPONSE_OK;
	}
	else
	{
		// �����ʧ�ܵĻ�,����Ƿ�Ϊ����ʶ�������
		if(ProtocolFrame->ProtocolHandleStatus & PROTOCOL_HANDLE_UNDEFINE)
		{
//			// Ϊ����ʶ�������,����ӦͨѶ֡,ͨѶ��·��λ����
//			ProtocolFrame->ResetProtocol(ProtocolFrame->CommID);
			return FALSE;
		}
		// ���ǲ���ʶ�������,���ش�����Ϣ��
		ProtocolFrame->pFrame->CMD |= PROTOCOL_CMD_RESPONSE_ERR;
		ProtocolFrame->pFrame->Length = 1;
		ProtocolFrame->pFrame->Data[0] = ProtocolFrame->ProtocolHandleStatus;
	}
					g_Var_inst.Test[1]=8;
	    	rt_kprintf("g_Var_inst.Test[1]: 0x%02x \n", g_Var_inst.Test[1]);
	// �����ݳ���
	ProtocolFrame->DataCount = (4 + ProtocolFrame->pFrame->Length);
	// ��������
	Sum = 0;
	pPacket = (U8*)(ProtocolFrame->pFrame);	
	for( i= 0;i < ProtocolFrame->DataCount;i ++)
	{
		Sum += *(pPacket++);
	}
	*(pPacket) = Sum;
	*(pPacket+1) = FRAME_END;
	ProtocolFrame->DataCount += 2;
	memcpy(Comm_PC_inst.Buffer,ProtocolFrame->pFrame,ProtocolFrame->DataCount);
	    	rt_kprintf("Comm_PC_inst.Buffer: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", 
			Comm_PC_inst.Buffer[0],Comm_PC_inst.Buffer[1],Comm_PC_inst.Buffer[2],Comm_PC_inst.Buffer[3],Comm_PC_inst.Buffer[4],Comm_PC_inst.Buffer[5],
			Comm_PC_inst.Buffer[6],Comm_PC_inst.Buffer[7],Comm_PC_inst.Buffer[8],Comm_PC_inst.Buffer[9],Comm_PC_inst.Buffer[10],Comm_PC_inst.Buffer[11]);
	//��������
	Comm_PC_PortSerialPutByte(Comm_PC_inst.Buffer,ProtocolFrame->DataCount)	;
	return TRUE;
}

BOOL Analysis_Protocol(Comm_st* ProtocolFrame)
{
	uint8 nCMD;

	// ͨѶ������
	nCMD = ProtocolFrame->pFrame->CMD;

	//Э�鴦��
	switch(nCMD)
	{
		case PROTOCOL_CMD_READ:	 //������
			if(Handle_CMD_READ(ProtocolFrame)==FALSE)	
			{
				return FALSE;				
			}
			break;
		case PROTOCOL_CMD_WRITE:	//д����
			Handle_CMD_Write(ProtocolFrame);	
			break;
		case PROTOCOL_CMD_T_ADDRESS_BAUDRATE:	 //д��ַ��������
			Handle_CMD_T_ADDRESS_BAUDRATE(ProtocolFrame);	
			break;
		case PROTOCOL_CMD_WRITEBAUDRATE:	//д������

			break;
		case PROTOCOL_CMD_TARNSPARENT:	 //͸������
			if(Handle_CMD_Tansparent(ProtocolFrame)==FALSE)
			{
					return FALSE;
			}
			break;
		default:
			break; 

	}

	// �����������
	if(Packet(ProtocolFrame))
	{
		return TRUE;
	}
	return FALSE;
}
