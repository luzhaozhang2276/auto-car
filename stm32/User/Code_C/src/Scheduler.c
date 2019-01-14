#include "Scheduler.h"
void Loop_1000Hz(u16 Time);
void Loop_500Hz(u16 Time);
void Loop_200Hz(u16 Time);
void Loop_100Hz(u16 Time);
void Loop_50Hz(u16 Time);

struct Scheduler_ Scheduler = 
{
	Loop_1000Hz,
	Loop_500Hz,
	Loop_200Hz,
	Loop_100Hz,
	Loop_50Hz
};

void Loop_1000Hz(u16 Time_Ms)
{
	
}

void Loop_500Hz(u16 Time_Ms)
{
  MPU6050.Updata();											//mpu6050���ݲɼ�
	Attitude.Updata();										//��̬����								-----��̬����
	//FlyControl.ATT_InnerLoop(Time_Ms);		//��̬�����ڻ�           -----------------------------------��̬ �ڻ�
	//FlyControl.POS_AccLoop(Time_Ms);			//λ�ÿ��Ƽ��ٶȻ�				-------------------λ�� ���ٶȻ�
}
void Loop_200Hz(u16 Time_Ms)
{
	Position.Updata(Time_Ms);							//λ�ý���								-----λ�ý���
	HMC5883.Updata();											//���������ݲɼ�
	//FlyControl.ATT_OuterLoop(Time_Ms);		//��̬�����⻷						-----------------------------------��̬ �⻷
  //FlyControl.POS_InnerLoop(Time_Ms);		//λ�ÿ����ٶ��ڻ�				-------------------λ�� �ٶȻ�
}
void Loop_100Hz(u16 Time_Ms)
{
	speed_measure.speed_get(Time_Ms);
	MS5611.Updata();											//��ѹ�����ݲɼ�		
	Communication.UpData();								//����ͨ��	
}
void Loop_50Hz(u16 Time_Ms)
{
	//GPS_Location.GPS_Update();						//GPS���ݲɼ�
	PWM_In.Updata();											//ң�����ݲɼ� 
  //FlyControl.POS_OuterLoop(Time_Ms);		//λ�ÿ����⻷						-------------------λ�� ��
	Power.Updata();									
	
	//ϵͳ״̬���
	SystemState.Updata(Time_Ms);
	//Motor.Output(False);
	if(CarControl.Para->IsLock == True){
		if(CarControl.Para->Mode == Auto)
			CarControl.stop();
		Motor.Stop();
		return;
	}
	
	if(CarControl.Para->Mode == Auto){
		CarControl.PWM_output();
	}
	else if(CarControl.Para->Mode == Manual){

		PWM_In.Data->CH1 = Math.Constrain(PWM_In.Data->CH1, 4000, 2900);
		PWM_In.Data->CH2 = Math.Constrain(PWM_In.Data->CH2, 4000, 2000);
		Motor.PWM->PWM1 = PWM_In.Data->CH1;
		Motor.PWM->PWM2 = PWM_In.Data->CH2;
		Motor.Output(False);

	}
}



