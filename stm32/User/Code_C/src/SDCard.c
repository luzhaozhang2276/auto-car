#include "SDCard.h"


u8  SD_Type=0;
SPI_InitTypeDef  SPI_InitStructure;
void SPI_SD_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//ʹ�� GPIOB ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);// ʹ�� SPI1 ʱ��
	
	//CS
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_12);
	
	//GPIOFB3,4,5 ��ʼ������: ���ù������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;//PB3~5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOB, &GPIO_InitStructure);// ��ʼ��
	//�������Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource3,GPIO_AF_SPI1); //PB3 ����Ϊ SPI1
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource4,GPIO_AF_SPI1); //PB4 ����Ϊ SPI1
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource5,GPIO_AF_SPI1); //PB5 ����Ϊ SPI1
	//����ֻ��� SPI �ڳ�ʼ��
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);//��λ SPI1
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);//ֹͣ��λ SPI1
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //���� SPI ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master; //���� SPI ����ģʽ:�� SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;  //���� SPI �����ݴ�С: 8 λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;//����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; //���ݲ����ڵڶ���ʱ����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; //NSS �ź���Ӳ������
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;  //Ԥ��Ƶ 256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //���ݴ���� MSB λ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;  //CRC ֵ����Ķ���ʽ
	SPI_Init(SPI1, &SPI_InitStructure); //����ָ���Ĳ�����ʼ������ SPIx �Ĵ���
	
	SPI_Cmd(SPI1, ENABLE); //ʹ�� SPI1

}

/*******************************************************************************
* Function Name  : SPI_ReadWriteByte
* Description    : SPI��дһ���ֽڣ�������ɺ󷵻ر���ͨѶ��ȡ�����ݣ�
* Input          : u8 TxData �����͵���
* Output         : None
* Return         : u8 RxData �յ�����
*******************************************************************************/
u8 SPI_ReadWriteByte(u8 TxData)
{
	while((SPI1->SR&SPI_I2S_FLAG_TXE)==RESET);		//�ȴ���������	  
	SPI1->DR = TxData;	 	                        //����һ��byte   
	while((SPI1->SR&SPI_I2S_FLAG_RXNE)==RESET){};   //�ȴ�������һ��byte  
	return SPI1->DR;          	                  //�����յ�������			
} 


/*******************************************************************************
* Function Name  : SPI_SetSpeed
* Description    : SPI�����ٶ�Ϊ����
* Input          : u8 SpeedSet 
*                  ����ٶ���������0�������ģʽ����0�����ģʽ
*                  SPI_SPEED_HIGH   1
*                  SPI_SPEED_LOW    0
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_SetSpeed(u8 SpeedSet)
{
	if(SpeedSet == SPI_SPEED_LOW)
		SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;	
	else
		SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;	
	
	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_Cmd(SPI1, ENABLE); //ʹ��SPI����
}
/*******************************************************************************
* Function Name  : SD_WaitReady
* Description    : �ȴ�SD��Ready
* Input          : None
* Output         : None
* Return         : u8 
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
u8 SD_WaitReady(void)
{
	u8 Ret;
	u16 Retry = 0;
	do
	{
		Ret = SPI_ReadWriteByte(0xFF);
		if(Retry++ == 0XFFFE) return 1;

	}while(Ret != 0xFF);

	return 0;
}



/*******************************************************************************
* Function Name  : SD_SendCommand
* Description    : ��SD������һ������
* Input          : u8 cmd   ���� 
*                  u32 arg  �������
*                  u8 crc   crcУ��ֵ
* Output         : None
* Return         : u8 r1 SD�����ص���Ӧ
*******************************************************************************/
u8 SD_SendCommand(u8 cmd, u32 arg, u8 crc)
{
	u8 Ret;
	u8 Retry = 0;

	SPI_ReadWriteByte(0xff);
	//Ƭѡ���õͣ�ѡ��SD��
	SD_CS_ENABLE();

	SPI_ReadWriteByte(cmd | 0x40);                         
	SPI_ReadWriteByte(arg >> 24);
	SPI_ReadWriteByte(arg >> 16);
	SPI_ReadWriteByte(arg >> 8);
	SPI_ReadWriteByte(arg);
	SPI_ReadWriteByte(crc);
	
	//�ȴ���Ӧ����ʱ�˳�
	while((Ret = SPI_ReadWriteByte(0xFF)) == 0xFF)
	{
			Retry++;
			if(Retry > 200) break;
	}
 
	//�ر�Ƭѡ
	SD_CS_DISABLE();
	//�������϶�������8��ʱ�ӣ���SD�����ʣ�µĹ���
	SPI_ReadWriteByte(0xFF);

	//����״ֵ̬
	return Ret;
}


