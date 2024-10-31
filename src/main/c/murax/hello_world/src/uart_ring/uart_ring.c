#include "uart_ring.h"

// #include "stdio.h"
// #include "stdlib.h"
#include "string.h"

static callBackFun_EventTypedef _e;
char *ringGetString(int i){
    return _e.stringData[i];
}
/**/
static ring_state ringBuffHandleDataFun(
    callBackFun_EventTypedef* e,
    cmdNodeTypedef cmdInfoData,
    int Separator_Cnt,
    char dataStringForHandle[CMD_STRING_SAVE_MAX_LENGTH]
    );

/* total init*/
void ringBuff_Total_Init(uartRingParameterTypedef *uart_ring_param_P)
{
    ringBuff_Init(uart_ring_param_P);
    cmdBuff_Init(uart_ring_param_P);
}

/*----------RING BUFF FUN----------*/
void ringBuff_Init(uartRingParameterTypedef *uart_ring_param_P)
{
    uart_ring_param_P->ringBuffHead = 0;
    uart_ring_param_P->ringBuffTail = uart_ring_param_P->ringBuffHead - 1;
    uart_ring_param_P->ringBuffLength = 0;
}

void ringBuff_Push(uartRingParameterTypedef *uart_ring_param_P, uint8_ring data)
{
    uart_ring_param_P->ringBuffTail = (uart_ring_param_P->ringBuffTail + 1) % RING_BUFF_LENGTH;
    uart_ring_param_P->ringBuffArr[uart_ring_param_P->ringBuffTail] = data;
    uart_ring_param_P->ringBuffLength++;
    if(uart_ring_param_P->ringBuffLength == RING_BUFF_LENGTH - 2){
        ringBuff_Init(uart_ring_param_P);
    }
}

/*
@brief show receive buffer content
@param *uart_ring_param_P the param's pointer
@param ringBuffStr return buffer content
*/
ring_state ringShowBuff(uartRingParameterTypedef *uart_ring_param_P, char ringBuffStr[RING_BUFF_LENGTH])
{
    if (uart_ring_param_P->ringBuffHead == (uart_ring_param_P->ringBuffTail + 1) % RING_BUFF_LENGTH)
    {
        ringBuffStr[0] = 0;
        ringBuffStr[1] = 0;
        return RING_WAIT;
    }
    int tmp = uart_ring_param_P->ringBuffHead - uart_ring_param_P->ringBuffTail;
    if (tmp < 0)
        tmp += RING_BUFF_LENGTH;
    for (int i = 0; i < tmp + 1; i++)
    {
        ringBuffStr[i] = uart_ring_param_P->ringBuffArr[(i + uart_ring_param_P->ringBuffHead) % RING_BUFF_LENGTH];
    }
    return RING_OK;
}

/*
@brief Clear the first element of the buffer
*/
static ring_state ringBuff_FreeHeadP(uartRingParameterTypedef *uart_ring_param_P)
{
    if (uart_ring_param_P->ringBuffHead == (uart_ring_param_P->ringBuffTail + 1) % RING_BUFF_LENGTH)
        return RING_WAIT;
    if(uart_ring_param_P->ringBuffLength == 0)
        return RING_WAIT;
    uart_ring_param_P->ringBuffHead = (uart_ring_param_P->ringBuffHead + 1) % RING_BUFF_LENGTH;
    uart_ring_param_P->ringBuffLength--;
    return RING_OK;
}

/*----------CMD Fun----------*/

