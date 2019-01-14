#include "Filter.h"

Filter_2nd:: Filter_2nd(void)
{
	b0 = 0;
	b1 = 0;
	b2 = 0;
//a0 = 1;
	a1 = 0;
	a2 = 0;

	LastIn.x = 0;
	LastIn.y = 0;
	LastIn.z = 0;
	
	PreOut.x = 0;
	PreOut.y = 0;
	PreOut.z = 0;
	
	LastOut.x = 0;
	LastOut.y = 0;
	LastOut.z = 0;
}

Filter_2nd:: Filter_2nd(float Init_b0,float Init_b1,float Init_b2,float Init_a1,float Init_a2)
{
	b0 = Init_b0;
	b1 = Init_b1;
	b2 = Init_b2;
//a0 = 1;
	a1 = Init_a1;
	a2 = Init_a2;

	LastIn.x = 0;
	LastIn.y = 0;
	LastIn.z = 0;
	
	PreOut.x = 0;
	PreOut.y = 0;
	PreOut.z = 0;
	
	LastOut.x = 0;
	LastOut.y = 0;
	LastOut.z = 0;
}
/*
	�����˲����Ļ�����ַ���
*/

Vector Filter_2nd:: LPF2ndFilter(Vector Input)
{
	Vector NewOut;

	NewOut.x = b0 * Input.x + b1 * LastIn.x + b2 * PreIn.x -  a1 * LastOut.x - a2 * PreOut.x ;
	NewOut.y = b0 * Input.y + b1 * LastIn.y + b2 * PreIn.y -  a1 * LastOut.y - a2 * PreOut.y;
	NewOut.z = b0 * Input.z + b1 * LastIn.z + b2 * PreIn.z -  a1 * LastOut.z - a2 * PreOut.z;

	PreIn = LastIn;
	LastIn = Input;
	
	PreOut = LastOut;
	LastOut = NewOut;
	
	return NewOut;
		
}

float Filter_2nd:: LPF2ndFilter(float Input)
{
	float NewOut;

	NewOut = b0 * Input + b1 * LastIn.x + b2 * PreIn.x -  a1 * LastOut.x - a2 * PreOut.x ;

	PreIn.x = LastIn.x;
	LastIn.x = Input;
	PreOut.x = LastOut.x;
	LastOut.x = NewOut;

	
	return NewOut;
		
}

/*
	ð������
*/
float Filter_MidValue::Bubble(float Input[3])
{
	
	if(Input[0] < Input[1])
	{
		float buf = Input[0];
		Input[0] = Input[1];
		Input[1] = buf;
		
		if(Input[1] < Input[2])
		{
			float buf = Input[1];
			Input[1] = Input[2];
			Input[2] = buf;
			
			if(Input[0] < Input[1])
			{
				float buf = Input[0];
				Input[0] = Input[1];
				Input[1] = buf;
			}
		}
	}
	else
	{
		if(Input[1] < Input[2])
		{
			float buf = Input[1];
			Input[1] = Input[2];
			Input[2] = buf;
			
			if(Input[0] < Input[1])
			{
				float buf = Input[0];
				Input[0] = Input[1];
				Input[1] = buf;
			}
		}
	}
	
	return Input[1];
}
/*
	��ֵ�˲�
*/
Vector Filter_MidValue::MidValue(Vector Input)
{
	Vector Output;
	float Buf[3];
	
	Data[Cnt++] = Input;
	
	if(Cnt == 3) Cnt = 0;
	
	Buf[0] = Data[0].x;
	Buf[1] = Data[1].x;
	Buf[2] = Data[2].x;
	
	Output.x = Bubble(Buf);
	
	Buf[0] = Data[0].y;
	Buf[1] = Data[1].y;
	Buf[2] = Data[2].y;
	
	Output.y = Bubble(Buf);
	
	Buf[0] = Data[0].z;
	Buf[1] = Data[1].z;
	Buf[2] = Data[2].z;
	
	Output.z = Bubble(Buf);
	
	return Output;
}

float Filter_MidValue::MidValue(float Input)
{
	float Output;
	float Buf[3];
	
	Data1[Cnt++] = Input;
	
	if(Cnt == 3) Cnt = 0;
	
	Buf[0] = Data1[0];
	Buf[1] = Data1[1];
	Buf[2] = Data1[2];
	
	Output = Bubble(Buf);
	
	return Output;
}


