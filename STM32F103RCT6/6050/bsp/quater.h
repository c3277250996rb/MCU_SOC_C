#ifndef __QUATER_H
#define __QUATER_H

#include "stdint.h"

//===================================================宏定义===================================================
//三轴
typedef enum
{
    X,
    Y,
    Z,
    NUM_XYZ
}imu_info_enum;

#define delta_T  0.008f

#define GYRO_SPL 16.4
#define ACC_SPL  4096.0

#define ANGLE_TO_RAD(x)     ((x) * PI / 180.0)                                  // 角度转换为弧度
#define RAD_TO_ANGLE(x)     ((x) * 180.0 / PI)                                  // 弧度转换为角度
#define PI                  (3.1415926535898)

//===================================================宏定义===================================================
//陀螺仪
typedef struct
{
   float gyro[NUM_XYZ]; //角速度
}gyroscope_info_struct;

//加速度计
typedef struct
{
    float angle[NUM_XYZ]; //加速度计得到的角度
    float acc  [NUM_XYZ]; //角加速度
}acc_info_struct;

//imu
typedef struct
{
    gyroscope_info_struct gyro; //陀螺仪
    acc_info_struct acc; //加速度计
}imu_info_struct;

//欧拉角结构体
typedef struct
{
    float pitch;
    float roll;
    float yaw;
}eulerAngle_info_struct;

//角速度漂移量
typedef struct
{
    float Xdata;
    float Ydata;
    float Zdata;
} gyroOffset_info_struct;

//加速度计速度漂移量
typedef struct
{
    double Xdata;
    double Ydata;
    double Zdata;
}accOffset_info_struct;

//===================================================变量声明===================================================
extern eulerAngle_info_struct eulerAngle;
extern gyroOffset_info_struct gyroOffset;
extern accOffset_info_struct accOffset;
extern imu_info_struct imu;
extern float imu_kp;                                             //加速度计的收敛速率比例增益
extern float imu_ki;                                             //陀螺仪收敛速率的积分增益
extern float exInt,eyInt,ezInt;   ;                                  // 误差积分
//===================================================变量声明===================================================

//===================================================函数声明===================================================
void imu_task(void);
void imu_data_deal(int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz,acc_info_struct *acc,gyroscope_info_struct *gyro);
void gyroOffsetInit(void);
void accOffsetInit(void);
void Update_Angle(acc_info_struct *acc, gyroscope_info_struct *gyro);
void acc_get_angle(void);
float myRsqrt(float num);
//===================================================函数声明===================================================

#endif
