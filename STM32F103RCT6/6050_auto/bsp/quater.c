/*
 * quater.c
 *
 *  Created on: 2023��11��20��
 *      Author: 33738
 */
#include "mpu6050.h"
#include "quater.h"
#include "math.h"


/******************************��������************************************/

eulerAngle_info_struct eulerAngle;                            //ŷ����
gyroOffset_info_struct gyroOffset;
accOffset_info_struct accOffset;
imu_info_struct imu;

float imu_kp= 15;                                           //���ٶȼƵ��������ʱ�������
float imu_ki= 0;                                            //�������������ʵĻ�������

float q0 = 1, q1 = 0, q2 = 0, q3 = 0;    // ��ʼλ����̬��Ϊ��0��0��0����Ӧ��Ԫ��Ϊ��1��0��0��0

//�������ٶ��������ϵķ������õ�ǰ��̬��������������������ϵķ��������Ļ���
float exInt = 0, eyInt = 0, ezInt = 0;    

/******************************��������************************************/

/*
 * @brief ������������Ư
 * ͨ���ɼ�һ���������ֵ�������������ƫ��ֵ��
 * ������� �����Ƕ�ȡ������ - ��Ʈֵ������ȥ�����ƫ������
 */
 
//������ƫ����
void gyroOffsetInit(void)
{
		gyroOffset.Xdata = 0;
		gyroOffset.Ydata = 0;
		gyroOffset.Zdata = 0;

		for (uint16_t i = 0; i < 200; i++)
		{
			  MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
				gyroOffset.Xdata += GX;
				gyroOffset.Ydata += GY;
				gyroOffset.Zdata += GZ;
				//HAL_Delay(5);
		}
		gyroOffset.Xdata /= 200;
		gyroOffset.Ydata /= 200;
		gyroOffset.Zdata /= 200;
}

//���ٶȼ�ƫ����
void accOffsetInit(void)
{
	 accOffset.Xdata = 0;
	 accOffset.Ydata = 0;
	 accOffset.Zdata = 0;

	 for (uint16_t i = 0; i < 200; i++)
	 {
			 MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
			 accOffset.Xdata += AX;
			 accOffset.Ydata += AY;
			 accOffset.Zdata += (float)AZ - ACC_SPL;
			 //HAL_Delay(5);
	 }

	 accOffset.Xdata /= 200;
	 accOffset.Ydata /= 200;
	 accOffset.Zdata /= 200;
}

//imu���ݴ���
void imu_data_deal(int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz,acc_info_struct *acc,gyroscope_info_struct *gyro)
{
     float alpha = 0.2;  //0.35

     acc->acc[X] = (ax - accOffset.Xdata) / ACC_SPL * alpha + acc->acc[X] * (1 - alpha);
     acc->acc[Y] = (ay - accOffset.Ydata) / ACC_SPL * alpha + acc->acc[Y] * (1 - alpha);
     acc->acc[Z] = (az - accOffset.Zdata) / ACC_SPL * alpha + acc->acc[Z] * (1 - alpha);

     gyro->gyro[X] = ANGLE_TO_RAD((gx - gyroOffset.Xdata) / GYRO_SPL);
     gyro->gyro[Y] = ANGLE_TO_RAD((gy - gyroOffset.Ydata) / GYRO_SPL);
     gyro->gyro[Z] = ANGLE_TO_RAD((gz - gyroOffset.Zdata) / GYRO_SPL);
}
//�����ǽ�����̬������
void imu_task(void)
{
		acc_get_angle();
		
    //���ݴ���
    imu_data_deal(AX, AY, AZ, GX, GY, GZ, &imu.acc, &imu.gyro);

    //����Ƕ�
    Update_Angle(&imu.acc,&imu.gyro);
}

/**
 * @brief �û����˲��㷨������������̬(�����ü��ٶȼ����������ǵĻ������)
 * ���ٶȼƶ���֮��������Ƚ����У��������ݼ��������̬���ţ������Ƕ������������У��������ݿ��ţ�
 * ������ʹ�û����������(�ڲ������㷨�Ŵ�̬���)��
 * ���ʹ����̬�����˲����������������ǣ��������ż��ٶȼơ�
 */