/*******************************************************************************
* Function Name  : SD_SendCommand_NoDeassert
* Description    : ��SD������һ������(�����ǲ�ʧ��Ƭѡ�����к������ݴ�����
* Input          : u8 cmd   ���� 
*                  u32 arg  �������
*                  u8 crc   crcУ��ֵ
* Output         : None
* Return         : u8 r1 SD�����ص���Ӧ
*******************************************************************************/
u8 SD_SendCommand_NoDeassert(u8 cmd, u32 arg, u8 crc)
{
	u8 Ret;
	u8 Retry = 0;

	SPI_ReadWriteByte(0xff);
	//Ƭѡ���õͣ�ѡ��SD��
	SD_CS_ENABLE();

	//����
	SPI_ReadWriteByte(cmd | 0x40);                         //�ֱ�д������
	SPI_ReadWriteByte(arg >> 24);
	SPI_ReadWriteByte(arg >> 16);
	SPI_ReadWriteByte(arg >> 8);
	SPI_ReadWriteByte(arg);
	SPI_ReadWriteByte(crc);

	//�ȴ���Ӧ����ʱ�˳�
	while((Ret = SPI_ReadWriteByte(0xFF)) == 0xFF)
	{
			Retry++;
			if(Retry > 200) break;
	}
	//������Ӧֵ
	return Ret;
}