float Filter_Fir::FirFilter(float Input)
{
	int i = 0;
	int j = 0;
	float Output = 0;
	Data[Cnt] = Input;

	for(i = 0;i < N;i++)
	{
		if(Cnt < i) j = N - i + Cnt; 
		else j = Cnt - i;
		Output += Data[j] * Para[i];
	}
	
	if(Cnt < N - 1) Cnt++;
	else
		Cnt = 0;	
	
	return Output; 
}

//�����˲� -> ���磺���ٶȲ���ֵ���ٶȲ���ֵ��ͬ �����ٶ�
//Inputһ�λ��ֽ��Ϊ�������
//Input��Measurement����

float Filter_Balance::BalanceFilter(float Input,float Measurement,double dt)
{
	float Error,P_Error,I_Error;
	
	Error = Measurement - Output;
	P_Error = Error * Kp;
	
	if(Ki != 0)
		I_Error += Error * Ki;
	
	Input += P_Error;
	Input += I_Error;
		
	Output += Input * dt;
	
	return Output;
}
/*
@���۲⿨����
Model			״̬1�ϴ�Ԥ��ֵ  
Input 		״̬1��ǰ�۲�ֵ	
*/
float Filter_EKF_Single::EKFFilter_Single(float Model,float Input)
{
	X_k_k1 = Model;
	P_k_k1 = P_k_k + Q;
	X_k_k  = X_k_k1 + Kg * (Input - X_k_k1);
	Kg     = P_k_k1 / (P_k_k1 + R);
	P_k_k  = (1 - Kg) * P_k_k1;
	
	return X_k_k;
}

/*
@˫�۲⿨����
Model			״̬1�ϴ�Ԥ��ֵ  �ϴ�Ԥ������
Model_D		״̬2�ϴ�Ԥ��ֵ	�ϴ�Ԥ���ٶ�
Input 		״̬1��ǰ�۲�ֵ	��ǰ�۲����
Input_D  	״̬2��ǰ�۲�ֵ	��ǰ�۲��ٶ�
Input_DD  ��ǰ������				��ǰ�۲���ٶ�
Out  			״̬1��ǰԤ��ֵ 	Ԥ������
Out_D			״̬2��ǰԤ��ֵ 	Ԥ���ٶ�
Dt				����ʱ�� 
*/
void Filter_EKF_Double::EKFFilter_Double(float Model,float Model_D ,float Input,float Input_D,float Input_DD,float &Out,float &Out_D,float Dt)
{
	//ʱ����� ����״̬
	Out += Out_D*Dt + 0.5*Input_DD*Dt*Dt;
	Out_D += Input_DD*Dt;
	//ʱ����� ����Э����
	P[0] = Pre_P[0] + Pre_P[2] * Dt + (Pre_P[1] + Pre_P[3] * Dt) * Dt + Q[0];
	P[1] = Pre_P[1] + Pre_P[3] * Dt;
	P[2] = Pre_P[2] + Pre_P[3] * Dt;
	P[3] = Pre_P[3] + Q[1];
	//�۲���� ���㿨��������
	float K_Denominator = 1.0 / ((P[0]+ R[0]) * (P[3] + R[1]) - P[1] * P[2]);
	Kg[0] = (P[0] * (P[3] + R[1]) - P[1] * P[2]) * K_Denominator;
	Kg[1] = (P[1] * R[0]) * K_Denominator;
	Kg[2] = (P[2] * R[1]) * K_Denominator;
	Kg[3] = (P[2] * (-P[1]) + P[3] *(P[0] + R[0])) * K_Denominator;
	//�۲���� �ں��������
	float Out_Err 	= Input - Model;
	float Out_D_Err = Input_D - Model_D;
	Out 	+= Kg[0] * Out_Err + Kg[1] * Out_D_Err;
	Out_D += Kg[2] * Out_Err + Kg[3] * Out_D_Err;
	//�۲���� ����״̬Э�������
	Pre_P[0] = (1 - Kg[0]) * P[0] - Kg[1] * P[2];
	Pre_P[1] = (1 - Kg[0]) * P[1] - Kg[1] * P[3];
	Pre_P[2] = (1 - Kg[3]) * P[2] - Kg[2] * P[0];
	Pre_P[3] = (1 - Kg[3]) * P[3] - Kg[2] * P[1];
}


