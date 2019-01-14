#ifndef __GPS_PARSE_H__
#define __GPS_PARSE_H__
#include "stm32f4xx.h"
#include "Common.h"

class PVT_Data_
{
	public:
	u32 iTOW;
	u16 year;					//�� 
	u8 month;					//�� 
	u8 day;					  //�� 
	u8 hour;					//ʱ 
	u8 min;						//�� 
	u8 sec;						//�� 
	u8 valid;					//��Ч��־λ 
	u32 tAcc;					//ʱ�侫�ȹ���
	s32 nano; 				//��������
	u8 fixType;				//��λ����
	u8 flags; 				//�Ƿ���Ч��λ  ���ģʽ�Ƿ���(DGPS)  ʡ��ģʽ��δ��  �����Ƿ���Ч
	u8 reserved1; 		//����
	u8 numSV;   			//���붨λ���Ǹ���
	s32 lon;					//����  deg
	s32 lat;					//γ��  deg
	s32 height;				//����߶�  mm
	s32 hMSL;					//GPS��ȡ���θ߶� 	mm
	u32 hAcc;					//ˮƽλ�ù��ƾ��� mm
	u32 vAcc;					//��ֱλ�ù��ƾ���	mm
	s32 velN;					//GPS��ȡ���ص���ϵ�����ٶ�  		mm/s
	s32 velE;					//GPS��ȡ���ص���ϵ�������ٶ�  	mm/s
	s32 velD;					//GPS��ȡ���ص���ϵ'��'���ٶ�  	mm/s
	s32 gSpeed;				//����Ե��ٶ�	mm/s
	s32 heading;			//�����˶������	deg
	u32 sAcc;					//�ٶȹ��ƾ��� mm/s
	u32 headingAcc;		//�����˶�����ǹ��ƾ��� deg
	u16 pDOP;					//λ�þ�������
	u16 reserved2;
	u32 reserved3;
};
#define GPS_PVT_BUF_SIZE 110
class GPS_PVT
{
public:
	GPS_PVT()
	{}
	void GPS_PVT_Parse(void);	
	PVT_Data_ PVT_Data;
private:
	u8 Rx_buf[GPS_PVT_BUF_SIZE];

};

class GPS
{
	public:
		GPS(){
		Home_OffectX = 0;
		Home_OffectY = 0;
		Home_OffectZ = 0;
		};
			
	u8 state;
	u8 SatNum;
	float pDOP;
	double Home_OffectX;
	double Home_OffectY;
	double Home_OffectZ;
			
	double POS_X;
	double POS_Y;
	double POS_Z;
	Vector POS;	
	Vector Speed;
	void GPS_Update(void);
			
	private:
		void GPS_Unit_transform(void);
};

extern GPS GPS_Location;
#endif