/*******************************************************************************
* Function Name  : SD_Init
* Description    : ��ʼ��SD��
* Input          : None
* Output         : None
* Return         : u8 
*                  0��NO_ERR
*                  1��TIME_OUT
*                  99��NO_CARD
*******************************************************************************/
u8 SD_Init(void)
{
	u16 i;      
	u8 Ret;      
	u16 Retry;  
	u8 Buff[6];
	
	SPI_SD_Init();
	
	if(!SD_DET()) return STA_NODISK;  //FatFS�����־��û�в������    
	SD_PWR_ON();                      //SD���ϵ�  
	for(i=0;i<0xf00;i++);             //��ʱ���ȴ�SD���ϵ����   
	for(i=0;i<10;i++)                 //�Ȳ���>74�����壬��SD���Լ���ʼ�����
	{
			SPI_ReadWriteByte(0xFF);
	}

	Retry = 0;
	do
	{      
			Ret = SD_SendCommand(CMD0, 0, 0x95); //����CMD0����SD������IDLE״̬
			Retry++;
	}while((Ret != 0x01) && (Retry < 200));

	Ret = SD_SendCommand_NoDeassert(8, 0x1aa, 0x87); //��ȡ��Ƭ��SD�汾��Ϣ 
	if(Ret == 0x05)                                 //�����Ƭ�汾��Ϣ��v1.0�汾��
	{
			
		SD_Type = SD_TYPE_V1;                       //���ÿ�����ΪSDV1.0����������⵽ΪMMC�������޸�ΪMMC  
		SD_CS_DISABLE();                            //�����V1.0����CMD8ָ���û�к�������
		SPI_ReadWriteByte(0xFF);                    //�෢8��CLK����SD������������

		//-----------------SD����MMC����ʼ����ʼ-----------------
		//������ʼ��ָ��CMD55+ACMD41
		//�����Ӧ��˵����SD�����ҳ�ʼ�����
		//û�л�Ӧ��˵����MMC�������������Ӧ��ʼ��
		Retry = 0;
		do
		{       
				Ret = SD_SendCommand(CMD55, 0, 0);    //�ȷ�CMD55��Ӧ����0x01���������
				if(Ret != 0x01)  return Ret;           
				Ret = SD_SendCommand(ACMD41, 0, 0);   //�õ���ȷ��Ӧ�󣬷�ACMD41��Ӧ�õ�����ֵ0x00����������200��
				Retry++;
		}while((Ret!= 0x00) && (Retry < 400));
		if(Retry == 400)
		{
				Retry = 0;
				do
				{
						Ret = SD_SendCommand(1, 0, 0);
						Retry++;
				}while((Ret != 0x00)&&(Retry < 400)); //����MMC����ʼ�����û�в��ԣ�
				if(Retry == 400)  return 1;           //MMC����ʼ����ʱ
				SD_Type = SD_TYPE_MMC;
		}
					 
		SPI_SetSpeed(SPI_SPEED_HIGH);                     
		SPI_ReadWriteByte(0xFF);
		
		Ret = SD_SendCommand(CMD16, 512, 0xff);   //����Sector Size
		if(Ret != 0x00) return Ret;
	}
	//������V2.0���ĳ�ʼ��
	//������Ҫ��ȡOCR���ݣ��ж���SD2.0����SD2.0HC��
	else 
	{
		if(Ret == 0x01)
		{
			//V2.0�Ŀ���CMD8�����ᴫ��4�ֽڵ����ݣ�Ҫ�����ٽ���������
			Buff[0] = SPI_ReadWriteByte(0xFF);  //should be 0x00
			Buff[1] = SPI_ReadWriteByte(0xFF);  //should be 0x00
			Buff[2] = SPI_ReadWriteByte(0xFF);  //should be 0x01
			Buff[3] = SPI_ReadWriteByte(0xFF);  //should be 0xAA
	 
			SD_CS_DISABLE();
			//the next 8 clocks
			SPI_ReadWriteByte(0xFF);
			
			//�жϸÿ��Ƿ�֧��2.7V-3.6V�ĵ�ѹ��Χ
			if(Buff[2]==0x01 && Buff[3]==0xAA)
			{      
				Retry = 0;  //֧�ֵ�ѹ��Χ�����Բ���
				do
				{
					Ret = SD_SendCommand(CMD55, 0, 0);
					if(Ret != 0x01) return Ret;
					Ret = SD_SendCommand(ACMD41, 0x40000000, 0);
					if(Retry++ > 200)  return Ret;  //��ʱ�򷵻�r1״̬
				}while(Ret != 0);                 //������ʼ��ָ��CMD55+ACMD41
				
					//��ʼ��ָ�����ɣ���������ȡOCR��Ϣ
					//-----------����SD2.0���汾��ʼ-----------
					Ret = SD_SendCommand_NoDeassert(CMD58, 0, 0);
					if(Ret!=0x00)
					{
							return Ret;  //�������û�з�����ȷӦ��ֱ���˳�������Ӧ��
					}
					//��OCRָ����󣬽�������4�ֽڵ�OCR��Ϣ
					Buff[0] = SPI_ReadWriteByte(0xFF);
					Buff[1] = SPI_ReadWriteByte(0xFF); 
					Buff[2] = SPI_ReadWriteByte(0xFF);
					Buff[3] = SPI_ReadWriteByte(0xFF);

					//OCR������ɣ�Ƭѡ�ø�
					SD_CS_DISABLE();
					SPI_ReadWriteByte(0xFF);

					//�����յ���OCR�е�bit30λ��CCS����ȷ����ΪSD2.0����SDHC
					//���CCS=1��SDHC   CCS=0��SD2.0
					if(Buff[0] & 0x40)    //���CCS
					{
							SD_Type = SD_TYPE_V2HC;
					}
					else
					{
							SD_Type = SD_TYPE_V2;
					}      
					//����SPIΪ����ģʽ
					SPI_SetSpeed(SPI_SPEED_HIGH);  
			}
    }
	}
  return Ret;
}



