#include "FlyControl.h"

#define ATT_ANGLE_SPEED_RC_Z_MAX  	20    						 		//ң��Z���ת���ٶ�
#define ATT_ANGLE_SPEED      				(45.0f * DEG_TO_RAD)  //���ת���ٶ�
#define ATT_ANGLE_MAX        				20.0f                 //������

#define ATT_FILTER_ANGLE   	 				0.1f                  //�ǶȻ�ǰ���˲���ϵ��
#define ATT_FILTER_SPEED     				0.1f                  //���ٶȻ�ǰ���˲�ϵ��

#define POS_POS_SET_MAX_XY       		1000.0f              //XY������ cm
#define POS_SPEED_SET_MAX_XY      	100.0f               //XY����ٶ� cm/s
#define POS_ACC_SET_MAX_XY        	100.0f               //XY�����ٶ� cm/s
#define POS_OUT_MAX_XY        			3.0f               //XY������  �Ƕ�ֵ

#define POS_POS_FEEBACK_MAX_XY      (1.5*POS_POS_SET_MAX_XY) //XY��������� cm
#define POS_SPEED_FEEBACK_MAX_XY    100.0f                    //XY������ٶ� cm/s
#define POS_ACC_FEEBACK_MAX_XY      100.0f                   //XY��������ٶ� cm/s

#define POS_POS_SET_MAX_Z        		500.0f                   //Z���߶� cm
#define POS_SPEED_SET_MAX_Z      		40.0f                    //Z����ٶ� cm/s
#define POS_ACC_SET_MAX_Z        		60.0f                    //Z�����ٶ� cm/s
#define POS_OUT_MAX_Z        				THROTTLE_60_PERCENT         //Z������

#define POS_POS_FEEBACK_MAX_Z       (1.5*POS_POS_SET_MAX_Z)  //Z������߶� cm
#define POS_SPEED_FEEBACK_MAX_Z     40.0f                    //Z������ٶ� cm/s
#define POS_ACC_FEEBACK_MAX_Z       100.0f                   //Z��������ٶ� cm/s

#define POS_FILTER_POS   	  				0.1f                    //λ��ǰ���˲���ϵ��
#define POS_FILTER_SPEED     				0.1f                    //�ٶȻ�ǰ���˲�ϵ��
#define POS_FILTER_ACC       				0.8f                    //���ٶȻ�ǰ���˲�ϵ��

#define THROTTLE_MIN PWM_RC_MIN
#define THROTTLE_MAX PWM_RC_MAX
#define THROTTLE_MID ((PWM_RC_MAX + PWM_RC_MIN) / 2)
#define THROTTLE_60_PERCENT ((THROTTLE_MAX - THROTTLE_MIN) * 0.6f)

void ATT_Inner_Loop(u32 Time);
void ATT_Outer_Loop(u32 Time);
void POS_Acc_Loop(u32 Time);
void POS_Inner_Loop(u32 Time);
void POS_Outer_Loop(u32 Time);

/*
	PID ATT_Inner_PID_x;
	PID ATT_Inner_PID_y;
	PID ATT_Inner_PID_z;
	
	PID ATT_Outer_PID_x;
	PID ATT_Outer_PID_y;	
	PID ATT_Outer_PID_z;	
	
	PID POS_Inner_PID_x;
	PID POS_Inner_PID_y;
	PID POS_Inner_PID_z;
	
	PID POS_Outer_PID_x;
	PID POS_Outer_PID_y;
	PID POS_Outer_PID_z;
	int Throttle;

	float ALT_Onland;
	BOOL IsLock;
	BOOL IsLost;
	BOOL IsError;
	Fly_Mode Mode;  
*/
struct Control_Para_ Control_Para = 
{
//	PID(160,20,10,100),
//	PID(160,20,10,100),
//	PID(400,0,0,0),
//	
//	PID(15,0,0,0),
//	PID(15,0,0,0),
//	PID(1,0,0,0),
//	
//	PID(0,0,0,0),
//	PID(0,0,0,0),
//	PID(1,0,0,0,Filter_2nd(0.0009446918438402,0.00188938368768,0.0009446918438402,-1.911197067426,0.9149758348014)),	//����Ƶ��200HZ ��ֹƵ�� 2HZ 
//	
//	PID(0,0,0,0),
//	PID(0,0,0,0),
//	PID(1,0,0,0,Filter_2nd(0.0009446918438402,0.00188938368768,0.0009446918438402,-1.911197067426,0.9149758348014)),	//����Ƶ��200HZ ��ֹƵ�� 2HZ 
//	
//	PID(0,0,0,0),
//	PID(0,0,0,0),
//	PID(1,0,0,0),