/*
�Ӽƺ������Ƕ��ܼ������̬����Ϊ��Ҫ�������ں��أ�����Ϊ���ٶȼƶ���֮����Ŷ������У�
���������ݼ��������̬���ţ�����������Ȼ������Щ�����У�������ʹ�������ǻ����Ư�ƣ�
�������Ҫ���л����������������ݣ��������żӼơ���������ʵ�Ӽ��޷��Ժ���ǽ���������
�����������Ҫ�����ơ�

���ں�֮ǰ��Ҫ�Դ�����ԭʼ���ݽ���һЩ������������£����ٶȼ�ˮƽ����ʱ��XY��Ӧ����
0����ģ���Z�����1��G����ˣ�������Ҫ�Լ��ٶȼƽ���XY������У׼��ע��Z��ɲ���һ��У
׼ȥ��~����ͬ���ģ���������ˮƽ��ֹ����ʱ�������ӦΪ0�������������ǽ��������У׼��
�������ǰѻ����׼ˮƽ��ֹ����ʱ�ɼ�����һ���ٴ��������ƽ����ΪУ׼ֵ��������ඣ�Ȼ��
����״̬�¸�����������ݾ��ǲɼ��������ݼ�ȥУ׼ֵඡ����˻������������ǲ������˲�����
�Խ��ܣ������ٶȼ������Ƚϴ���������Ҳ���������������˲���������20��ȵĻ������ڣ���
�ݻ����кܴ󲨶������������������̬��ֻ��0.3�����ҵĶ������ҿ����һ�㶼�ǽ���8���
�͹��ˣ����Ե����������˲�Ч����û�����������ˡ�

�����˲��Ǿ�����˲���������ȿ������˲����ٶ�Ҫ�졢������С������Ч�����翨������
����ļ���ʵ�ֵ��ǻ����˲���

ע�Ȿ��ʹ�õĻ����˲��������������ٶȣ�����������ǽǶȺͼ��ٶȼƽǶȵ�ƫ��ٰѸ���
��ı����ͻ����������������ٶȣ�����PID���������ȥ���������������

����һ�ָ��������ķ����������Ƕȣ�
���ݻ��ֽǶ�+=���ٶ�*dt���ںϽǶ�=����Ȩֵ*���ݻ��ֽǶ�+(1-����Ȩֵ)*���ٶȽǶȣ�
���ǣ�ʵ�ʸ�����Ԫ����Ҫ���ǽ��ٶȣ��������������Ƕȵķ�������ʵ�á�

*/

