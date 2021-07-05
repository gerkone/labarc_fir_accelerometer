/**
  ******************************************************************************
  * @file           : acc_fir.h
  * @brief          : Header for acc_fir.c file.
  *                   This file contains some a FIR custom definition for
  *                   accelerometer signal processing.
  *                   references:
  *                   https://www.keil.com/pack/doc/CMSIS/DSP/html/arm_fir_example_f32_8c-example.html
  ******************************************************************************
  */

#ifndef __ACC_FIR_H
#define __ACC_FIR_H

#include "arm_math.h"

/* BEGIN FIR DEFINES */
//number of samples processed at a time
#define BLOCK_SIZE            25
// length of the filter, number of coefficients, filter order + 1
#define NUM_TAPS              25

/* END FIR DEFINES*/

/* FIR coefficients */
const float32_t firCoeff[NUM_TAPS];
/*FIR related variables*/
float32_t firStateF32_x[BLOCK_SIZE + NUM_TAPS - 1];
float32_t firStateF32_y[BLOCK_SIZE + NUM_TAPS - 1];
float32_t firStateF32_z[BLOCK_SIZE + NUM_TAPS - 1];
// one fir each axis
arm_fir_instance_f32 S_x;
arm_fir_instance_f32 S_y;
arm_fir_instance_f32 S_z;
// fir input columns
float32_t fir_in_x[BLOCK_SIZE];
float32_t fir_in_y[BLOCK_SIZE];
float32_t fir_in_z[BLOCK_SIZE];

/* FILTER */
void lpf_fir(float32_t *input_samples, float32_t *output_samples, arm_fir_instance_f32 S);

void fir_3_axes(int16_t acc_data[BLOCK_SIZE][3], float32_t *output_samples_x, float32_t *output_samples_y, float32_t *output_samples_z);
void init_fir_3_axes();


/* HELPERS */
void exctract_column(int16_t in[BLOCK_SIZE][3], float32_t *out, unsigned int col);
#endif