	PID(95,40,6,100),
	PID(90,40,5,100),
	PID(400,0,0,0),
	
	PID(10,0,0,0),
	PID(10,0,0,0),
	PID(1,0,0,0),
	
	PID(0.003,0.01,0,10,Filter_2nd(0.00015514842347569903,0.00031029684695139806,0.00015514842347569903,-1.964460580205232,0.96508117389913495)),	//����Ƶ��500HZ ��ֹ2HZ 
	PID(0.003,0.01,0,10,Filter_2nd(0.00015514842347569903,0.00031029684695139806,0.00015514842347569903,-1.964460580205232,0.96508117389913495)),	//����Ƶ��500HZ ��ֹ2HZ 
	PID(0.8,1.5,0,600,Filter_2nd(0.00015514842347569903,0.00031029684695139806,0.00015514842347569903,-1.964460580205232,0.96508117389913495)),	//����Ƶ��500HZ ��ֹ2HZ 

	PID(0.2,0,0.0008,0,Filter_2nd(0.06745527388907,0.1349105477781,0.06745527388907,-1.14298050254,0.4128015980962)),	//����Ƶ��200HZ ��ֹƵ�� 20HZ 
	PID(0.2,0,0.0008,0,Filter_2nd(0.06745527388907,0.1349105477781,0.06745527388907,-1.14298050254,0.4128015980962)),	//����Ƶ��200HZ ��ֹƵ�� 20HZ 
	PID(2,0,0.1,0,Filter_2nd(0.06745527388907,0.1349105477781,0.06745527388907,-1.14298050254,0.4128015980962)),	//����Ƶ��200HZ ��ֹƵ�� 20HZ 

	PID(1,0,0,0),
	PID(1,0,0,0),
	PID(1,0,0,0),

	THROTTLE_MIN,
	
	0,
	
	Vector(0,0,0),
	True,
	False,
	False,
	NO_HEAD,
	ALT
};

struct FlyControl_ FlyControl = 
{	
	&Control_Para,
	
	ATT_Inner_Loop,
	ATT_Outer_Loop,
	
	POS_Acc_Loop,
	POS_Inner_Loop,
	POS_Outer_Loop,	
};

