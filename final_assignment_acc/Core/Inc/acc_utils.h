/**
  ******************************************************************************
  * @file           : acc_utils.h
  * @brief          : Header for acc_utils.c file.
  *                   This file contains some utils functions for accelerometer
  *                   operations (planar orientation).
  ******************************************************************************
  */

#ifndef __ACC_UTILS_H
#define __ACC_UTILS_H

#include <math.h>

float cal_roll(float a_y, float a_z);
float cal_pitch(float a_x, float a_y, float a_z);

#endif
