#ifndef MAIN_H
#define MAIN_H

/* LISPINC standard program-wide conventions and includes */
#include <stdio.h>

#include "usfstd.h"
#include "usfaesc.h"
#include "mpc.h"

typedef enum RpnvalType {RPNVAL_ERROR, RPNVAL_VALUE} RpnvalType;
typedef struct Rpnval {
	RpnvalType type;
	union {
		char *err;
		u64 val;
	};
} Rpnval;

#endif