/*
	�����Ƶ�λ 
*/
Filter_2nd Rate_Filter(0.0036216815149286421f,0.0072433630298572842f,0.0036216815149286421f,-1.8226949251963083f,0.83718165125602262f);
void ATT_Inner_Loop(u32 Time)
{
	Vector Rate_filter_Temp;
	Vector Inner_Output;
	if(Control_Para.IsLock == True)            //ͣ�� ��λ
	{
		Control_Para.ATT_Inner_PID_x.Rst_I();
		Control_Para.ATT_Inner_PID_y.Rst_I();
		Control_Para.ATT_Inner_PID_z.Rst_I();
		Motor.Stop();
		return;
	}

	//Rate_filter_Temp = Rate_Filter.LPF2ndFilter(*Attitude.Rate);
	//�����Ƕ�ȡ��������ֱ�������ڻ�����
	Control_Para.ATT_Inner_PID_x.Feedback = Attitude.Rate->x;
	Control_Para.ATT_Inner_PID_y.Feedback = Attitude.Rate->y;
	Control_Para.ATT_Inner_PID_z.Feedback = Attitude.Rate->z;
		
	Inner_Output.x = Control_Para.ATT_Inner_PID_x.Cal_PID_POS(Time);
	Inner_Output.y = Control_Para.ATT_Inner_PID_y.Cal_PID_POS(Time);
 	Inner_Output.z = Control_Para.ATT_Inner_PID_z.Cal_PID_POS(Time);
		
	//�������
	Motor.PWM->PWM1 = - Inner_Output.x -  Inner_Output.y + Inner_Output.z + Control_Para.Throttle;	
	Motor.PWM->PWM2 = - Inner_Output.x +  Inner_Output.y - Inner_Output.z + Control_Para.Throttle;
	Motor.PWM->PWM3 = + Inner_Output.x +  Inner_Output.y + Inner_Output.z + Control_Para.Throttle; 
	Motor.PWM->PWM4 = + Inner_Output.x -  Inner_Output.y - Inner_Output.z + Control_Para.Throttle;
	
	Motor.Output(True);
}
/*
	�����Ƶ�λ 
*/
void ATT_Outer_Loop(u32 Time)
{
	double ATT_Outer_PID_z_Feedback_2_Setpoint = 0;
	Vector ATT_XY_RC;
	Vector Outer_Output;
	float Z_Angle_Change;
	
	if(Control_Para.IsLock == True) 
	{
		Control_Para.ATT_Outer_PID_x.Rst_I();
		Control_Para.ATT_Outer_PID_y.Rst_I();
		Control_Para.ATT_Outer_PID_z.Rst_I();
		Control_Para.Home_Z_Angle = Attitude.Angle->z;
		Control_Para.ATT_Outer_PID_z.Setpoint = Radians(Control_Para.Home_Z_Angle);	 //����ʱƫ���Ǳ���
		return;
	}
	//-----------XY��̬����-----------------------------------------------------//	
	if(Control_Para.Mode == ATT || Control_Para.Mode == ALT)  //��̬ģʽ�Ͷ���ģʽ 
	{
		ATT_XY_RC.x = Radians(Math.Dead_Zone(PWM_RC_F_B - PWM_RC_MID,10) / PWM_RC_RANGE * ATT_ANGLE_MAX);
		ATT_XY_RC.y = Radians(Math.Dead_Zone(PWM_RC_L_R - PWM_RC_MID,10) / PWM_RC_RANGE * ATT_ANGLE_MAX);
		//Xǰ����ת Y������ת ��ͷģʽң����ת��	
		if(Control_Para.Head_Mode == NO_HEAD)  //��ͷģʽ�½���XY��̬������ת��
		{	//�������ӻ���ԭʼ����ϵת����ǰ��������ϵ
			Z_Angle_Change = Attitude.Angle->z - Control_Para.Home_Z_Angle;
		}
		else if(Control_Para.Head_Mode == HEAD) 
		{//���������ֵ�ǰ��������ϵ
			Z_Angle_Change = 0;
		}
	}
	else if(Control_Para.Mode == POS)	//����ģʽ  
	{
		ATT_XY_RC.x = Control_Para.POS_Acc_PID_x.Output;
		ATT_XY_RC.y = Control_Para.POS_Acc_PID_y.Output;
		//Xǰ����ת Y������ת
		//�������ӵ�������ϵת����������ϵ
		Z_Angle_Change = Attitude.Angle->z;
	}
	
	if(Z_Angle_Change > 180 )	//180�Ƕ��������
		Z_Angle_Change = Z_Angle_Change - 360;
	else if(Z_Angle_Change < -180 )
		Z_Angle_Change = Z_Angle_Change + 360;
	else if(abs(Z_Angle_Change) < 180 )
		Z_Angle_Change = Z_Angle_Change;
	
	ATT_XY_RC = Math.XY_Coordinate_Rotate(ATT_XY_RC.x,ATT_XY_RC.y,Z_Angle_Change);//����������ϵת��		

	Control_Para.ATT_Outer_PID_x.Setpoint = (1 - ATT_FILTER_ANGLE) * Control_Para.ATT_Outer_PID_x.Setpoint + ATT_FILTER_ANGLE * ATT_XY_RC.x;
	Control_Para.ATT_Outer_PID_y.Setpoint = (1 - ATT_FILTER_ANGLE) * Control_Para.ATT_Outer_PID_y.Setpoint + ATT_FILTER_ANGLE * ATT_XY_RC.y;

	Control_Para.ATT_Outer_PID_x.Feedback = Radians(Attitude.Angle->x);
	Control_Para.ATT_Outer_PID_y.Feedback = Radians(Attitude.Angle->y);
	// PID���㼰�޷�
	Outer_Output.x = Math.Constrain(Control_Para.ATT_Outer_PID_x.Cal_PID_POS(Time),ATT_ANGLE_SPEED,-ATT_ANGLE_SPEED);
	Outer_Output.y = Math.Constrain(Control_Para.ATT_Outer_PID_y.Cal_PID_POS(Time),ATT_ANGLE_SPEED,-ATT_ANGLE_SPEED);
	// ���ƽ��
	Control_Para.ATT_Inner_PID_x.Setpoint = (1 - ATT_FILTER_SPEED) * Control_Para.ATT_Inner_PID_x.Setpoint + ATT_FILTER_SPEED * Outer_Output.x;
	Control_Para.ATT_Inner_PID_y.Setpoint = (1 - ATT_FILTER_SPEED) * Control_Para.ATT_Inner_PID_y.Setpoint + ATT_FILTER_SPEED * Outer_Output.y;	


	//------------Z��̬����-----------------------------------------------------//	
	Control_Para.ATT_Outer_PID_z.Feedback = Radians(Attitude.Angle->z);		
	ATT_Outer_PID_z_Feedback_2_Setpoint = Control_Para.ATT_Outer_PID_z.Feedback; //+-180�ȴ���ǰȡֵ����֤Ŀ��ֵΪ+-180��Χ
	
	//ƫ���ǿ�Խ+-180�ȴ���
	if((Control_Para.ATT_Outer_PID_z.Setpoint - Control_Para.ATT_Outer_PID_z.Feedback) > M_PI)
	{
		Control_Para.ATT_Outer_PID_z.Feedback = Control_Para.ATT_Outer_PID_z.Feedback + 2*M_PI;
	}
	else if((Control_Para.ATT_Outer_PID_z.Setpoint - Control_Para.ATT_Outer_PID_z.Feedback) < -M_PI)
	{
		Control_Para.ATT_Outer_PID_z.Feedback = Control_Para.ATT_Outer_PID_z.Feedback - 2*M_PI;
	}
	
	Outer_Output.z = Math.Constrain(Control_Para.ATT_Outer_PID_z.Cal_PID_POS(Time),ATT_ANGLE_SPEED,-ATT_ANGLE_SPEED);

	//���ݴ��������ƫ����
	PWM_In_POS Yaw_Control_Choose = PWM_In.POS_Judge(PWM_RC_Lr_Rr);	
	if(Yaw_Control_Choose != PWM_In_Mid )//����򲻽��нǶȿ��� ��ֱ���ô�������ƽ��ٶ�   ��������˳������ֻ���ʱ�Ƕ� 
	{
		Control_Para.ATT_Outer_PID_z.Setpoint = ATT_Outer_PID_z_Feedback_2_Setpoint;
		Control_Para.ATT_Inner_PID_z.Setpoint = (1 - ATT_FILTER_SPEED) * Control_Para.ATT_Inner_PID_z.Setpoint + ATT_FILTER_SPEED * Radians((float)Math.Dead_Zone(PWM_RC_MID - PWM_RC_Lr_Rr ,10) / PWM_RC_RANGE * ATT_ANGLE_SPEED_RC_Z_MAX);
	}
	else//�������ֱ���� �⻷�Ƕȿ�������� ���� ���ٶ�
	{
		Control_Para.ATT_Inner_PID_z.Setpoint = (1 - ATT_FILTER_SPEED) * Control_Para.ATT_Inner_PID_z.Setpoint + ATT_FILTER_SPEED * Outer_Output.z;
	}
}