/*******************************************************************************
* Function Name  : SD_ReceiveData
* Description    : ��SD���ж���ָ�����ȵ����ݣ������ڸ���λ��
* Input          : u8 *Data(��Ŷ������ݵ��ڴ�>len)
*                  u16 len(���ݳ��ȣ�
*                  u8 Release(������ɺ��Ƿ��ͷ�����CS�ø� 0�����ͷ� 1���ͷţ�
* Output         : None
* Return         : u8 
*                  0��NO_ERR
*                  other��������Ϣ
*******************************************************************************/
u8 SD_ReceiveData(u8 *Data, u16 Len, u8 Release)
{
	u16 Retry;
	u8 Ret;

	// ����һ�δ���
	SD_CS_ENABLE();
	//�ȴ�SD������������ʼ����0xFE
	Retry = 0;
	do
	{
		Ret = SPI_ReadWriteByte(0xFF);
		Retry++;
		if(Retry > 2000)  //2000�εȴ���û��Ӧ���˳�����
		{
			SD_CS_DISABLE();
			return 1;
		}
	}while(Ret != 0xFE);
	//��ʼ��������
	while(Len--)
	{
		*Data = SPI_ReadWriteByte(0xFF);
		Data++;
	}
	//������2��αCRC��dummy CRC��
	SPI_ReadWriteByte(0xFF);
	SPI_ReadWriteByte(0xFF);
	//�����ͷ����ߣ���CS�ø�
	if(Release == RELEASE)
	{
		//�������
		SD_CS_DISABLE();
		SPI_ReadWriteByte(0xFF);
	}

	return 0;
}


/*******************************************************************************
* Function Name  : SD_GetCID
* Description    : ��ȡSD����CID��Ϣ��������������Ϣ
* Input          : u8 *cid_Data(���CID���ڴ棬����16Byte��
* Output         : None
* Return         : u8 
*                  0��NO_ERR
*                  1��TIME_OUT
*                  other��������Ϣ
*******************************************************************************/
u8 SD_GetCID(u8 *Cid_Data)
{
	u8 Ret;

	//��CMD10�����CID
	Ret = SD_SendCommand(CMD10, 0, 0xFF);
	if(Ret != 0x00) return Ret;
	//����16���ֽڵ�����
	SD_ReceiveData(Cid_Data, 16, RELEASE);
	return 0;
}


/*******************************************************************************
* Function Name  : SD_GetCSD
* Description    : ��ȡSD����CSD��Ϣ�������������ٶ���Ϣ
* Input          : u8 *cid_Data(���CID���ڴ棬����16Byte��
* Output         : None
* Return         : u8 
*                  0��NO_ERR
*                  1��TIME_OUT
*                  other��������Ϣ
*******************************************************************************/
u8 SD_GetCSD(u8 *Csd_Data)
{
	u8 Ret;

	//��CMD9�����CSD
	Ret = SD_SendCommand(CMD9, 0, 0xFF);
	if(Ret != 0x00) return Ret;
	//����16���ֽڵ�����
	SD_ReceiveData(Csd_Data, 16, RELEASE);
	return 0;
}


/*******************************************************************************
* Function Name  : SD_GetCapacity
* Description    : ��ȡSD��������
* Input          : None
* Output         : None
* Return         : u32 capacity 
*                   0�� ȡ�������� 
*******************************************************************************/
u32 SD_GetCapacity(void)
{
	u8 Csd[16];
	u32 Capacity;
	u8 Ret;
	u16 i;
	u16 Temp;

	//ȡCSD��Ϣ������ڼ��������0
	if(SD_GetCSD(Csd) != 0) return 0;	 
	//���ΪSDHC�����������淽ʽ����
	if((Csd[0] & 0xC0) == 0x40)
	{
			Capacity =  (((u32)(Csd[8])<<8) + (u32)Csd[9] + 1) * (u32)1024;
	}
	else
	{

		i = Csd[6] & 0x03;
		i <<= 8;
		i += Csd[7];
		i <<= 2;
		i += ((Csd[8] & 0xc0) >> 6);
	
		//C_SIZE_MULT
		Ret = Csd[9]&0x03;
		Ret<<= 1;
		Ret += ((Csd[10] & 0x80) >> 7);
	
		//BLOCKNR
		Ret += 2;
		Temp = 1;
		while(Ret)
		{
			Temp *= 2;
			Ret--;
		}
		Capacity = ((u32)(i + 1)) * ((u32)Temp);
		// READ_BL_LEN
		i = Csd[5]&0x0f;
		Temp = 1;
		while(i)
		{
			Temp *= 2;
			i--;
		}
    //The final result
		Capacity *= (u32)Temp; 
	}
	return (u32)Capacity;
}


