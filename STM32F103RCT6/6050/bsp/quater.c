/*
 * quater.c
 *
 *  Created on: 2023年11月20日
 *      Author: 33738
 */
#include "mpu6050.h"
#include "quater.h"
#include "math.h"


/******************************变量定义************************************/

eulerAngle_info_struct eulerAngle;                            //欧拉角
gyroOffset_info_struct gyroOffset;
accOffset_info_struct accOffset;
imu_info_struct imu;

float imu_kp= 15;                                           //加速度计的收敛速率比例增益
float imu_ki= 0;                                            //陀螺仪收敛速率的积分增益

float q0 = 1, q1 = 0, q2 = 0, q3 = 0;    // 初始位置姿态角为：0、0、0，对应四元数为：1、0、0、0

//重力加速度在三轴上的分量与用当前姿态计算得来的重力在三轴上的分量的误差的积分
float exInt = 0, eyInt = 0, ezInt = 0;    

/******************************变量定义************************************/

/*
 * @brief 计算陀螺仪零漂
 * 通过采集一定数据求均值计算陀螺仪零点偏移值。
 * 后可以用 陀螺仪读取的数据 - 零飘值，即可去除零点偏移量。
 */
 
//陀螺仪偏移量
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

//加速度计偏移量
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

//imu数据处理
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
//陀螺仪解算姿态任务函数
void imu_task(void)
{
		acc_get_angle();
		
    //数据处理
    imu_data_deal(AX, AY, AZ, GX, GY, GZ, &imu.acc, &imu.gyro);

    //解算角度
    Update_Angle(&imu.acc,&imu.gyro);
}

/**
 * @brief 用互补滤波算法解算陀螺仪姿态(即利用加速度计修正陀螺仪的积分误差)
 * 加速度计对振动之类的噪声比较敏感，长期数据计算出的姿态可信；陀螺仪对振动噪声不敏感，短期数据可信，
 * 但长期使用积分误差严重(内部积分算法放大静态误差)。
 * 因此使用姿态互补滤波，短期相信陀螺仪，长期相信加速度计。
 */

/*
加计和陀螺仪都能计算出姿态，但为何要对它们融合呢，是因为加速度计对振动之类的扰动很敏感，
但长期数据计算出的姿态可信，而陀螺仪虽然对振动这些不敏感，但长期使用陀螺仪会出现漂移，
因此我们要进行互补，短期相信陀螺，长期相信加计。不过，其实加计无法对航向角进行修正，
修正航向角需要磁力计。

在融合之前先要对传感器原始数据进行一些处理。理想情况下，加速度计水平放置时，XY轴应该是
0输出的，仅Z轴输出1个G，因此，我们需要对加速度计进行XY轴的零点校准（注意Z轴可不能一起校
准去了~）；同样的，陀螺仪在水平静止放置时各轴输出应为0，因此需对陀螺仪进行三轴的校准。
方法就是把机体标准水平静止放置时采集它个一两百次数据求个平均作为校准值保存起来喽，然后
工作状态下各轴输出的数据就是采集来的数据减去校准值喽。仅此还不够，陀螺仪不进行滤波还可
以接受，但加速度计噪声比较大，所以至少也得来个滑动窗口滤波，我用了20深度的滑动窗口，数
据还是有很大波动，不过最后计算出的姿态角只有0.3度左右的抖动（我看大家一般都是建议8深度
就够了，所以单滑动窗口滤波效果是没法做到更好了。

互补滤波是经典的滤波方法，相比卡尔曼滤波的速度要快、计算量小，但是效果不如卡尔曼。
这个文件里实现的是互补滤波。

注意本文使用的互补滤波方法是修正角速度：先求出陀螺仪角度和加速度计角度的偏差，再把该误
差的比例和积分项用于修正角速度，类似PID控制用误差去修正输入控制量。

另有一种更容易理解的方法是修正角度：
陀螺积分角度+=角速度*dt；融合角度=陀螺权值*陀螺积分角度+(1-陀螺权值)*加速度角度；
但是，实际更新四元数需要的是角速度，所以这种修正角度的方法并不实用。

*/

