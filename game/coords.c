#include "coords.h"

#include <math.h>

float Q_rsqrt( float number )
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;						// evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}

float coords_getSqDist(float dx, float dy) {
	return dx*dx + dy*dy;
}



Vector coords_getStartSpeed(
	float dx,
	float dy,
	float gravity,
	float maxSpeed
) {
	/*const float inv_duration = 1/duration;

	Vector result = {
		x: dx * inv_duration,
		y: dy * inv_duration - gravity * duration * .5f
	};

	float norm = result.x * result.x + result.y * result.y;
	if (norm > maxSpeed*maxSpeed) {
		norm = maxSpeed * Q_rsqrt(norm);
		result.x *= norm;
		result.y *= norm;
	}*/

	Vector result = {
		x: dx,
		y: dy
	};

	float norm = dx*dx + dy*dy;
	norm = maxSpeed * Q_rsqrt(norm);
	result.x *= norm;
	result.y *= norm;

	return result;
}
