/*
 * @Author: hsq007 2267222816@qq.com
 * @Date: 2024-12-14 22:22:58
 * @LastEditors: hsq007 2267222816@qq.com
 * @LastEditTime: 2024-12-15 17:42:12
 * @FilePath: \20241214-M2006电机实验\File\User\hsq_math.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef HSQ_MATH_H__
#define HSQ_MATH_H__

typedef unsigned char  	uint8_t;    //无符号8位整型变量
typedef signed   char  	int8_t;		//有符号8位整型变量
typedef unsigned short 	uint16_t;	//无符号16位整型变量
typedef signed   short 	int16_t;	//有符号16位整型变量 
typedef unsigned int   	uint32_t;	//无符号32位整型变量
typedef int   			int32_t;	//有符号32位整型变量
typedef float          	f32;		//单精度浮点数32位长度
typedef double         	fp64;		//双精度浮点数64位长度


#define  HSQ_MATH_K_RPM2RADPS   (0.104719755119660f)
#define  HSQ_MATH_K_RADPS2RPM   (9.549296585513721f)
#define  HSQ_MATH_K_RAD2DEG     (57.29577951308232f)
#define  HSQ_MATH_K_DEG2RAD     (0.017453292519943f)
#define  HSQ_MATH_PI            (3.141592653589793f)
#define  HSQ_MATH_2PI           (6.283185307179586f)
#define  HSQ_MATH_PI_DIV2       (1.570796326794897f)
#define  HSQ_MATH_EXP1          (2.718281828459046f) // 自然底数e

f32 MATH_absf(f32 input);
f32 MATH_linear_table_interpolation(f32 *pxtable, f32 *pytable, int16_t len, f32 x);

f32 HSQ_MATH_absf(f32 input);
f32 HSQ_MATH_linear_interpolation(f32 x_0, f32 x_1, f32 y_0, f32 y_1, f32 x);
f32 HSQ_MATH_linear_table_interpolation(f32 *pxtable, f32 *pytable, int16_t len, f32 x);
f32 HSQ_MATH_limit(f32 max, f32 x);
f32 HSQ_MATH_limit_between(f32 x, f32 min, f32 max);
uint32_t HSQ_MATH_limit_between_u32(uint32_t x, uint32_t min, uint32_t max);
f32 HSQ_MATH_parabola_interpolation(f32 x_0, f32 y_0, f32 x_1, f32 y_1, f32 x);
f32 HSQ_MATH_sign(f32 x);
fp64 HSQ_MATH_ln(fp64 a);
f32 HSQ_MATH_dead_zone(f32 input, f32 dead_zone);
f32 HSQ_MATH_dead_zone_linear(f32 x, f32 d, f32 k);
#endif

