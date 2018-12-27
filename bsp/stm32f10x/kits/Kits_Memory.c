#include "Kits_Memory.h"
// ==================================================================
// Function name: 	ArraySwap
// Description:		������ֽ���������ֽ����ݽ�������	
// Parameter:			pData = ���ݵ�ַָ��;nLen = ���ݳ���,���ܴ���100�ֽ�
// Return:           	NULL
// Other:            	NULL
// ==================================================================
void MemoryReverse(U8 *pData,U8 nLen)
{
    unsigned char j;
    unsigned char i;
    unsigned char nSwap;
    
    if (nLen == 0 || nLen == 1)
    {
        return;
    }
    
    i = 0;
    j = nLen - 1;
    while (i < j)
    {
        nSwap = *(pData + i);
        *(pData + i) = *(pData + j);
        *(pData + j) = nSwap;
        i++;
        j--;
    }
    return;
}
#if NO_USE == 0
//����������
void ReverseCopy(unsigned char *pDis,unsigned char *pSource,unsigned char Length)
{
	unsigned char i;
	for(i=0;i<Length;i++)
	{
		pDis[i]=pSource[Length-1-i];
	}
}
#endif
void MemoryCopy(U8 *pDis,const U8 *pSource,U8 Length)
{
	if(!Length)
	{
		return;
	}
	while(Length--)
	{
		*(pDis++) = *(pSource++);
	}
	return;
}
void MemoryReverseCopy(U8 *pDis,const U8 *pSource,U8 Length)
{
	if(!Length)
	{
		return;
	}
	pSource += (Length-1);
	while(Length--)
	{
		*(pDis++) = *(pSource--);
	}
	return;
}
//�ж�����ȫΪ0
BOOL Judge_ArrayZero(U8 *pData,U8 nLen)
{
	U8 i;
	for(i=0;i<nLen;i++)
	{
		if(pData[i]!=0x00)
		{
			return FALSE;
		}
	}
	return TRUE;
}

