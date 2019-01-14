#include "GPS_Parse.h"
#include "GPS_UART.h"
#include "arm_math.h"
#include "Math_User.h"
#include "Attitude.h"

GPS_PVT GPS_PVT_Data;
void GPS_PVT::GPS_PVT_Parse(void)
{
	u8 i = 0;
	u8 Start = 0;
	u16 Length = 0;
	u8 CK_A = 0;
	u8 CK_B = 0;
	
	if(GPS_UART.receive(Rx_buf,GPS_PVT_BUF_SIZE) != 0) 
	{
		for(i = 0;i < GPS_PVT_BUF_SIZE; i++)
		{
			if((GPS_PVT_BUF_SIZE - 1) == i)
				return;
			if(Rx_buf[i] == 0XB5 & Rx_buf[i+1] == 0X62)
				break;
		}
		
		Length = (u16)Rx_buf[i + 4] + ((u16)Rx_buf[i + 5]<<8);
		if(i + Length + 7 > GPS_PVT_BUF_SIZE)
			return;
		

		Start = i + 2;
		for(i = 0;i < 4 + Length;i++)
		{
			CK_A = CK_A + Rx_buf[Start + i];
			CK_B = CK_B + CK_A;		
		}	
		if(CK_A == Rx_buf[Start + 4 + Length] && CK_B == Rx_buf[Start + 5 + Length])
		{ 
			PVT_Data.iTOW 				= ((PVT_Data_*)&Rx_buf[Start + 4])->iTOW;
			PVT_Data.year 				= ((PVT_Data_*)&Rx_buf[Start + 4])->year;
			PVT_Data.month 				= ((PVT_Data_*)&Rx_buf[Start + 4])->month;
			PVT_Data.day 					= ((PVT_Data_*)&Rx_buf[Start + 4])->day;
			PVT_Data.hour 				= ((PVT_Data_*)&Rx_buf[Start + 4])->hour;
			PVT_Data.min 					= ((PVT_Data_*)&Rx_buf[Start + 4])->min;
			PVT_Data.sec 					= ((PVT_Data_*)&Rx_buf[Start + 4])->sec;
			PVT_Data.valid 				= ((PVT_Data_*)&Rx_buf[Start + 4])->valid;
			PVT_Data.tAcc					= ((PVT_Data_*)&Rx_buf[Start + 4])->tAcc;
			PVT_Data.nano 				= ((PVT_Data_*)&Rx_buf[Start + 4])->nano;
			PVT_Data.fixType 			= ((PVT_Data_*)&Rx_buf[Start + 4])->fixType;
			PVT_Data.flags 				= ((PVT_Data_*)&Rx_buf[Start + 4])->flags;
			PVT_Data.reserved1 		= ((PVT_Data_*)&Rx_buf[Start + 4])->reserved1;
			PVT_Data.numSV 				= ((PVT_Data_*)&Rx_buf[Start + 4])->numSV;
			PVT_Data.lon 					= ((PVT_Data_*)&Rx_buf[Start + 4])->lon;
			PVT_Data.lat 					= ((PVT_Data_*)&Rx_buf[Start + 4])->lat;
			PVT_Data.height 			= ((PVT_Data_*)&Rx_buf[Start + 4])->height;
			PVT_Data.hMSL 				= ((PVT_Data_*)&Rx_buf[Start + 4])->hMSL;
			PVT_Data.hAcc 				= ((PVT_Data_*)&Rx_buf[Start + 4])->hAcc;
			PVT_Data.hAcc 				= ((PVT_Data_*)&Rx_buf[Start + 4])->hAcc;
			PVT_Data.velN 				= ((PVT_Data_*)&Rx_buf[Start + 4])->velN;
			PVT_Data.velE 				= ((PVT_Data_*)&Rx_buf[Start + 4])->velE;
			PVT_Data.velD 				= ((PVT_Data_*)&Rx_buf[Start + 4])->velD;
			PVT_Data.gSpeed 			= ((PVT_Data_*)&Rx_buf[Start + 4])->gSpeed;
			PVT_Data.heading 			= ((PVT_Data_*)&Rx_buf[Start + 4])->heading;
			PVT_Data.sAcc 				= ((PVT_Data_*)&Rx_buf[Start + 4])->sAcc;
			PVT_Data.headingAcc 	= ((PVT_Data_*)&Rx_buf[Start + 4])->headingAcc;
			PVT_Data.pDOP 				= ((PVT_Data_*)&Rx_buf[Start + 4])->pDOP;
			PVT_Data.reserved2 		= ((PVT_Data_*)&Rx_buf[Start + 4])->reserved2;
			PVT_Data.reserved3 		= ((PVT_Data_*)&Rx_buf[Start + 4])->reserved3;
		}
	}	
}

GPS GPS_Location;
void GPS::GPS_Update(void)
{
	GPS_PVT_Data.GPS_PVT_Parse();
	GPS_Unit_transform();
}