void POS_Acc_Loop(u32 Time)
{
	//-------------Ԥ�ȴ���-------------------------------------------------------------------//
	if(Control_Para.IsLock == True)  
	{
		Control_Para.POS_Acc_PID_x.Rst_I();
		Control_Para.POS_Acc_PID_y.Rst_I();
		Control_Para.POS_Acc_PID_z.Rst_I();
		return;
	}
	
	//��̬ģʽ����ֱ��ȡ����ң�������źţ�����ģʽ����ȡ����PID������	
	if(Control_Para.Mode == ATT) 
	{
		//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
		//YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
		//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
		Control_Para.POS_Acc_PID_x.Rst_I();
		Control_Para.POS_Acc_PID_y.Rst_I();
		Control_Para.POS_Acc_PID_z.Rst_I();
		
		Control_Para.Throttle = (1 - POS_FILTER_ACC) * Control_Para.Throttle + POS_FILTER_ACC * 2 * (PWM_RC_D_U - PWM_RC_MID);		
		Control_Para.Throttle = Math.Constrain(Control_Para.Throttle,POS_OUT_MAX_Z,0);
		return;		
	}
	if(Control_Para.Mode == ALT) 
	{
		Control_Para.POS_Acc_PID_x.Rst_I();
		Control_Para.POS_Acc_PID_y.Rst_I();
	}
	else if(Control_Para.Mode == POS) 
	{
		//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
		//--------------pid����\ֱ�����------------------------------------------------------------------//
		Control_Para.POS_Acc_PID_x.Feedback = Math.Constrain( Position.Acc.x,POS_ACC_FEEBACK_MAX_XY,-POS_ACC_FEEBACK_MAX_XY);
		// PID���㼰�޷�
		Control_Para.POS_Acc_PID_x.Output = Math.Constrain(Control_Para.POS_Acc_PID_x.Cal_PID_POS_BT_LPF(Time),POS_OUT_MAX_XY,-POS_OUT_MAX_XY);
		
		//YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
		//--------------pid����\ֱ�����------------------------------------------------------------------//
		Control_Para.POS_Acc_PID_y.Feedback = Math.Constrain( Position.Acc.y,POS_ACC_FEEBACK_MAX_XY,-POS_ACC_FEEBACK_MAX_XY);
		// PID���㼰�޷�
		Control_Para.POS_Acc_PID_y.Output = Math.Constrain(Control_Para.POS_Acc_PID_y.Cal_PID_POS_BT_LPF(Time),POS_OUT_MAX_XY,-POS_OUT_MAX_XY);
	}
	
	if(Control_Para.Mode == ALT || Control_Para.Mode == POS) 
	{
		//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
		//--------------pid����\ֱ�����------------------------------------------------------------------//
		Control_Para.POS_Acc_PID_z.Feedback = Math.Constrain( Position.Acc.z,POS_ACC_FEEBACK_MAX_Z,-POS_ACC_FEEBACK_MAX_Z);
		// PID���㼰�޷�
		Control_Para.Throttle = Math.Constrain(Control_Para.POS_Acc_PID_z.Cal_PID_POS_BT_LPF(Time),POS_OUT_MAX_Z,0);
	}
}

