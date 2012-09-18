#ifndef MSPIO_SHARED
#define MSPIO_SHARED

#include "vmb.h"
#include "option.h"
#include "param.h"

extern char version[];
extern char howto[];

// The address of the device where the output will be sent to
extern uint64_t output_address;

#endif