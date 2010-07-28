#ifndef _common_types_h_
#define _common_types_h_

/**
 * @file common_types.h
 *
 * This header file contains some typedefs to make programming easier.
 */

typedef unsigned char uint_8; /**< Unsigned 8-bit variable.*/
typedef unsigned short int uint_16; /**< Unsigned 16-bit variable.*/
typedef unsigned int uint_32; /**< Unsigned 32-bit variable.*/
typedef unsigned long long int uint_64; /**< Unsigned 64-bit variable.*/

typedef char int_8; /**< Signed 8-bit variable.*/
typedef short int int_16; /**< Signed 16-bit variable.*/
typedef int int_32; /**< Signed 32-bit variable.*/
typedef long long int int_64; /**< Signed 64-bit variable.*/

#ifndef BOOL
/**
 * Macro to define a boolean type in C.
 */
#define BOOL uint_8
#endif
#ifndef FALSE
/**
 * Macro for the boolean value false.
 */
#define FALSE 0
#endif
#ifndef TRUE
/**
 * Macro for the boolean value true.
 */
#define TRUE 1

#endif

#endif
