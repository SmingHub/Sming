/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.3-dev */

#ifndef PB_HOSTED_PB_H_INCLUDED
#define PB_HOSTED_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _PinMode {
    PinMode_INPUT = 0,
    PinMode_OUTPUT = 1,
    PinMode_INPUT_PULLUP = 2
} PinMode;

typedef enum _HostedCommand_Version {
    HostedCommand_Version_HOSTED_V_1_0 = 0
} HostedCommand_Version;

/* Struct definitions */
typedef struct _RequestDigitalRead {
    uint32_t pin;
} RequestDigitalRead;

typedef struct _RequestDigitalWrite {
    uint32_t pin;
    uint32_t value;
} RequestDigitalWrite;

typedef struct _RequestPinMode {
    uint32_t pin;
    PinMode mode;
} RequestPinMode;

typedef struct _ResponseDigitalRead {
    uint32_t value;
} ResponseDigitalRead;

typedef struct _HostedCommand {
    HostedCommand_Version version;
    uint32_t id;
    pb_size_t which_payload;
    union {
        RequestDigitalWrite requestDigitalWrite;
        RequestPinMode requestPinMode;
        RequestDigitalRead requestDigitalRead;
        ResponseDigitalRead responseDigitalRead;
    } payload;
} HostedCommand;


/* Helper constants for enums */
#define _PinMode_MIN PinMode_INPUT
#define _PinMode_MAX PinMode_INPUT_PULLUP
#define _PinMode_ARRAYSIZE ((PinMode)(PinMode_INPUT_PULLUP+1))

#define _HostedCommand_Version_MIN HostedCommand_Version_HOSTED_V_1_0
#define _HostedCommand_Version_MAX HostedCommand_Version_HOSTED_V_1_0
#define _HostedCommand_Version_ARRAYSIZE ((HostedCommand_Version)(HostedCommand_Version_HOSTED_V_1_0+1))


/* Initializer values for message structs */
#define RequestDigitalWrite_init_default         {0, 0}
#define RequestPinMode_init_default              {0, _PinMode_MIN}
#define RequestDigitalRead_init_default          {0}
#define ResponseDigitalRead_init_default         {0}
#define HostedCommand_init_default               {_HostedCommand_Version_MIN, 0, 0, {RequestDigitalWrite_init_default}}
#define RequestDigitalWrite_init_zero            {0, 0}
#define RequestPinMode_init_zero                 {0, _PinMode_MIN}
#define RequestDigitalRead_init_zero             {0}
#define ResponseDigitalRead_init_zero            {0}
#define HostedCommand_init_zero                  {_HostedCommand_Version_MIN, 0, 0, {RequestDigitalWrite_init_zero}}

/* Field tags (for use in manual encoding/decoding) */
#define RequestDigitalRead_pin_tag               1
#define RequestDigitalWrite_pin_tag              1
#define RequestDigitalWrite_value_tag            2
#define RequestPinMode_pin_tag                   1
#define RequestPinMode_mode_tag                  2
#define ResponseDigitalRead_value_tag            1
#define HostedCommand_version_tag                1
#define HostedCommand_id_tag                     2
#define HostedCommand_requestDigitalWrite_tag    10
#define HostedCommand_requestPinMode_tag         11
#define HostedCommand_requestDigitalRead_tag     12
#define HostedCommand_responseDigitalRead_tag    13

/* Struct field encoding specification for nanopb */
#define RequestDigitalWrite_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UINT32,   pin,               1) \
X(a, STATIC,   SINGULAR, UINT32,   value,             2)
#define RequestDigitalWrite_CALLBACK NULL
#define RequestDigitalWrite_DEFAULT NULL

#define RequestPinMode_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UINT32,   pin,               1) \
X(a, STATIC,   SINGULAR, UENUM,    mode,              2)
#define RequestPinMode_CALLBACK NULL
#define RequestPinMode_DEFAULT NULL

#define RequestDigitalRead_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UINT32,   pin,               1)
#define RequestDigitalRead_CALLBACK NULL
#define RequestDigitalRead_DEFAULT NULL

#define ResponseDigitalRead_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UINT32,   value,             1)
#define ResponseDigitalRead_CALLBACK NULL
#define ResponseDigitalRead_DEFAULT NULL

#define HostedCommand_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UENUM,    version,           1) \
X(a, STATIC,   SINGULAR, UINT32,   id,                2) \
X(a, STATIC,   ONEOF,    MESSAGE,  (payload,requestDigitalWrite,payload.requestDigitalWrite),  10) \
X(a, STATIC,   ONEOF,    MESSAGE,  (payload,requestPinMode,payload.requestPinMode),  11) \
X(a, STATIC,   ONEOF,    MESSAGE,  (payload,requestDigitalRead,payload.requestDigitalRead),  12) \
X(a, STATIC,   ONEOF,    MESSAGE,  (payload,responseDigitalRead,payload.responseDigitalRead),  13)
#define HostedCommand_CALLBACK NULL
#define HostedCommand_DEFAULT NULL
#define HostedCommand_payload_requestDigitalWrite_MSGTYPE RequestDigitalWrite
#define HostedCommand_payload_requestPinMode_MSGTYPE RequestPinMode
#define HostedCommand_payload_requestDigitalRead_MSGTYPE RequestDigitalRead
#define HostedCommand_payload_responseDigitalRead_MSGTYPE ResponseDigitalRead

extern const pb_msgdesc_t RequestDigitalWrite_msg;
extern const pb_msgdesc_t RequestPinMode_msg;
extern const pb_msgdesc_t RequestDigitalRead_msg;
extern const pb_msgdesc_t ResponseDigitalRead_msg;
extern const pb_msgdesc_t HostedCommand_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define RequestDigitalWrite_fields &RequestDigitalWrite_msg
#define RequestPinMode_fields &RequestPinMode_msg
#define RequestDigitalRead_fields &RequestDigitalRead_msg
#define ResponseDigitalRead_fields &ResponseDigitalRead_msg
#define HostedCommand_fields &HostedCommand_msg

/* Maximum encoded size of messages (where known) */
#define RequestDigitalWrite_size                 12
#define RequestPinMode_size                      8
#define RequestDigitalRead_size                  6
#define ResponseDigitalRead_size                 6
#define HostedCommand_size                       22

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
