/*
 * spy.h
 *
 *  Created on: 22 Eki 2022
 *      Author: Safa
 */

#ifndef INC_SPY_H_
#define INC_SPY_H_

#ifdef SPY /* tracing enabled? */
#include <stdio.h> /* for printf() */
#define MY_PRINTF(format_, ...) printf(format_, ##__VA_ARGS__)

#else /* tracing disabled */
#define MY_PRINTF(format_, ...) (0)
#endif /* SPY */

#endif /* INC_SPY_H_ */
