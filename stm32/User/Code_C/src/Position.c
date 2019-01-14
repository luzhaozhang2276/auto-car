#include "Position.h"

void Position_Updata(u16 Time_Ms);
void Position_Init(void);
Filter_Balance FIL_Speed(0.1,0);
Filter_Balance FIL_Position_xyz(1,0);
struct Position_ Position = 
{
	Vector(0,0,0),
	Vector(0,0,0),
	Vector(0,0,0),
	Position_Init,
	Position_Updata
};

//Time :MS
void Position_Init(void)
{
	MS5611.Init();
	GPS_UART.Init(115200);
}

/****��ѹ�����׻����˲����������ο���Դ�ɿ�APM****/
#define TIME_CONTANST_ZER 2.5f
#define K_ACC_ZER 	    (1.0f / (TIME_CONTANST_ZER * TIME_CONTANST_ZER * TIME_CONTANST_ZER))
#define K_VEL_ZER	        (3.0f / (TIME_CONTANST_ZER * TIME_CONTANST_ZER))														
#define K_POS_ZER         (2.0f / TIME_CONTANST_ZER)
#define Delay_Cnt 60
void Altitude_Update(u16 Time_Ms,Vector ACC_Earth)
{
	float Origion_Acc_z;
	static float acc_correction = 0;
	static float vel_correction = 0;
	static float pos_correction = 0;
	static float Last_Acc_z = 0;
	static float Origion_Speed_z = 0;
	static float Origion_Position_z = 0;
	float SpeedDealt = 0;
	float Altitude_Dealt=0;
	float Altitude_Estimate=0;
	static float Altitude_History[Delay_Cnt+1];
	float Delta_T;
	u16 Cnt=0;
	
	static uint16_t Save_Cnt=0;
	Save_Cnt++;//���ݴ洢����
	
	Delta_T = Time_Ms/1000.0;//msתs ��λ s
	Altitude_Estimate = MS5611.Data->Altitude*100.0;//�߶ȹ۲��� mתcm ��λcm
	Origion_Acc_z = ACC_Earth.z;//���ٶ�  ��λcm/s2
	
	//�ɹ۲�������ѹ�ƣ��õ�״̬���
	Altitude_Dealt = Altitude_Estimate -  Position.Position_xyz.z;//��ѹ��(������)��SINS�������Ĳ��λcm
	//��·���ַ����������ߵ�
	acc_correction +=Altitude_Dealt* K_ACC_ZER*Delta_T ;//���ٶȽ�����
	vel_correction +=Altitude_Dealt* K_VEL_ZER*Delta_T ;//�ٶȽ�����
	pos_correction +=Altitude_Dealt* K_POS_ZER*Delta_T ;//λ�ý�����
	//���ٶȼƽ��������
	Last_Acc_z = Position.Acc.z;//��һ�μ��ٶ���
	Position.Acc.z = Origion_Acc_z + acc_correction;// ���ٶȵ�λcm/s2
	//�ٶ�������������£����ڸ���λ��,���ڲ���h=0.005,��Խϳ���
	//������ö����������������΢�ַ��̣��������ø��߽׶Σ���Ϊ���ٶ��źŷ�ƽ��
	SpeedDealt = (Last_Acc_z + Position.Acc.z) * Delta_T/2.0;
	//ԭʼλ�ø���
	Origion_Position_z += (Position.Speed.z + 0.5*SpeedDealt) * Delta_T;
	//λ�ý��������
	Position.Position_xyz.z = Origion_Position_z + pos_correction;    
	//ԭʼ�ٶȸ���
	Origion_Speed_z += SpeedDealt;
	//�ٶȽ��������
	Position.Speed.z = Origion_Speed_z + vel_correction;

	if(Save_Cnt>=1)
	{
		for(Cnt = Delay_Cnt;Cnt > 0;Cnt--)//����
		{
			Altitude_History[Cnt]=Altitude_History[Cnt-1];
		}
		Altitude_History[0] = Position.Position_xyz.z;
		Save_Cnt=0;
	}
}

/*********************************ˮƽλ��˫�۲⿨�����ں� GPS+ACC *****************************************/
Filter_EKF_Double Filter_EKF_X(0.02,0.05,50,200);
Filter_EKF_Double Filter_EKF_Y(0.02,0.05,50,200);
void XY_Update(u16 Time_Ms,Vector ACC_Earth)
{
	if(GPS_Location.state != 0 && GPS_Location.SatNum > 5)
	{
		Filter_EKF_X.EKFFilter_Double(Position.Position_xyz.x,		//�ϴ�Ԥ������				cm
																	Position.Speed.x,						//�ϴ�Ԥ���ٶ�				cm/s
																	GPS_Location.POS_X,					//��ǰGPS�۲����   	cm
																	GPS_Location.Speed.x,				//��ǰGPS�۲��ٶ�   	cm/s
																	ACC_Earth.x,						//��ǰ���ٶȼƹ۲���ٶ� cm/s2
																	Position.Position_xyz.x,		//Ԥ��������        	cm
																	Position.Speed.x,						//Ԥ�����ٶ�					cm/s
																	Time_Ms/1000);							//����ʱ��						s
		Position.Acc.x = ACC_Earth.x; //cm/s2
		Filter_EKF_Y.EKFFilter_Double(Position.Position_xyz.y,		//�ϴ�Ԥ������
																	Position.Speed.y,						//�ϴ�Ԥ���ٶ�
																	GPS_Location.POS_Y,					//��ǰGPS�۲����
																	GPS_Location.Speed.y,				//��ǰGPS�۲��ٶ�
																	ACC_Earth.y,						//��ǰ���ٶȼƹ۲���ٶ�
																	Position.Position_xyz.y,		//Ԥ��������
																	Position.Speed.y,						//Ԥ�����ٶ�
																	Time_Ms/1000);							//����ʱ��						s
		Position.Acc.y = ACC_Earth.y;	//cm/s2
	}
}

void Position_Updata(u16 Time_Ms)
{
	Vector ACC_Earth;
	ACC_Earth = Math.Body_To_Earth(MPU6050.Data->ACC_ADC,Attitude.Angle->z,Attitude.Angle->y,Attitude.Angle->x);
	
	ACC_Earth.x /= 4095;
	ACC_Earth.x *= 980.0f;//cm/s2
	
	ACC_Earth.y /= 4095;
	ACC_Earth.y *= 980.0f;//cm/s2
	
	ACC_Earth.z -= 4095;
	ACC_Earth.z /= 4095;
	ACC_Earth.z *= 980.0f;//cm/s2
	
//--------------�߶��ں�--------------------------------------------------------------------//
	Altitude_Update(Time_Ms,ACC_Earth);
//--------------ˮƽλ���ں�--------------------------------------------------------------------//
	XY_Update(Time_Ms,ACC_Earth);
	
}