//互补滤波函数
//输入参数：g表陀螺仪角速度(弧度/s)，a表加计（m/s2或g都可以，会归一化）
void Update_Angle(acc_info_struct *acc, gyroscope_info_struct *gyro)
{
		float halfT = 0.5 * delta_T;    // 采样周期的一半

		float norm; // 矢量的模或四元数的范数
		float vx, vy, vz; // 当前姿态计算得来的重力在三轴上的分量
		float ex, ey, ez; // 当前加速度计测得的重力加速度在三轴上的分量与当前姿态计算得来的重力在三轴上的分量的误差

		float q0q0 = q0 * q0;
		float q0q1 = q0 * q1;
		float q0q2 = q0 * q2;
		float q1q1 = q1 * q1;
		float q1q3 = q1 * q3;
		float q2q2 = q2 * q2;
		float q2q3 = q2 * q3;
		float q3q3 = q3 * q3;

		if (acc->acc[X] * acc->acc[Y] * acc->acc[Z] == 0) // 加速度计处于自由落体状态时不进行姿态解算，因为会产生分母无穷大的情况
				return;

		norm = myRsqrt(acc->acc[X] * acc->acc[X] + acc->acc[Y] * acc->acc[Y] + acc->acc[Z] * acc->acc[Z]); // 单位化加速度计
		acc->acc[X] = acc->acc[X] * norm; // 归一化加速度计，这样变更了量程也不需要修改KP参数
		acc->acc[Y] = acc->acc[Y] * norm;
		acc->acc[Z] = acc->acc[Z] * norm;

		// 用当前姿态计算出重力在三个轴上的分量，重力在n系下是[0,0,g]，乘以转换矩阵就转到b系
		// 参考坐标n系转化到载体坐标b系，用四元数表示的方向余弦矩阵第三行即是
		vx = 2 * (q1q3 - q0q2);
		vy = 2 * (q0q1 + q2q3);
		vz = q0q0 - q1q1 - q2q2 + q3q3;

		// 计算测得的重力与计算得重力间的误差，这个误差是通过向量外积（叉乘）求出来的
		ex = (acc->acc[Y] * vz - acc->acc[Z] * vy);
		ey = (acc->acc[Z] * vx - acc->acc[X] * vz);
		ez = (acc->acc[X] * vy - acc->acc[Y] * vx);

		// 对误差进行积分
		exInt += ex;
		eyInt += ey;
		ezInt += ez;

		// 将误差PI（比例和积分项）补偿到陀螺仪角速度
		gyro->gyro[X] += imu_kp * ex + exInt * imu_ki;
		gyro->gyro[Y] += imu_kp * ey + eyInt * imu_ki;
		gyro->gyro[Z] += imu_kp * ez + ezInt * imu_ki; // 没有磁力计的话无法修正偏航角

		// 下面进行姿态的更新，也就是四元数微分方程的求解
		// 采用一阶毕卡解法，相关知识可参见《惯性器件与惯性导航系统》P212
		q0 = q0 + (-q1 * gyro->gyro[X] - q2 * gyro->gyro[Y] - q3 * gyro->gyro[Z]) * halfT;
		q1 = q1 + ( q0 * gyro->gyro[X] + q2 * gyro->gyro[Z] - q3 * gyro->gyro[Y]) * halfT;
		q2 = q2 + ( q0 * gyro->gyro[Y] - q1 * gyro->gyro[Z] + q3 * gyro->gyro[X]) * halfT;
		q3 = q3 + ( q0 * gyro->gyro[Z] + q1 * gyro->gyro[Y] - q2 * gyro->gyro[X]) * halfT;

		// 单位化四元数，空间旋转时不会拉伸，仅有旋转角度，类似线性代数中的正交变换
		norm = myRsqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
		q0 = q0 * norm;
		q1 = q1 * norm;
		q2 = q2 * norm;
		q3 = q3 * norm;

		// 四元数到欧拉角的转换
		// 其中YAW航向角由于加速度计对其没有修正作用，因此直接用陀螺仪积分代替
		eulerAngle.pitch = RAD_TO_ANGLE(atan2(2 * q2 * q3 + 2 * q0 * q1, q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3)); // -180~180
		eulerAngle.roll =  RAD_TO_ANGLE(asin(2 * q0 * q2 - 2 * q1 * q3)); // -90~90
		eulerAngle.yaw =   RAD_TO_ANGLE(atan2(2 * q1 * q2 + 2 * q0 * q3, q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3)); // 0~360
}

//加速度计获取角度
void acc_get_angle(void)
{
		float acc_x = ( AX-accOffset.Xdata ) / ACC_SPL;
		float acc_y = ( AY-accOffset.Ydata ) / ACC_SPL;
		float acc_z = ( AZ-accOffset.Zdata ) / ACC_SPL;

		imu.acc.angle[X]=  RAD_TO_ANGLE(atan( acc_y / acc_z));
		imu.acc.angle[Y]= -RAD_TO_ANGLE(atan( acc_x / sqrt(acc_y * acc_y +acc_z * acc_z)));

}

//求倒数平方根函数
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