void cmdBuff_Init(uartRingParameterTypedef *uart_ring_param_P)
{
    uart_ring_param_P->cmdBuffLength = 0;
}
/*
@brief push the cmd to the param
@param *uart_ring_param_P the param's pointer
@param *cmd_String cmd data , example "helloworld STRING FLOAT INT STRING\n" will
    return 2 string variable 1 FLOAT variable and 1 INT variable when Calling callBackFn
@param *callBackFn the function you want to execute when you receive this command
@return ring_state RING_OK-push success RING_ERR-Incorrect format RING_BUFF_OVERFLOW-cmd length overflow
*/
ring_state cmdBuff_Push(uartRingParameterTypedef *uart_ring_param_P, uint8_ring cmd_String[CMD_STRING_ADD_MAX_LENGTH], void (*callBackFn)(void)){
    int str_length = 0;
    if( cmd_String[0] == 0) return RING_ERR;
    for(int i = 0; i < CMD_STRING_ADD_MAX_LENGTH; i++){
        if(cmd_String[i] == 0){
            if(cmd_String[i-1] == END_FLAG){
                str_length = i;
                break;
            }
            else 
                return RING_ERR;
        }
        if(i == CMD_STRING_ADD_MAX_LENGTH - 1)
            return RING_OVERFLOW;
    }
    int Separator_Cnt = 0;
    int str_tmpIndex = 0;
    char str_tmp[CMD_STRING_SAVE_MAX_LENGTH] = "";
    for(int i = 0; i < str_length; i++){
        if(cmd_String[i] == SEPARATOR_FLAG || cmd_String[i] == END_FLAG){
            str_tmp[str_tmpIndex+1] = 0;
            if(Separator_Cnt == 0){
                if (uart_ring_param_P->cmdBuffLength != 0)
                {
                    for (int i = 0; i < uart_ring_param_P->cmdBuffLength; i++)
                    {
                        if (strcmp((char *)uart_ring_param_P->cmdBuffArr[i].cmd_String, (char *)cmd_String) == 0)
                            return RING_ERR;
                    }
                }
                memcpy((char *)uart_ring_param_P->cmdBuffArr[uart_ring_param_P->cmdBuffLength].cmd_String, (char *)str_tmp, sizeof(str_tmp));
            }
            else if(Separator_Cnt <= RING_MAX_DATATYPE_LENGTH){
                if(str_tmp[0] == INT_FLAG[0] && str_tmp[1] == INT_FLAG[1] && str_tmp[2] == INT_FLAG[2]){
                    uart_ring_param_P->cmdBuffArr[uart_ring_param_P->cmdBuffLength].dataTypeArr[Separator_Cnt-1] = RING_RETURN_INT;
                }
                else if(str_tmp[0] == FLOAT_FLAG[0] && str_tmp[1] == FLOAT_FLAG[1] &&
                        str_tmp[2] == FLOAT_FLAG[2] && str_tmp[3] == FLOAT_FLAG[3] &&
                        str_tmp[4] == FLOAT_FLAG[4] ){
                    uart_ring_param_P->cmdBuffArr[uart_ring_param_P->cmdBuffLength].dataTypeArr[Separator_Cnt-1] = RING_RETURN_FLOAT;
                }
                else if(str_tmp[0] == STRING_FLAG[0] && str_tmp[1] == STRING_FLAG[1] &&
                        str_tmp[2] == STRING_FLAG[2] && str_tmp[3] == STRING_FLAG[3] &&
                        str_tmp[4] == STRING_FLAG[4] && str_tmp[5] == STRING_FLAG[5] ){
                    uart_ring_param_P->cmdBuffArr[uart_ring_param_P->cmdBuffLength].dataTypeArr[Separator_Cnt-1] = RING_RETURN_STRING;
                }
                else 
                    return RING_ERR;
            }
            else
                return RING_ERR;
            if(cmd_String[i+1] == END_FLAG || cmd_String[i] == END_FLAG)
                break;
            str_tmpIndex = 0;
            Separator_Cnt++;
        }
        else{
            str_tmp[str_tmpIndex] = cmd_String[i];
            str_tmpIndex++;
        }
    }
    uart_ring_param_P->cmdBuffArr[uart_ring_param_P->cmdBuffLength].callBackFn = callBackFn;
    for(int i = RING_MAX_DATATYPE_LENGTH;i > Separator_Cnt; i--){
        uart_ring_param_P->cmdBuffArr[uart_ring_param_P->cmdBuffLength].dataTypeArr[i-1] = RING_DATATYPE_NULL;
    }
    uart_ring_param_P->cmdBuffLength++;
    return RING_OK;
}

/*
@brief del cmd
*/
ring_state cmdBuff_DelCmd(uartRingParameterTypedef *uart_ring_param_P, uint8_ring cmd_String[CMD_STRING_SAVE_MAX_LENGTH]){
    int index = 0;
    if (uart_ring_param_P->cmdBuffLength == 0)
        return RING_WAIT;
    for (int i = 0; i < uart_ring_param_P->cmdBuffLength; i++)
    {
        if (strcmp((char *)uart_ring_param_P->cmdBuffArr[i].cmd_String, (char *)cmd_String) == 0)
        {
            index = i;
            uart_ring_param_P->cmdBuffLength--;
            for (int i = index; i < uart_ring_param_P->cmdBuffLength; i++)
            {
                uart_ring_param_P->cmdBuffArr[i] = uart_ring_param_P->cmdBuffArr[i + 1];
            }
            return RING_OK;
        }
    }
    return RING_ERR;
}

