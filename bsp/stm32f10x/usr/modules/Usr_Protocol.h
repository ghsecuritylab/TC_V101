#ifndef __USR_PROTOCOL_H__
#define __USR_PROTOCOL_H__
#include "Usr_Portserial.h"

//#define     OFFSET(Struct, Member) ((unsigned char*) &((Struct *)0)->Member)

#define     OFFSET(Struct, Member) ((unsigned int)(&((Struct *)0)->Member))
#define     SIZEOF(Struct, Member) (sizeof(((Struct *)0)->Member))

// ͨѶ���������
#define PROTOCOL_CMD_READ							(0x01)	// ��
#define PROTOCOL_CMD_WRITE							(0x02)	// д
#define PROTOCOL_CMD_T_ADDRESS_BAUDRATE					(0x03)	// д��ַ,д������
#define PROTOCOL_CMD_WRITEBAUDRATE					(0x04)	// д������
#define PROTOCOL_CMD_TARNSPARENT					(0x05)	// ͸������


// ����Э�鴦����(ProtocolHandleStatus��λ)
#define PROTOCOL_HANDLE_SUCCEED		(0x80)	// �����ɹ�
#define PROTOCOL_HANDLE_UNDEFINE	(0x40)	// ����ʶ���������

#define PROTOCOL_HANDLE_OTHER_ERROR			(0x01)	// ��������
#define PROTOCOL_HANDLE_NO_SUPPORT_ERROR	(0x02)	// ��֧�ֵ�����
#define PROTOCOL_HANDLE_DATA_ERROR			(0x04)	// ���ݴ���

#define PROTOCOL_CMD_RESPONSE_OK	(0x80)		// ��վ��Ӧ������֡
#define PROTOCOL_CMD_RESPONSE_ERR	(0xC0)		// ��վ���쳣Ӧ��֡

// ͨѶ������ָ��
typedef void (*ProtocolHandler)(void);
// CheckDI ��Ϣ�ṹ��
typedef struct
{
	U16_UNION DI;		// ����DI��
	U8 Address;		// ���ݴ�ŵĵ�ַ
	U8 Type;			// ��������(��/д,ֻд,ֻ��)
	U8 OpsType;			// ���ݲ�������(����д)
	U8 MomeryType;		// ���ݴ�Ž���(EEROM,FRAM,FALSH,VARRAM)����λ0x80��ʾ��ʾ����
	ProtocolHandler pfnWriteSuccessed;	// д�ɹ����ò������º���
	U8 Length;			// ���ݳ���
}ProtocolDIInfo;

// ��������
enum DATA_TYPE
{
	TYPE_WR = 0,// �ɶ���д
	TYPE_W,		// ֻд
	TYPE_R		// ֻ��
};
// ���ݲ�������(����д)
enum OPS_TYPE
{
	OPS_READ =0,
	OPS_WRITE
};
enum MEMORYTYPE
{
	MEMORY_RAM = 0,
	MEMORY_EEROM, 
	MEMORY_MAX
};

typedef struct{

	//���ݱ�־DIType:BIT0-BIT2,������������,BIT0-������,BIT1-д����;
	unsigned char DIType;
	unsigned char DI1;
	unsigned char DI0;
	unsigned char LEN;
	unsigned int DataAddr;
}Tab_DI;



extern BOOL Analysis_Protocol(Comm_st* ProtocolFrame);

#endif