/*******************************************************************************
* Function Name  : SD_ReadSingleBlock
* Description    : ��SD����һ��block
* Input          : u32 Sector ȡ��ַ��Sectorֵ���������ַ�� 
*                  u8 *Buffer ���ݴ洢��ַ����С����512byte�� 
* Output         : None
* Return         : u8 r1 
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
u8 SD_ReadSingleBlock(u32 Sector, u8 *Buffer)
{
	u8 Ret;
  //����Ϊ����ģʽ
	SPI_SetSpeed(SPI_SPEED_HIGH);   
  //�������SDHC����Sector��ַת��byte��ַ
  Sector = Sector<<9;
	Ret = SD_SendCommand(CMD17, Sector, 0);//������
	if(Ret != 0x00) return Ret;  
  Ret = SD_ReceiveData(Buffer, 512, RELEASE);
	return Ret;  
}

/*******************************************************************************
* Function Name  : SD_WriteSingleBlock
* Description    : д��SD����һ��block
* Input          : u32 Sector ������ַ��Sectorֵ���������ַ�� 
*                  u8 *Buffer ���ݴ洢��ַ����С����512byte�� 
* Output         : None
* Return         : u8 r1 
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
u8 SD_WriteSingleBlock(u32 Sector, const u8 *Data)
{
	u8 Ret;
	u16 i;
	u16 Retry;
	//����Ϊ����ģʽ
	SPI_SetSpeed(SPI_SPEED_HIGH);

  //�������SDHC����������Sector��ַ������ת����byte��ַ
  if(SD_Type!=SD_TYPE_V2HC)
  {
		Sector = Sector<<9;
  }

  Ret = SD_SendCommand(CMD24, Sector, 0x00);
  if(Ret != 0x00) return Ret;   
	//��ʼ׼�����ݴ���
	SD_CS_ENABLE();
	//�ȷ�3�������ݣ��ȴ�SD��׼����
	SPI_ReadWriteByte(0xff);
	SPI_ReadWriteByte(0xff);
	SPI_ReadWriteByte(0xff);
	//����ʼ����0xFE
	SPI_ReadWriteByte(0xFE);

	//��һ��Sector������
	for(i=0;i<512;i++)
	{
			SPI_ReadWriteByte(*Data++);
	}
	//��2��Byte��dummy CRC
	SPI_ReadWriteByte(0xff);
	SPI_ReadWriteByte(0xff);  
  //�ȴ�SD��Ӧ��
  Ret = SPI_ReadWriteByte(0xff);
	if((Ret & 0x1F) != 0x05)
	{
			SD_CS_DISABLE();
			return Ret;
	}   
	//�ȴ��������
	Retry = 0;
	while(!SPI_ReadWriteByte(0xff))
	{
		Retry++;
		if(Retry>0xfffe)        //�����ʱ��д��û����ɣ������˳�
		{
			SD_CS_DISABLE();
			return 1;            //д�볬ʱ����1
		}
	}
	//д����ɣ�Ƭѡ��1
	SD_CS_DISABLE();
	SPI_ReadWriteByte(0xff);

	return 0;
}


/*******************************************************************************
* Function Name  : SD_ReadMultiBlock
* Description    : ��SD���Ķ��block
* Input          : u32 Sector ȡ��ַ��Sectorֵ���������ַ�� 
*                  u8 *Buffer ���ݴ洢��ַ����С����512byte��
*                  u8 Count ������Count��block
* Output         : None
* Return         : u8 r1 
*                  0�� �ɹ�
*                  other��ʧ��
*******************************************************************************/
u8 SD_ReadMultiBlock(u32 Sector, u8 *Buffer, u8 Count)
{
	u8 Ret;
  //����Ϊ����ģʽ
	SPI_SetSpeed(SPI_SPEED_HIGH);
  //�������SDHC����Sector��ַת��byte��ַ
	Sector = Sector<<9;
  //SD_WaitReady();
  //�����������
	Ret = SD_SendCommand(CMD18, Sector, 0);//������
	if(Ret != 0x00) return Ret;
	do
	{
		if(SD_ReceiveData(Buffer, 512, NO_RELEASE) != 0x00) break;
		Buffer += 512;
	} while(--Count);

	SD_SendCommand(CMD12, 0, 0);  //ȫ��������ϣ�����ֹͣ����
	SD_CS_DISABLE();              //�ͷ�����
	SPI_ReadWriteByte(0xFF);
	
	return Count;                 //���û�д��꣬����ʣ�����
}

