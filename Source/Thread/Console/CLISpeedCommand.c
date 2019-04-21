/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"
#if 1


BaseType_t prvSpeedSet( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
extern void speedSet(int32_t slVc,int32_t slVe,int32_t slTe);

const CLI_Command_Definition_t xSpeedSet =
{
    "vset",
    "\r\n\r\n=========Speed Set========\r\n \
vset <vc> <ve> <te>:Speed Setting\r\n \
<vc>: Current Speed(rpm)\r\n \
<ve>: Expected Speed(rpm)\r\n \
<te>: Expected Time(ms)\r\n \
ie,vset 200 500 1000\r\n",
    prvSpeedSet, /* The function to run. */
    3 /* Three parameters are expected, which can take any value. */
};



BaseType_t prvSpeedSet( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    const char *pcParameter;
    BaseType_t xParameterStringLength, xReturn;
    static UBaseType_t uxParameterNumber = 0;
    static int32_t slVc = 0;
    static int32_t slVe = 0;
    static int32_t slTe = 0;

    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    ( void ) xWriteBufferLen;
    configASSERT( pcWriteBuffer );

    if( uxParameterNumber == 0 )
    {
        /* Command Process*/
        uxParameterNumber = 1U;
        memset( pcWriteBuffer, 0x00, xWriteBufferLen );
        xReturn = pdPASS;
    }
    else
    {
        /* Obtain the parameter string. */
        pcParameter = FreeRTOS_CLIGetParameter
                        (
                            pcCommandString,        /* The command string itself. */
                            uxParameterNumber,      /* Return the next parameter. */
                            &xParameterStringLength /* Store the parameter string length. */
                        );

        if(uxParameterNumber == 1)     /*Type */
        {
            uxParameterNumber++;
            slVc = autoStrtol(pcParameter);
            xReturn = pdPASS;
        }
        else if(uxParameterNumber == 2) /*Address */
        {
            uxParameterNumber++;
            slVe = autoStrtol(pcParameter);
            xReturn = pdPASS;
        }
        else if(uxParameterNumber == 3) /*Value */
        {
            slTe = autoStrtol(pcParameter);

            speedSet(slVc,slVe,slTe);

            pcWriteBuffer[0] = '0';
            uxParameterNumber = 0;
            xReturn = pdFALSE;
        }
    }

    return xReturn;
}

#endif
