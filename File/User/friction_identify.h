#ifndef __FRICTION_IDENTIFY_H__
#define __FRICTION_IDENTIFY_H__

#include "hsq_math.h"

void Friction_Identify_init(void);
void Friction_Identify_step(float dt);
float fric_compensate_step(float spd_fbk);

#endif

