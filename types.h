/*
 * =====================================================================================
 *
 *       Filename:  types.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16.01.2012 11:00:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef TYPES_H
#define TYPES_H

#include "config.h"


#if MAX_CPU <= 16
typedef unsigned short cpumask_t;
#elif MAX_CPU <= 32
typedef unsigned int cpumask_t;
#elif MAX_CPU <= 64
typedef unsigned long long cpumask_t;
#endif

#endif // TYPES_H
