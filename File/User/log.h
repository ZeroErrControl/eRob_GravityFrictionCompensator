#ifndef __LOG_H__
#define __LOG_H__
#include "stdio.h"

#define MAIN_LOG(format, ...) printf("[MAIN]" format"\r\n", ##__VA_ARGS__)
#define FRIC_LOG(format, ...) printf("[FRIC]" format"\r\n", ##__VA_ARGS__)

#endif