//�����˲�����
//���������g�������ǽ��ٶ�(����/s)��a��Ӽƣ�m/s2��g�����ԣ����һ����
void Update_Angle(acc_info_struct *acc, gyroscope_info_struct *gyro)
{
		float halfT = 0.5 * delta_T;    // �������ڵ�һ��

		float norm; // ʸ����ģ����Ԫ���ķ���
		float vx, vy, vz; // ��ǰ��̬��������������������ϵķ���
		float ex, ey, ez; // ��ǰ���ٶȼƲ�õ��������ٶ��������ϵķ����뵱ǰ��̬��������������������ϵķ��������

		float q0q0 = q0 * q0;
		float q0q1 = q0 * q1;
		float q0q2 = q0 * q2;
		float q1q1 = q1 * q1;
		float q1q3 = q1 * q3;
		float q2q2 = q2 * q2;
		float q2q3 = q2 * q3;
		float q3q3 = q3 * q3;

		if (acc->acc[X] * acc->acc[Y] * acc->acc[Z] == 0) // ���ٶȼƴ�����������״̬ʱ��������̬���㣬��Ϊ�������ĸ���������
				return;

		norm = myRsqrt(acc->acc[X] * acc->acc[X] + acc->acc[Y] * acc->acc[Y] + acc->acc[Z] * acc->acc[Z]); // ��λ�����ٶȼ�
		acc->acc[X] = acc->acc[X] * norm; // ��һ�����ٶȼƣ��������������Ҳ����Ҫ�޸�KP����
		acc->acc[Y] = acc->acc[Y] * norm;
		acc->acc[Z] = acc->acc[Z] * norm;

		// �õ�ǰ��̬������������������ϵķ�����������nϵ����[0,0,g]������ת�������ת��bϵ
		// �ο�����nϵת������������bϵ������Ԫ����ʾ�ķ������Ҿ�������м���
		vx = 2 * (q1q3 - q0q2);
		vy = 2 * (q0q1 + q2q3);
		vz = q0q0 - q1q1 - q2q2 + q3q3;

		// �����õ����������������������������ͨ�������������ˣ��������
		ex = (acc->acc[Y] * vz - acc->acc[Z] * vy);
		ey = (acc->acc[Z] * vx - acc->acc[X] * vz);
		ez = (acc->acc[X] * vy - acc->acc[Y] * vx);

		// �������л���
		exInt += ex;
		eyInt += ey;
		ezInt += ez;

		// �����PI�������ͻ���������������ǽ��ٶ�
		gyro->gyro[X] += imu_kp * ex + exInt * imu_ki;
		gyro->gyro[Y] += imu_kp * ey + eyInt * imu_ki;
		gyro->gyro[Z] += imu_kp * ez + ezInt * imu_ki; // û�д����ƵĻ��޷�����ƫ����

		// ���������̬�ĸ��£�Ҳ������Ԫ��΢�ַ��̵����
		// ����һ�ױϿ��ⷨ�����֪ʶ�ɲμ���������������Ե���ϵͳ��P212
		q0 = q0 + (-q1 * gyro->gyro[X] - q2 * gyro->gyro[Y] - q3 * gyro->gyro[Z]) * halfT;
		q1 = q1 + ( q0 * gyro->gyro[X] + q2 * gyro->gyro[Z] - q3 * gyro->gyro[Y]) * halfT;
		q2 = q2 + ( q0 * gyro->gyro[Y] - q1 * gyro->gyro[Z] + q3 * gyro->gyro[X]) * halfT;
		q3 = q3 + ( q0 * gyro->gyro[Z] + q1 * gyro->gyro[Y] - q2 * gyro->gyro[X]) * halfT;

		// ��λ����Ԫ�����ռ���תʱ�������죬������ת�Ƕȣ��������Դ����е������任
		norm = myRsqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
		q0 = q0 * norm;
		q1 = q1 * norm;
		q2 = q2 * norm;
		q3 = q3 * norm;

		// ��Ԫ����ŷ���ǵ�ת��
		// ����YAW��������ڼ��ٶȼƶ���û���������ã����ֱ���������ǻ��ִ���
		eulerAngle.pitch = RAD_TO_ANGLE(atan2(2 * q2 * q3 + 2 * q0 * q1, q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3)); // -180~180
		eulerAngle.roll =  RAD_TO_ANGLE(asin(2 * q0 * q2 - 2 * q1 * q3)); // -90~90
		eulerAngle.yaw =   RAD_TO_ANGLE(atan2(2 * q1 * q2 + 2 * q0 * q3, q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3)); // 0~360
}

//���ٶȼƻ�ȡ�Ƕ�
void acc_get_angle(void)
{
		float acc_x = ( AX-accOffset.Xdata ) / ACC_SPL;
		float acc_y = ( AY-accOffset.Ydata ) / ACC_SPL;
		float acc_z = ( AZ-accOffset.Zdata ) / ACC_SPL;

		imu.acc.angle[X]=  RAD_TO_ANGLE(atan( acc_y / acc_z));
		imu.acc.angle[Y]= -RAD_TO_ANGLE(atan( acc_x / sqrt(acc_y * acc_y +acc_z * acc_z)));

}

//����ƽ��������
float myRsqrt(float num)
{
    float halfx = 0.5f * num;
    float y = num;
    long i = *(long*)&y;
    i = 0x5f375a86 - (i >> 1);

    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));
    y = y * (1.5f - (halfx * y * y));

    return y;
}