/*
		���һ��A�ľ� γ��Ϊ(LonA, LatA)���ڶ���B�ľ�γ��Ϊ(LonB, LatB)��
		����0�Ⱦ��ߵĻ�׼������ȡ���ȵ���ֵ(Longitude)������ȡ���ȸ�ֵ(-Longitude)��
		��γȡ90-γ��ֵ(90- Latitude)����γȡ90+γ��ֵ(90+Latitude)��
		�򾭹����������������㱻��Ϊ(MLonA, MLatA)��(MLonB, MLatB)��
		��ô���������Ƶ������Եõ����������������¹�ʽ��
		C = sin(MLatA)*sin(MLatB)*cos(MLonA-MLonB) + cos(MLatA)*cos(MLatB)
		Distance = R*Arccos(C)*Pi/180

		�ȷ�ת����
		���ȷֵ�λ����ת��Ϊ�ȵ�λ���� 
		��=��+��/60
		���磺
		���� = 116��20.12��
		γ�� = 39��12.34�� 
		���� = 116 + 20.12 / 60 = 116.33533�� 
		γ�� = 39 + 12.34 / 60 = 39.20567��

		������ʽ
		l = n��Բ�Ľǣ��� �У�Բ���ʣ��� r���뾶��/180=��(Բ�Ľǻ�����)�� r���뾶��
		
		ֱ��ת����
		���������ߣ��ϼ������������ߣ�����39940.67���γ��һ�Ⱥ�110.94���һ�ֺ�1.849���һ���30.8�ף���ͬγ�ȵļ����һ���ġ�
		������Ȧ����40075.36�����������³ľ������ڱ�γ40�����ң�γ��Ȧ��Ϊ40075*sin(90-40)���������ľ���һ�Ⱥ�85.276���һ�ֺ�1.42���һ���23.69�ס�
		�ڳ�����������γ���ϣ����Ȳ�1�ȶ�Ӧ��ʵ�ʾ����� 110.94����*cosγ�� ��
*/

float longitude_scale(int32_t lat)
{
	static int32_t last_lat = 0;
	static float scale = 1.0;
	//�Ƚ�����γ�����ֵ�������ظ��������Һ���
	if(abs((float)(last_lat-lat)) < 100000){
	// we are within 0.01 degrees (about 1km) of the
	// same latitude. We can avoid the cos() and return
	// the same scale factor.
		return scale;
	}
	scale = cos(lat * 1.0e-7f * DEG_TO_RAD);
	scale = Math.Constrain(scale,1.0f,0.01f);
	last_lat = lat;
	return scale;
}
void GPS::GPS_Unit_transform(void)
{
//  radius of earth in meters
//  #define RADIUS_OF_EARTH 6378100
//  scaling factor from 1e-7 degrees to meters at equater
//  == 1.0e-7 * DEG_TO_RAD * RADIUS_OF_EARTH
  #define LOCATION_SCALING_FACTOR 1.1131884502145034f   //*100 >cm
	
	#define EARTH_RADIUS 6371393
	#define LAT0 0
	#define LON0 0
	static u8 Cnt = 0;
	static double Lon_Deg = 0 ,Lat_Deg = 0;
	int Lat_Int_Temp = 0, Lon_Int_Temp = 0;
	

	Lon_Deg = GPS_PVT_Data.PVT_Data.lon;
	Lat_Deg = GPS_PVT_Data.PVT_Data.lat;

	POS_X = Lon_Deg * LOCATION_SCALING_FACTOR * longitude_scale(Lat_Deg) - Home_OffectX;	//���� ��λcm
	POS_Y = Lat_Deg * LOCATION_SCALING_FACTOR - Home_OffectY;															//γ�� ��λcm
	POS_Z = GPS_PVT_Data.PVT_Data.hMSL * 10 - Home_OffectZ; 															//��Ժ�ƽ��߶� ��λcm	

	//λ����Ҫ�����˲� ����
	Speed.x = GPS_PVT_Data.PVT_Data.velE * 0.1;	// ��λcm/s
	Speed.y = GPS_PVT_Data.PVT_Data.velN * 0.1;	// ��λcm/s
	Speed.z = GPS_PVT_Data.PVT_Data.velD * 0.1;	// ��λcm/s
	//�ٶ���Ҫ�����˲�	����
	state = GPS_PVT_Data.PVT_Data.flags&0x01;
	SatNum = GPS_PVT_Data.PVT_Data.numSV;
	pDOP = GPS_PVT_Data.PVT_Data.pDOP*0.01;
	
	//λ��ƫ��ȡֵ  ��ʱʹ�� ������Ҫ�޸�
	if(state!= 0 && Cnt < 105)
	{
		Cnt++;
	}
	if(Cnt == 100)
	{
		Home_OffectX = POS_X;
		Home_OffectY = POS_Y;
		Home_OffectZ = POS_Z;
	}
}







