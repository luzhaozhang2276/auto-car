#include "Attitude.h"
#define Kp 1.5f
#define Ki 0.0f
#define Z_FILTER 0.01f //������һ�׻����˲�ϵ��

Quaternion Q;//,Qz;
u8 IsCalulate = True;
struct Vector Angle;
struct Vector Rate;

/*
	����ϵ��ϵ˵����
	��������ϵ����MPU6050оƬ������ϵһ��  -> PWM����ӿڷ����ǻ�ͷ����
	            ˳�������᷽��ȥ��˳ʱ�뷽��Ϊ������
	            Pitch �� ������ϵY���˶�		Vector Angle.y
	            Roll  �� ������ϵX���˶�		Vector Angle.x
							Yaw   �� ������ϵZ���˶�   Vector Angle.z
	�ο�����ϵ������������ϵ  

*/

void Attitude_Init(void);
void Updata_Eular(void);

struct Attitude_ Attitude = 
{
	&Rate,
  &Angle,
	Attitude_Init,
	Updata_Eular,
};
void Attitude_Init(void)
{
	MPU6050.Init();
	HMC5883.Init();
}
void Updata_Quaternion(Vector GYR,Vector ACC,Vector MAG,double DltaT_S)
{
	float Norm;
	double HalfT = DltaT_S / 2.0f;
	double vx,vy,vz;
	double ex,ey,ez;
	double gx,gy,gz;
	double ax,ay,az;
	static double exInt = 0, eyInt = 0, ezInt = 0;//������̬�������Ļ���
	
	Vector Mxyz;			//�����Ʋ���ֵ
	Vector MAG_Earth;
	double MAG_Z_angle;
	
	/***************************************************
	����gx��gy��gz�ֱ��Ӧ������Ľ��ٶȣ���λ�ǻ���/��
������ax��ay��az�ֱ��Ӧ������ļ��ٶ�ԭʼ����
	***************************************************/
	
	ax = ACC.x;
	ay = ACC.y;
	az = ACC.z;

	gx = Radians(GYR.x * MPU6050_GYR_2000DPS);
	gy = Radians(GYR.y * MPU6050_GYR_2000DPS);
	gz = Radians(GYR.z * MPU6050_GYR_2000DPS);	
	
	Attitude.Rate->x = gx;
	Attitude.Rate->y = gy;
	Attitude.Rate->z = gz;
	
	
	//�����ٶȵ�ԭʼ���ݣ���һ�����õ���λ���ٶ�
	arm_sqrt_f32(ax * ax + ay * ay + az * az,&Norm);
	if(Norm == 0) return;
	ax = ax / Norm; 
	ay = ay / Norm;
	az = az / Norm;
	
	/**************************************************
	����Ԫ������ɡ��������Ҿ����еĵ����е�����Ԫ�ء�
	�������Ҿ����ŷ���ǵĶ��壬��������ϵ������������
	ת����������ϵ��������������Ԫ�ء����������vx��vy��vz��
	��ʵ���ǵ�ǰ�Ļ����������ϵ�ϣ����������������λ������
	(�ñ�ʾ������̬����Ԫ�����л���)
	***************************************************/
	vx = 2.0f * (Q.q2 * Q.q4 - Q.q1 * Q.q3);
	vy = 2.0f * (Q.q1 * Q.q2 + Q.q3 * Q.q4);
	vz = 1.0f - 2.0f * ( Q.q2 * Q.q2 + Q.q3 * Q.q3);//Q.w * Q.w + Q.z * Q.z;
	
	/***************************************************
	���������������������(Ҳ����������)����ʾ��	ex��
	ey��ez�����������������Ĳ���������������Ծ���λ�ڻ���
	����ϵ�ϵģ������ݻ������Ҳ���ڻ�������ϵ�����Ҳ���Ĵ�
	С�����ݻ����������ȣ����������������ݡ����������ǶԻ�
	��ֱ�ӻ��֣����Զ����ݵľ�������ֱ�������ڶԻ�������ϵ��
	������
	***************************************************/
	ex = (ay * vz - az * vy);
	ey = (az * vx - ax * vz);
	ez = (ax * vy - ay * vx);
	
	/***************************************************
	�ò���������PI����������ƫ��ͨ������Kp��Ki������������
	�Կ��Ƽ��ٶȼ����������ǻ�����̬���ٶ�
	***************************************************/
	if(Ki > 0)
	{
		exInt = exInt + ex * Ki;
		eyInt = eyInt + ey * Ki;
		ezInt = ezInt + ez * Ki;
		gx = gx + Kp * ex + exInt;
		gy = gy + Kp * ey + eyInt;
		gz = gz + Kp * ez + ezInt;
	}
	else
	{
		gx = gx + Kp * ex;
		gy = gy + Kp * ey;
		gz = gz + Kp * ez;   
	}
	
	//��Ԫ��΢�ַ��� 
	Q.q1 += (-Q.q2 * gx - Q.q3 * gy - Q.q4 * gz) * HalfT;
	Q.q2 += ( Q.q1 * gx + Q.q3 * gz - Q.q4 * gy) * HalfT;
	Q.q3 += ( Q.q1 * gy - Q.q2 * gz + Q.q4 * gx) * HalfT;
	Q.q4 += ( Q.q1 * gz + Q.q2 * gy - Q.q3 * gx) * HalfT;

	//��Ԫ����λ��
	arm_sqrt_f32(Q.q1 * Q.q1 + Q.q2 * Q.q2 + Q.q3 * Q.q3 + Q.q4 * Q.q4,&Norm);

	if(Norm == 0) return;
	
	Q.q1 = Q.q1 / Norm;
	Q.q2 = Q.q2 / Norm;
	Q.q3 = Q.q3 / Norm;
	Q.q4 = Q.q4 / Norm;
	
	Angle.x = Degrees(atan2f(2.0f*(Q.q1*Q.q2 + Q.q3*Q.q4),1 - 2.0f*(Q.q2*Q.q2 + Q.q3*Q.q3)));
	Angle.y = Degrees(Safe_Asin(2.0f*(Q.q1*Q.q3 - Q.q2*Q.q4)));
	//Angle.z = Degrees(atan2f(2.0f*(Q.q2*Q.q3 + Q.q1*Q.q4),1 - 2.0f*(Q.q3*Q.q3 + Q.q4*Q.q4)));	
	
//--------------�������ں�z��--------------------------------------------------------------------------------------//	

	
	Angle.z += Degrees(gz*DltaT_S);																//�����ǻ���Z�Ƕ�
	if(Angle.z > 180)
			Angle.z = -180;
	if(Angle.z < -180)
			Angle.z = 180;	
	
	Mxyz = MAG;
	arm_sqrt_f32(Mxyz.x*Mxyz.x + Mxyz.y*Mxyz.y + Mxyz.z*Mxyz.z,&Norm);       //mag���ݹ�һ��
		if(Norm == 0) return;//����ֵ�˳��������������
	Mxyz.x = Mxyz.x / Norm;
	Mxyz.y = Mxyz.y / Norm;
	Mxyz.z = Mxyz.z / Norm;		
	
	if(HMC5883.IsSensorError == False)													//������������������һ�׻����˲�
	{ 
		MAG_Earth = Math.Body_To_Earth(MAG,0,Angle.y ,Angle.x);				//����������ת�� ����->����
  	MAG_Z_angle  = Degrees(atan2f(MAG_Earth.x,MAG_Earth.y));		//�����Ƽ���Ƕ�
		if(MAG_Z_angle - Angle.z > 180 )
			Angle.z = (1- Z_FILTER) * Angle.z + Z_FILTER * (MAG_Z_angle - 360);
		else if(MAG_Z_angle - Angle.z < -180 )
			Angle.z = (1- Z_FILTER) * Angle.z + Z_FILTER * (MAG_Z_angle + 360);
		else if(abs(MAG_Z_angle - Angle.z) < 180 )
			Angle.z = (1- Z_FILTER) * Angle.z + Z_FILTER * MAG_Z_angle;
	}
	if(Angle.z > 180)
			Angle.z = -180;
	if(Angle.z < -180)
			Angle.z = 180;	
}
//	Angle.y = Degrees(atan2f(2.0f*(Q.q1*Q.q3 - Q.q2*Q.q4),1 - 2.0f*(Q.q3*Q.q3 + Q.q4*Q.q4)));
//	Angle.z = Degrees(Safe_Asin(2.0f*(Q.q2*Q.q3 + Q.q1*Q.q4)));
//	Angle.x = Degrees(atan2f(2.0f*(Q.q1*Q.q2 - Q.q3*Q.q4),1 - 2.0f*(Q.q2*Q.q2 + Q.q4*Q.q4)));	
	
