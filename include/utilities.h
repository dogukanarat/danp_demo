/* utilities.h - one line definition */

/* All Rights Reserved */

#ifndef INC_UTILITIES_H
#define INC_UTILITIES_H

/* Includes */

#include <stdio.h>
#include <stdarg.h>
#include "danp/danp.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Configurations */


/* Definitions */


/* Types */


/* External Declarations */

extern void logMessage(
    danpLogLevel_t level,
    const char *funcName,
    const char *message,
    va_list args);

extern int32_t transaction(
    uint16_t id,
    uint16_t dPort,
    uint8_t *data,
    size_t dataLen,
    uint8_t *responseBuffer,
    size_t responseBufferLen,
    size_t *responseLen,
    uint32_t timeout);


#ifdef __cplusplus
}
#endif

#endif /* INC_UTILITIES_H */