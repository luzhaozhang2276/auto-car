#include "Math_User.h"

double Constrain(double AimN, double Limit_Up,double Limit_Down);
float Dead_Zone(float AimN,float Limit);
Vector Body_To_Earth(Vector Body,float Yaw,float Roll,float Pitch);
Vector XY_Coordinate_Rotate(float X,float Y,float Angle);
struct Math_ Math =
{
	Constrain,
	Dead_Zone,
	Body_To_Earth,
	XY_Coordinate_Rotate
};


double Constrain(double AimN, double Limit_Up,double Limit_Down)
{
	if(AimN > Limit_Up) return Limit_Up;
	else
		if(AimN < Limit_Down) return Limit_Down;
		else return AimN;
}

float Dead_Zone(float AimN,float Limit)
{
	if(fabsf(AimN) < Limit) return 0;
	else return AimN;
}

/*
	����ϵ��ϵ˵����
	��������ϵ����ͷ-Y����β+Y,��+X,��-X/�ϱ����������Ҷ�/��ͷָ������ΪĬ�ϳ�ʼ״̬
							Yaw   �� ������ϵZ���˶�   z
	            Roll �� ������ϵY���˶�		y
	            Pitc  �� ������ϵX���˶�		x
	�ο�����ϵ������������ϵ  

	��Z-Y-X˳����:
	��������ϵ����������ϵ����ת����R(b2n)
	R(b2n) =R(��)^T*R(��)^T*R(��)^T
	R=
		 {cos��*cos��     -cos��*sin��+sin��*sin��*cos��        sin��*sin��+cos��*sin��*cos��}
		 {cos��*sin��     cos��*cos�� +sin��*sin��*sin��       -cos��*sin��+cos��*sin��*sin��}
		 {-sin��          cos��sin ��                          cos��cos��    								}

PSI     ��
Theta  	��
Phi			��
*/

Vector Body_To_Earth(Vector Body,float Yaw,float Roll,float Pitch)
{
	Vector Earth;
	Yaw  *= DEG_TO_RAD;
	Roll *= DEG_TO_RAD;
	Pitch*= DEG_TO_RAD;
	
	float COS_PSI   = arm_cos_f32(Yaw);
	float COS_Theta = arm_cos_f32(Roll);
	float COS_Phi   = arm_cos_f32(Pitch);
	float SIN_PSI   = arm_sin_f32(Yaw);	
	float SIN_Theta = arm_sin_f32(Roll);
	float SIN_Phi	  = arm_sin_f32(Pitch);
	
	Earth.x =  COS_PSI * COS_Theta * Body.x + (-COS_Phi * SIN_PSI + SIN_Phi * SIN_Theta * COS_PSI) * Body.y + ( SIN_PSI * SIN_Phi + COS_Phi * SIN_Theta * COS_PSI) * Body.z;
	Earth.y =  COS_Theta * SIN_PSI * Body.x + ( COS_Phi * COS_PSI + SIN_Phi * SIN_Theta * SIN_PSI) * Body.y + (-COS_PSI * SIN_Phi + COS_Phi * SIN_Theta * SIN_PSI) * Body.z;
	Earth.z = -SIN_Theta 					 * Body.x + 																 COS_Theta * SIN_Phi * Body.y + 																 COS_Theta * COS_Phi * Body.z;
	return Earth;
};

//Vector Body_To_Earth(Vector Body,float Pitch,float Roll)
//{
//	Vector Earth;
//	Pitch *= DEG_TO_RAD;
//	Roll *= DEG_TO_RAD;
//	float COS_Phi   = arm_cos_f32(Roll);
//	float COS_Theta = arm_cos_f32(Pitch);
//	float SIN_Phi   = arm_sin_f32(Roll);
//	float SIN_Theta = arm_sin_f32(Pitch);
//	
//	Earth.x =  COS_Theta * Body.x + SIN_Phi * SIN_Theta * Body.y + COS_Phi * SIN_Theta * Body.z;
//	Earth.y =                                 COS_Phi   * Body.y -             SIN_Phi * Body.z;
//	Earth.z = -SIN_Theta * Body.x + SIN_Phi * COS_Theta * Body.y + COS_Phi * COS_Theta * Body.z;
//	return Earth;
//};

Vector XY_Coordinate_Rotate(float X,float Y,float Angle)
{
	Vector Earth;
	Angle *= DEG_TO_RAD;
	float COS = arm_cos_f32(Angle);
	float SIN = arm_sin_f32(Angle);
	
	Earth.x =  COS * X +  SIN * Y;
	Earth.y = -SIN * X +  COS * Y;
	Earth.z = 0;
	
	return Earth;
};

float To_180_degrees(float x)
{
	return (x>180?(x-360):(x<-180?(x+360):x));
}

//��֤����ֵ����Ч��
float Safe_Asin(float v)
{
    if (isnan(v)) {
        return 0.0f;
    }
    if (v >= 1.0f) {
        return M_PI/2;
    }
    if (v <= -1.0f) {
        return -M_PI/2;
    }
    return asinf(v);
}
//�Ƕ�ת����
float Radians(float deg) 
	{
	return deg * DEG_TO_RAD;
}

//����ת�Ƕ�
float Degrees(float rad) 
	{
	return rad * RAD_TO_DEG;
}