void POS_Inner_Loop(u32 Time)
{
	Vector Inner_Output;
	//-------------Ԥ�ȴ���-------------------------------------------------------------------//
	if(Control_Para.IsLock == True)  
	{
		Control_Para.POS_Inner_PID_x.Rst_I();
		Control_Para.POS_Inner_PID_y.Rst_I();
		Control_Para.POS_Inner_PID_z.Rst_I();
		return;
	}
	//��̬ģʽ����ֱ��ȡ����ң�������źţ�����ģʽ����ȡ����PID������	
	if(Control_Para.Mode == ATT) 
	{
		Control_Para.POS_Inner_PID_x.Rst_I();	
		Control_Para.POS_Inner_PID_y.Rst_I();	
		Control_Para.POS_Inner_PID_z.Rst_I();		
		return;		
	}
	if(Control_Para.Mode == ALT) 
	{
		Control_Para.POS_Inner_PID_x.Rst_I();
		Control_Para.POS_Inner_PID_y.Rst_I();
	}
	else if(Control_Para.Mode == POS) //XY����
	{
		//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
		//--------------pid����------------------------------------------------------------------//
		Control_Para.POS_Inner_PID_x.Feedback = Math.Constrain(Position.Speed.x,POS_SPEED_FEEBACK_MAX_XY,-POS_SPEED_FEEBACK_MAX_XY);
		// PID���㼰�޷�
		Inner_Output.x =  Math.Constrain(Control_Para.POS_Inner_PID_x.Cal_PID_POS(Time),POS_ACC_SET_MAX_XY,-POS_ACC_SET_MAX_XY);	
		//--------------�������------------------------------------------------------------------//
		// ���ƽ��
		Control_Para.POS_Acc_PID_x.Setpoint = (1 - POS_FILTER_ACC) * Control_Para.POS_Acc_PID_x.Setpoint + POS_FILTER_ACC * Inner_Output.x;
		
		//YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
		//--------------pid����------------------------------------------------------------------//
		Control_Para.POS_Inner_PID_y.Feedback = Math.Constrain(Position.Speed.y,POS_SPEED_FEEBACK_MAX_XY,-POS_SPEED_FEEBACK_MAX_XY);
		// PID���㼰�޷�
		Inner_Output.y =  Math.Constrain(Control_Para.POS_Inner_PID_y.Cal_PID_POS(Time),POS_ACC_SET_MAX_XY,-POS_ACC_SET_MAX_XY);	
		//--------------�������------------------------------------------------------------------//
		// ���ƽ��
		Control_Para.POS_Acc_PID_y.Setpoint = (1 - POS_FILTER_ACC) * Control_Para.POS_Acc_PID_y.Setpoint + POS_FILTER_ACC * Inner_Output.y;
	}
	
	if(Control_Para.Mode == ALT || Control_Para.Mode == POS) //Z����
	{
		//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
		//--------------pid����------------------------------------------------------------------//
		Control_Para.POS_Inner_PID_z.Feedback = Math.Constrain(Position.Speed.z,POS_SPEED_FEEBACK_MAX_Z,-POS_SPEED_FEEBACK_MAX_Z);
		// PID���㼰�޷�
		Inner_Output.z =  Math.Constrain(Control_Para.POS_Inner_PID_z.Cal_PID_POS(Time),POS_ACC_SET_MAX_Z,-POS_ACC_SET_MAX_Z);	
		//--------------�������------------------------------------------------------------------//
		// ���ƽ��
		Control_Para.POS_Acc_PID_z.Setpoint = (1 - POS_FILTER_ACC) * Control_Para.POS_Acc_PID_z.Setpoint + POS_FILTER_ACC * Inner_Output.z;
	}
}