/*----------CMP CMD AND CALL CALLBACK FUN----------*/
ring_state ringBuffHandleFun(uartRingParameterTypedef *uart_ring_param_P){
    if (uart_ring_param_P->cmdBuffLength == 0)
        return RING_ERR;
    if (uart_ring_param_P->ringBuffHead == (uart_ring_param_P->ringBuffTail + 1) % RING_BUFF_LENGTH)
        return RING_WAIT;
    int ringBuffIndex_tmp = 0;
    int ringBuff_cmdLength_tmp = 0;
    while (1){
        if(uart_ring_param_P->ringBuffArr[(ringBuffIndex_tmp + uart_ring_param_P->ringBuffHead) % RING_BUFF_LENGTH] == END_FLAG){
            ringBuff_cmdLength_tmp = ringBuffIndex_tmp + 1;
            break;
        }
       if(ringBuffIndex_tmp == uart_ring_param_P->ringBuffLength - 1){
            return RING_WAIT;
       }
       ringBuffIndex_tmp++;
    }

    ringBuffIndex_tmp = 0;
    int stringBeginIndex = 0;
    char cmdString_tmp[CMD_STRING_SAVE_MAX_LENGTH];
    while (1){
        if( uart_ring_param_P->ringBuffArr[(ringBuffIndex_tmp + uart_ring_param_P->ringBuffHead) % RING_BUFF_LENGTH] == SEPARATOR_FLAG
            || uart_ring_param_P->ringBuffArr[(ringBuffIndex_tmp + uart_ring_param_P->ringBuffHead) % RING_BUFF_LENGTH] == END_FLAG){
            cmdString_tmp[stringBeginIndex] = 0;
            // ringBuffIndex_tmp++;
            // stringBeginIndex = 0;
            break;
        }
        cmdString_tmp[stringBeginIndex] = uart_ring_param_P->ringBuffArr[(ringBuffIndex_tmp + uart_ring_param_P->ringBuffHead) % RING_BUFF_LENGTH];
        ringBuffIndex_tmp++;
        stringBeginIndex++;
    }
        
    int cmdBuffIndex_tmp = 0;
    int str_length = 0;
    int Separator_Cnt = 0;
    for (int i = 0; i < RING_MAX_DATATYPE_LENGTH; i++){
        _e.intData[i] = 0;
        // _e.floatData[i] = 0;
        _e.stringData[i][0] = 0;
        _e.stringData[i][1] = 0;
    }
    
    while (1)
    {
        if(strcmp(cmdString_tmp,(char*)uart_ring_param_P->cmdBuffArr[cmdBuffIndex_tmp].cmd_String) == 0){
            while (1){
                if(uart_ring_param_P->ringBuffArr[(ringBuffIndex_tmp + uart_ring_param_P->ringBuffHead) % RING_BUFF_LENGTH] == SEPARATOR_FLAG
                || uart_ring_param_P->ringBuffArr[(ringBuffIndex_tmp + uart_ring_param_P->ringBuffHead) % RING_BUFF_LENGTH] == END_FLAG ){
                    cmdString_tmp[stringBeginIndex] = 0;
                    _e.state = RING_OK;
                    if( 0 < Separator_Cnt && Separator_Cnt < RING_MAX_DATATYPE_LENGTH + 1){
                        _e.state = ringBuffHandleDataFun(&_e,uart_ring_param_P->cmdBuffArr[cmdBuffIndex_tmp],Separator_Cnt-1,cmdString_tmp);
                        if(_e.state == RING_ERR)
                            break;
                    }
                    else{

                    }
                    cmdString_tmp[0] = 0;
                    cmdString_tmp[1] = 0;
                    if(uart_ring_param_P->ringBuffArr[(ringBuffIndex_tmp + uart_ring_param_P->ringBuffHead) % RING_BUFF_LENGTH] == END_FLAG ){
                        break;
                    }
                    Separator_Cnt++;
                    stringBeginIndex = -1;
                }
                else{
                    cmdString_tmp[stringBeginIndex] = uart_ring_param_P->ringBuffArr[(ringBuffIndex_tmp + uart_ring_param_P->ringBuffHead) % RING_BUFF_LENGTH];
                }
                stringBeginIndex++;
                ringBuffIndex_tmp++;
            }
        }
        else{
            _e.state = RING_NEXT;
            if(cmdBuffIndex_tmp == uart_ring_param_P->cmdBuffLength - 1)
                _e.state = RING_ERR;
        }
        switch (_e.state){
        case RING_NEXT:
            cmdBuffIndex_tmp++;
            break;

        case RING_OK:
            for (int i = 0; ; i++){
                if(uart_ring_param_P->ringBuffArr[uart_ring_param_P->ringBuffHead] == END_FLAG){
                    ringBuff_FreeHeadP(uart_ring_param_P);
                    break;
                }
                ringBuff_FreeHeadP(uart_ring_param_P);
            }
            uart_ring_param_P->cmdBuffArr[cmdBuffIndex_tmp].callBackFn();
            return _e.state;
        
        case RING_ERR:
            for (int i = 0; ; i++){
                if(uart_ring_param_P->ringBuffArr[uart_ring_param_P->ringBuffHead] == END_FLAG){
                    ringBuff_FreeHeadP(uart_ring_param_P);
                    break;
                }
                ringBuff_FreeHeadP(uart_ring_param_P);
            }
            return RING_ERR;

        default:
            for (int i = 0; ; i++){
                if(uart_ring_param_P->ringBuffArr[uart_ring_param_P->ringBuffHead] == END_FLAG){
                    ringBuff_FreeHeadP(uart_ring_param_P);
                    break;
                }
                ringBuff_FreeHeadP(uart_ring_param_P);
            }
            return RING_ERR;
        }
    }
}