//	Angle.z = 0;
//	//˫ŷ����ȫ��̬����
//	if(abs(Angle.y) <= 45)
//	{

//	}
//	else
//	{
//		Angle.y = Degrees(atan2f(2.0f*(Q.q2*Q.q3 - Q.q1*Q.q4),1 - 2.0f*(Q.q3*Q.q3 + Q.q4*Q.q4)));
//		Angle.x = Degrees(atan2f(2.0f*(Q.q3*Q.q4 - Q.q1*Q.q2),1 - 2.0f*(Q.q2*Q.q2 + Q.q3*Q.q3)));
//		Angle.z = Degrees(Safe_Asin(2.0f*(Q.q1*Q.q3 + Q.q2*Q.q4)));
//	}
/***************************************************
* ����ŷ����
* ÿ��2MS����һ����̬��
***************************************************/

void Updata_Eular(void)
{
	double DltaT;
	uint64_t Time_Now = 0;
	static uint64_t Time_Pre = 0;
	
	Time_Now = SystemTime.Now_US();
	DltaT = (Time_Now - Time_Pre) * (double)1e-6;
	Time_Pre = Time_Now;
	
	Updata_Quaternion(MPU6050.Data->GYR_ADC,MPU6050.Data->ACC_ADC,HMC5883.Data->MAG_ADC,DltaT);	

}





