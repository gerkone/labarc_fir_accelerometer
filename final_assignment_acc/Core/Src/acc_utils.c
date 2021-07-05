#include <acc_utils.h>

/**
 * @brief Accelerometer roll
 * @param[in]       a_y acceleration along y axis
 * @param[in]       a_z acceleration along z axis
 * @param[in]       a_z acceleration along z axis
 * @return 			roll value - rotation along x axis.
 *
 */
float cal_roll(float a_x, float a_y, float a_z) {
	return atan2(a_y, sqrt(pow(a_x, 2) + pow(a_z, 2))) * 180.0 / M_PI;
}

/**
 * @brief accelerometer pitch
 * @param[in]       a_x acceleration along x axis
 * @param[in]       a_y acceleration along y axis
 * @param[in]       a_z acceleration along z axis
 * @return 			pitch value - rotation along y axis.
 *
 */
float cal_pitch(float a_x, float a_y, float a_z) {
	return atan2(-a_x, sqrt(pow(a_y, 2) + pow(a_z, 2))) * 180.0 / M_PI;
}