/*
@brief Storage variables
@return RING_OK-finish RING_ERR-error RING_OVERFLOW-overflow
*/
ring_state ringBuffHandleDataFun(
    callBackFun_EventTypedef* e,
    cmdNodeTypedef cmdInfoData,
    int Separator_Cnt,
    char dataStringForHandle[CMD_STRING_SAVE_MAX_LENGTH]
    ){
    if(Separator_Cnt >= RING_MAX_DATATYPE_LENGTH)
        return RING_OVERFLOW;
    int dataType_tmp = cmdInfoData.dataTypeArr[Separator_Cnt];
    int dataIndex = 0;
    for(int i = 0; i < Separator_Cnt;i++){
        if(cmdInfoData.dataTypeArr[i] == dataType_tmp)
            dataIndex++;
    }
    int stringLength = 0;
    for (int i = 0; i < CMD_STRING_SAVE_MAX_LENGTH; i++){
        if(dataStringForHandle[i] == 0){
            stringLength = i;
            break;
        }
    }
    int tmp = 1;
    int numVal;
    int pointCnt = 0;
    switch (dataType_tmp){
        case RING_RETURN_INT:
            if(0){
                e->intData[dataIndex] = 0;
                for (int i = 0; i < stringLength; i++){
                    numVal = dataStringForHandle[stringLength - i - 1] - '0';
                    if(0 <= numVal && numVal <= 9){
                        e->intData[dataIndex] += numVal * tmp;
                        tmp *= 10;
                    }
                    else
                        return RING_ERR;
                }
            }
            // e->intData[dataIndex] = atoi(dataStringForHandle);
            e->intData[dataIndex] = 0;
            break;
        case RING_RETURN_FLOAT:
            if(0){
                for (int i = 0; i < stringLength; i++){
                    if('0' <= dataStringForHandle[stringLength - i - 1] && dataStringForHandle[stringLength - i - 1] <= '9'){
                        continue;
                    }
                    else if(dataStringForHandle[stringLength - i - 1] == '.'){
                        pointCnt++;
                        if(pointCnt > 1)
                            return RING_ERR;
                    }
                    else
                        return RING_ERR;
                }
            }
            // e->floatData[dataIndex] = (float)atof(dataStringForHandle);
            // e->floatData[dataIndex] = 0.0;
            break;
        case RING_RETURN_STRING:
            strcpy(e->stringData[dataIndex],dataStringForHandle);
            break;
        default:
            return RING_OVERFLOW;
    }
    return RING_OK;
}
