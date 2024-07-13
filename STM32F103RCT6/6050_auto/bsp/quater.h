#ifndef __QUATER_H
#define __QUATER_H

#include "stdint.h"

//===================================================�궨��===================================================
//����
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

#define ANGLE_TO_RAD(x)     ((x) * PI / 180.0)                                  // �Ƕ�ת��Ϊ����
#define RAD_TO_ANGLE(x)     ((x) * 180.0 / PI)                                  // ����ת��Ϊ�Ƕ�
#define PI                  (3.1415926535898)

//===================================================�궨��===================================================
//������
typedef struct
{
   float gyro[NUM_XYZ]; //���ٶ�
}gyroscope_info_struct;

//���ٶȼ�
typedef struct
{
    float angle[NUM_XYZ]; //���ٶȼƵõ��ĽǶ�
    float acc  [NUM_XYZ]; //�Ǽ��ٶ�
}acc_info_struct;

//imu
typedef struct
{
    gyroscope_info_struct gyro; //������
    acc_info_struct acc; //���ٶȼ�
}imu_info_struct;

//ŷ���ǽṹ��
typedef struct
{
    float pitch;
    float roll;
    float yaw;
}eulerAngle_info_struct;

//���ٶ�Ư����
typedef struct
{
    float Xdata;
    float Ydata;
    float Zdata;
} gyroOffset_info_struct;

//���ٶȼ��ٶ�Ư����
typedef struct
{
    double Xdata;
    double Ydata;
    double Zdata;
}accOffset_info_struct;

//===================================================��������===================================================
extern eulerAngle_info_struct eulerAngle;
extern gyroOffset_info_struct gyroOffset;
extern accOffset_info_struct accOffset;
extern imu_info_struct imu;
extern float imu_kp;                                             //���ٶȼƵ��������ʱ�������
extern float imu_ki;                                             //�������������ʵĻ�������
extern float exInt,eyInt,ezInt;   ;                                  // ������
//===================================================��������===================================================

//===================================================��������===================================================
void imu_task(void);
void imu_data_deal(int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz,acc_info_struct *acc,gyroscope_info_struct *gyro);
void gyroOffsetInit(void);
void accOffsetInit(void);
void Update_Angle(acc_info_struct *acc, gyroscope_info_struct *gyro);
void acc_get_angle(void);
float myRsqrt(float num);
//===================================================��������===================================================

#endif
