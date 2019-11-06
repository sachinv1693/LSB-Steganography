#ifndef TYPES_H
#define TYPES_H

/* User defined types */
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned char* uchar_ptr;

/* Status will be used in fn. return type */
typedef enum
{
    e_failure,
	e_success
} Status;

typedef enum
{
    e_encode,
    e_decode,
    e_unsupported
} OperationType;

#endif
