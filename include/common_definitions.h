/* network_definitions.h - one line definition */

/* All Rights Reserved */

#ifndef INC_NETWORK_DEFINITION_H
#define INC_NETWORK_DEFINITION_H

/* Includes */


#ifdef __cplusplus
extern "C" {
#endif


/* Configurations */


/* Definitions */

#define STACK_SIZE            (4 * 1024)
#define PRIORITY              (7)
#define SERVER_STREAM_PORT    (10)
#define SERVER_DGRAM_PORT     (11)
#define SOCK_TIMEOUT          (1000)

#define CLIENT_NODE_ID           20
#define SERVER_NODE_ID           10

#if SERVER_MODE == 1
#define OWN_NODE_ID SERVER_NODE_ID
#define REMOTE_NODE_ID CLIENT_NODE_ID
#else
#define OWN_NODE_ID CLIENT_NODE_ID
#define REMOTE_NODE_ID SERVER_NODE_ID
#endif

/* Types */


/* External Declarations */


#ifdef __cplusplus
}
#endif

#endif /* INC_NETWORK_DEFINITION_H */