/*******************************************************************************
* Function Name  : SD_WriteMultiBlock
* Description    : д��SD����N��block
* Input          : u32 Sector ������ַ��Sectorֵ���������ַ�� 
*                  u8 *Buffer ���ݴ洢��ַ����С����512byte��
*                  u8 Count д���block��Ŀ
* Output         : None
* Return         : u8 r1 
*                  0�� �ɹ�
*                  other��ʧ��
*******************************************************************************/
u8 SD_WriteMultiBlock(u32 Sector, const u8 *Data, u8 Count)
{
	u8 Ret;
	u16 i;

	//����Ϊ����ģʽ
	SPI_SetSpeed(SPI_SPEED_HIGH);
	//�������SDHC����������Sector��ַ������ת����byte��ַ
	if(SD_Type != SD_TYPE_V2HC)
	{
		Sector = Sector << 9;
	}
	//���Ŀ�꿨����MMC��������ACMD23ָ��ʹ��Ԥ����
	if(SD_Type != SD_TYPE_MMC)
	{
		Ret = SD_SendCommand(ACMD23, Count, 0x00);
	}
  //�����д��ָ��
	Ret = SD_SendCommand(CMD25, Sector, 0x00);
	if(Ret != 0x00) return Ret;
    
  //��ʼ׼�����ݴ���
	SD_CS_ENABLE();
	//�ȷ�3�������ݣ��ȴ�SD��׼����
	SPI_ReadWriteByte(0xff);
	SPI_ReadWriteByte(0xff);

	//��N��Sectorд���ѭ������
	do
	{
		//����ʼ����0xFC �����Ƕ��д��
		SPI_ReadWriteByte(0XFC);  
		//��һ��Sector������
		for(i = 0;i < 512;i++)
		{
			SPI_ReadWriteByte(*Data++);
		}
		//��2��Byte��dummy CRC
		SPI_ReadWriteByte(0XFF);
		SPI_ReadWriteByte(0XFF);     
		//�ȴ�SD��Ӧ��
		Ret = SPI_ReadWriteByte(0XFF);
		if((Ret & 0X1F) != 0X05) //���Ӧ��Ϊ��������������ֱ���˳�
		{
			SD_CS_DISABLE();    
			return Ret;
		}
		//�ȴ�SD��д�����
		if(SD_WaitReady()==1)
		{
				SD_CS_DISABLE();   
				return 1;
		}
	}while(--Count);
    
	//��������������0xFD
	Ret = SPI_ReadWriteByte(0XFD);
	if(Ret==0X00) Count =  0XFE; 
	//д����ɣ�Ƭѡ��1
	SD_CS_DISABLE();
	SPI_ReadWriteByte(0XFF);
	return Count;         //����Countֵ�����д����Count=0������Count=1
}
