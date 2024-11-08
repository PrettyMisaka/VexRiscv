#ifndef __UART_RING_H__
#define __UART_RING_H__

#define UART_RING_VERSION "uart_ring Version:2.1"

#define INT_FLAG        "INT"
#define FLOAT_FLAG      "FLOAT"
#define STRING_FLAG     "STRING"

#define SEPARATOR_FLAG  ' '
#define END_FLAG        '\n'

#define RING_BUFF_LENGTH 64
#define CMD_STRING_ADD_MAX_LENGTH 30
#define CMD_STRING_SAVE_MAX_LENGTH 20
#define CMD_BUFF_LENGTH 10

#define RING_MAX_DATATYPE_LENGTH 1
#define RING_RETURN_STRING_MAX_LENGTH 20

typedef unsigned int uint16_ring;
typedef unsigned char uint8_ring;
typedef float float_ring;

// work state
typedef enum{
    RING_OK = 0,
    RING_ERR,
    RING_WAIT,
    RING_FREEHEAD,
    RING_INT,
    RING_FLOAT,
    RING_WAITFORFLAG,
    RING_OVERFLOW,
    RING_NEXT
}ring_state;

//
typedef enum{
    RING_DATATYPE_NULL = -1,
    RING_RETURN_INT = 0,
    RING_RETURN_FLOAT,
    RING_RETURN_STRING
}ring_return_dataType;

//The variable passed in in the callback function
typedef struct {
    ring_state state;
    int offset;
    int intData[RING_MAX_DATATYPE_LENGTH];
    // float_ring floatData[RING_MAX_DATATYPE_LENGTH];
    char stringData[RING_MAX_DATATYPE_LENGTH][RING_RETURN_STRING_MAX_LENGTH];
} callBackFun_EventTypedef;

//save cmd data
typedef struct cmdNodeInf{
    uint8_ring  cmd_String[CMD_STRING_SAVE_MAX_LENGTH];
    void (*callBackFn)(void);
    ring_return_dataType dataTypeArr[RING_MAX_DATATYPE_LENGTH];
} cmdNodeTypedef;

typedef struct {
    uint8_ring ringBuffArr[RING_BUFF_LENGTH];
    int ringBuffHead;
    int ringBuffTail;
    int ringBuffLength;
    cmdNodeTypedef cmdBuffArr[CMD_BUFF_LENGTH];
    int cmdBuffLength;
} uartRingParameterTypedef;

char *ringGetString(int i);
/*----------INIT FUN----------*/
void ringBuff_Total_Init(uartRingParameterTypedef * uart_ring_param_P);
void ringBuff_Init(uartRingParameterTypedef * uart_ring_param_P);
void cmdBuff_Init(uartRingParameterTypedef * uart_ring_param_P);
/*----------PUSH AND DEL FUN----------*/
void ringBuff_Push( uartRingParameterTypedef * uart_ring_param_P, uint8_ring data);
ring_state cmdBuff_Push( uartRingParameterTypedef * uart_ring_param_P,
                         uint8_ring cmd_String[CMD_STRING_SAVE_MAX_LENGTH],
                         void (*callBackFn)(void));
ring_state cmdBuff_DelCmd( uartRingParameterTypedef * uart_ring_param_P,
                           uint8_ring cmd_String[CMD_STRING_SAVE_MAX_LENGTH]);
/*----------HANDLE FUN---------*/
ring_state ringShowBuff( uartRingParameterTypedef * uart_ring_param_P, char ringBuffStr[RING_BUFF_LENGTH]);
ring_state ringBuffHandleFun( uartRingParameterTypedef * uart_ring_param_P);


#endif
