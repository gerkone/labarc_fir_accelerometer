#include <acc_fir.h>


/* FIR coefficients, calculated using fir1(25 - 1, 10/50)*/
const float32_t firCoeff[NUM_TAPS] = { +0.0020104028f, +0.0016210204f,
	-0.0000000000f, -0.0044467088f, -0.0116854663f, -0.0181342601f,
	-0.0167737131f, +0.0000000000f, +0.0358771087f, +0.0869769736f,
	+0.1414878809f, +0.1834533329f, +0.1992268579f, +0.1834533329f,
	+0.1414878809f, +0.0869769736f, +0.0358771087f, +0.0000000000f,
	-0.0167737131f, -0.0181342601f, -0.0116854663f, -0.0044467088f,
	-0.0000000000f, +0.0016210204f, +0.0020104028f, };


/**
 * @brief init 3 FIRs, one each axis of the accelerometer
 * @return 	none.
 *
 */
void init_fir_3_axes() {
	/*
	 * DSP FIR params description
	 * S: structure containing
	 *   - number of filter coefficients in the filter
	 *   - pointer to the state variable array. The array is of length numTaps+block_size-1
	 *   - pointer to the coefficient array. The array is of length numTaps
	 * firCoeffs32: pointer to filter coefficient array
	 * firStateF32: pointer to state buffer
	 * block_size: number of samples to be processed each call
	*/
	// initialize 3 fir instances, one each axis
	arm_fir_init_f32(&S_x, NUM_TAPS, (float32_t*) &firCoeff[0], &firStateF32_x[0], BLOCK_SIZE);
	arm_fir_init_f32(&S_y, NUM_TAPS, (float32_t*) &firCoeff[0], &firStateF32_y[0], BLOCK_SIZE);
	arm_fir_init_f32(&S_z, NUM_TAPS, (float32_t*) &firCoeff[0], &firStateF32_z[0], BLOCK_SIZE);
	return;
}

/**
 * @brief Apply the FIRs on each axis
 * @param[acc_data]       	  **raw accelerometer data. unsigned int matrix of size block_size * 3
 * @param[output_samples_x]    *x output buffer. Array of size block_size
 * @param[output_samples_y]    *y output buffer. Array of size block_size
 * @param[output_samples_z]    *z output buffer. Array of size block_size
 * @return 						none.
 */
void fir_3_axes(int16_t acc_data[BLOCK_SIZE][3], float32_t *output_samples_x, float32_t *output_samples_y, float32_t *output_samples_z) {

	// select the column of each axis
	exctract_column(acc_data, fir_in_x, 0);
	exctract_column(acc_data, fir_in_y, 1);
	exctract_column(acc_data, fir_in_z, 2);

	// call fir on each axes
	lpf_fir(fir_in_x, output_samples_x, S_x);
	lpf_fir(fir_in_y, output_samples_y, S_y);
	lpf_fir(fir_in_z, output_samples_z, S_z);
	return;
}


/**
 * @brief Custom finite impulse response filter. Block size is the same as sample length
 * @param[input_samples]       *input buffer. Array of size block_size
 * @param[output_samples]      *output buffer. Array of size block_size
 * @param[S]      			   	current fir instance
 * @return 						none.
 */
void lpf_fir(float32_t *input_samples, float32_t *output_samples, arm_fir_instance_f32 S) {
	// apply fir
	arm_fir_f32(&S, input_samples, output_samples, BLOCK_SIZE);
	return;
}

/*
 * @brief 			this function splices a column from a matrix and casts it to float.
 * @param[in]		input matrix
 * @param[out] 		output array - float values
 * @param[col] 		column to keep
 * @return 			none
 */
void exctract_column(int16_t in[BLOCK_SIZE][3], float32_t *out, unsigned int col) {
	for (int i = 0; i < BLOCK_SIZE; i++) {
		out[i] = (float32_t) in[i][col];
	}
	return;
}