// �����ķ�ʽ���ڼ򵥣��ڷ��й�����Ҳ���������������͵���������ܼ򵥵ĸ�������ֵ����
void POS_Outer_Loop(u32 Time)
{
	Vector POS_XY_RC;
	Vector Outer_Output;
	float Z_Angle_Change;
	//-------------Ԥ�ȴ���-------------------------------------------------------------------//
	if(Control_Para.IsLock == True) 
	{
		Control_Para.POS_Outer_PID_x.Rst_I();
		Control_Para.POS_Outer_PID_y.Rst_I();
		Control_Para.POS_Outer_PID_z.Rst_I();

		Control_Para.Home = Position.Position_xyz;
		Control_Para.POS_Outer_PID_x.Setpoint = Control_Para.Home.x;		
		Control_Para.POS_Outer_PID_y.Setpoint = Control_Para.Home.y;	
		Control_Para.POS_Outer_PID_z.Setpoint = Control_Para.Home.z;	
		return;
	}
	//ʵʱ��¼��̬ģʽ�£��������߶ȣ���Ӧ���ڷ���������̬ģʽ�л�������ģʽ
	if(Control_Para.Mode == ATT) 
	{
		Control_Para.POS_Outer_PID_x.Setpoint = Position.Position_xyz.x;
		Control_Para.POS_Outer_PID_y.Setpoint = Position.Position_xyz.y;
		Control_Para.POS_Outer_PID_z.Setpoint = Position.Position_xyz.z;
		Control_Para.POS_Outer_PID_x.Rst_I();
		Control_Para.POS_Outer_PID_y.Rst_I();
		Control_Para.POS_Outer_PID_z.Rst_I();
		return;
	}
	
	if(Control_Para.Mode == ALT) 
	{
		Control_Para.POS_Outer_PID_x.Rst_I();
		Control_Para.POS_Outer_PID_y.Rst_I();
	}
	else if(Control_Para.Mode == POS) //XY����
	{
		//��ͷ\��ͷģʽת��
		if(Control_Para.Head_Mode == HEAD)
			Z_Angle_Change = Control_Para.Home_Z_Angle;
		else
			Z_Angle_Change = Attitude.Angle->z;
		// x ���ѡ�� �����ң��
		PWM_In_POS Position_X_Control_Choose = PWM_In.POS_Judge(PWM_RC_L_R);
		// y ���ѡ�� �����ң��
		PWM_In_POS Position_Y_Control_Choose = PWM_In.POS_Judge(PWM_RC_F_B);
		
		if(Position_X_Control_Choose != PWM_In_Mid)
		{
			POS_XY_RC.x = ((PWM_RC_L_R - PWM_RC_MID)/PWM_RC_RANGE*2*POS_SPEED_SET_MAX_XY); //�ٶȿ�����ȡֵ	
		}
		else
			POS_XY_RC.x = 0;
		
		if(Position_Y_Control_Choose != PWM_In_Mid)
		{
			POS_XY_RC.y = ((PWM_RC_F_B - PWM_RC_MID)/PWM_RC_RANGE*2*POS_SPEED_SET_MAX_XY);	//�ٶȿ�����ȡֵ
		}
		else
			POS_XY_RC.y = 0;
		POS_XY_RC = Math.XY_Coordinate_Rotate(POS_XY_RC.x,POS_XY_RC.y,Z_Angle_Change);//����������ϵת��	
		
		
		//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
		//--------------pid����------------------------------------------------------------------//
		// x PID����
		Control_Para.POS_Outer_PID_x.Setpoint = Math.Constrain(Control_Para.POS_Outer_PID_x.Setpoint,	Control_Para.Home.x + POS_POS_SET_MAX_XY,Control_Para.Home.x - POS_POS_SET_MAX_XY);//
		Control_Para.POS_Outer_PID_x.Feedback = Math.Constrain(Position.Position_xyz.x,Control_Para.Home.x + POS_POS_FEEBACK_MAX_XY,Control_Para.Home.x - POS_POS_FEEBACK_MAX_XY);
		Outer_Output.x = Math.Constrain(Control_Para.POS_Outer_PID_x.Cal_PID_POS(Time),POS_SPEED_SET_MAX_XY,-POS_SPEED_SET_MAX_XY);
		//--------------�������------------------------------------------------------------------//
		// x ���ѡ�� �����ң��
		if(POS_XY_RC.x != 0)
		{
			Control_Para.POS_Outer_PID_x.Setpoint = Position.Position_xyz.x; //�ٶȿ����µ�ǰ����ΪĿ����� == �޾������
			Control_Para.POS_Inner_PID_x.Setpoint = (1 - POS_FILTER_SPEED)*Control_Para.POS_Inner_PID_x.Setpoint + POS_FILTER_SPEED * POS_XY_RC.x; //�ٶȿ�����ȡֵ
		}
		else
		{
			Control_Para.POS_Inner_PID_x.Setpoint = (1 - POS_FILTER_SPEED)*Control_Para.POS_Inner_PID_x.Setpoint + POS_FILTER_SPEED * Outer_Output.x;
		}	
		//YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
		//--------------pid����------------------------------------------------------------------//
		// y PID����
		Control_Para.POS_Outer_PID_y.Setpoint = Math.Constrain(Control_Para.POS_Outer_PID_y.Setpoint,	Control_Para.Home.y + POS_POS_SET_MAX_XY,Control_Para.Home.y - POS_POS_SET_MAX_XY);//
		Control_Para.POS_Outer_PID_y.Feedback = Math.Constrain(Position.Position_xyz.y,Control_Para.Home.y + POS_POS_FEEBACK_MAX_XY,Control_Para.Home.y - POS_POS_FEEBACK_MAX_XY);
		Outer_Output.y = Math.Constrain(Control_Para.POS_Outer_PID_y.Cal_PID_POS(Time),POS_SPEED_SET_MAX_XY,-POS_SPEED_SET_MAX_XY);
		//--------------�������------------------------------------------------------------------//
		// y ���ѡ�� �����ң��
		if(POS_XY_RC.y != 0)
		{
			Control_Para.POS_Outer_PID_y.Setpoint = Position.Position_xyz.y;	//�ٶȿ����µ�ǰ����ΪĿ����� == �޾������
			Control_Para.POS_Inner_PID_y.Setpoint = (1 - POS_FILTER_SPEED)*Control_Para.POS_Inner_PID_y.Setpoint + POS_FILTER_SPEED * POS_XY_RC.y;	//�ٶȿ�����ȡֵ
		}
		else
		{
			Control_Para.POS_Inner_PID_y.Setpoint = (1 - POS_FILTER_SPEED)*Control_Para.POS_Inner_PID_y.Setpoint + POS_FILTER_SPEED * Outer_Output.y;
		}		
	}
	
	if(Control_Para.Mode == ALT || Control_Para.Mode == POS) //Z����
	{
		//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
		//--------------pid����------------------------------------------------------------------//
		// Z PID����
		Control_Para.POS_Outer_PID_z.Setpoint = Math.Constrain(Control_Para.POS_Outer_PID_z.Setpoint,	Control_Para.Home.z + POS_POS_SET_MAX_Z,Control_Para.Home.z);// ���и߶��޷�  ��̬ģʽ���ɷɸߣ���̬ģʽ�л�������ģʽ��������Σ�գ�
		Control_Para.POS_Outer_PID_z.Feedback = Math.Constrain(Position.Position_xyz.z,Control_Para.Home.z + POS_POS_FEEBACK_MAX_Z,Control_Para.Home.z);
		Outer_Output.z = Math.Constrain(Control_Para.POS_Outer_PID_z.Cal_PID_POS(Time),POS_SPEED_SET_MAX_Z,-POS_SPEED_SET_MAX_Z);
		//--------------�������------------------------------------------------------------------//
		// Z ���ѡ�� ��ͣ��ң��
		PWM_In_POS Position_Z_Control_Choose = PWM_In.POS_Judge(PWM_RC_D_U);
		if(Position_Z_Control_Choose != PWM_In_Mid)
		{
			Control_Para.POS_Outer_PID_z.Setpoint = Position.Position_xyz.z;
			Control_Para.POS_Inner_PID_z.Setpoint = (1 - POS_FILTER_SPEED)*Control_Para.POS_Inner_PID_z.Setpoint + POS_FILTER_SPEED * ((PWM_RC_D_U - PWM_RC_MID)/PWM_RC_RANGE*2*POS_SPEED_SET_MAX_Z);
		}
		else
		{
			Control_Para.POS_Inner_PID_z.Setpoint = (1 - POS_FILTER_SPEED)*Control_Para.POS_Inner_PID_z.Setpoint + POS_FILTER_SPEED * Outer_Output.z;
		}
	}
	
}


