/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"


int32_t ioRead(int32_t baseAddr,int32_t offset,int8_t bytes)
{
    if(bytes== 1)
        return *((volatile int8_t *)baseAddr+offset);
    else if(bytes == 2)
        return *((volatile int16_t *)baseAddr+offset);
    else
        return *((volatile int32_t *)baseAddr+offset);
}

void ioWrite(int32_t baseAddr,int32_t offset,int32_t value,int8_t bytes)
{
    if(bytes == 1)
        *((volatile int8_t *)baseAddr+offset) = (int8_t)value;
    else if(bytes == 2)
        *((volatile int16_t *)baseAddr+offset) = (int16_t)value;
    else
        *((volatile int32_t *)baseAddr+offset) = value;
}

int32_t autoStrtol(const char *str)
{
    if(strncmp(str,"0x",2) == 0 || strncmp(str,"0X",2) == 0)
        return strtol(str, NULL, 16);
    else if(*str == '0')
        return strtol(str, NULL, 8);
    else
        return strtol(str, NULL, 10);
}

uint8_t strSplitToData(char *pcParameter, uint8_t *data)
{
    uint8_t len = 0;
    char *p = strtok(pcParameter,",");
    while(p != NULL)
    {
        *(data+len) = autoStrtol(p);
        len++;
        p = strtok(NULL,",");
    }
    return len;
}
