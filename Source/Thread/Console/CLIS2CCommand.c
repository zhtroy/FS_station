/*
 * CLIS2CCommand.c
 *
 *  Created on: 2019-6-11
 *      Author: DELL
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"
#include "s2c_com_new.h"


BaseType_t prvSetCarNums( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
BaseType_t prvSetStationStatus( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
BaseType_t prvDelCar( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
BaseType_t prvShowStationStatus( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

const CLI_Command_Definition_t xSetCarNums =
{
    "setCar",
    "\r\n Setting Car Nums of Station. \
    ex:setCar 3\r\n",
    prvSetCarNums, /* The function to run. */
    1 /* Three parameters are expected, which can take any value. */
};

const CLI_Command_Definition_t xSetState =
{
    "setSts",
    "\r\n Setting Station Status. \
    ex:setSts 0\r\n",
    prvSetStationStatus, /* The function to run. */
    1 /* Three parameters are expected, which can take any value. */
};
const CLI_Command_Definition_t xDelCar =
{
    "delCar",
    "\r\n Setting Station Status. \
    ex:delCar 0x6001\r\n",
    prvDelCar, /* The function to run. */
    1
};

const CLI_Command_Definition_t xShowStation =
{
    "show",
    "\r\n Show Station Status. \
    ex:show \r\n",
    prvShowStationStatus, /* The function to run. */
    0
};

BaseType_t prvSetCarNums( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    const char *pcParameter;
    BaseType_t xParameterStringLength, xReturn;
    static UBaseType_t uxParameterNumber = 0;
    static uint8_t ucValue = 0;

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

        if(uxParameterNumber == 1)
        {
            ucValue = autoStrtol(pcParameter);
            S2CSetCarNums(ucValue);
            xReturn = pdFALSE;
        }
    }

    return xReturn;
}

BaseType_t prvSetStationStatus( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    const char *pcParameter;
    BaseType_t xParameterStringLength, xReturn;
    static UBaseType_t uxParameterNumber = 0;
    static uint8_t ucValue = 0;

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

        if(uxParameterNumber == 1)
        {
            ucValue = autoStrtol(pcParameter);
            S2CSetStationStatus(ucValue);
            xReturn = pdFALSE;
        }
    }

    return xReturn;
}

BaseType_t prvDelCar( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    const char *pcParameter;
    BaseType_t xParameterStringLength, xReturn;
    static UBaseType_t uxParameterNumber = 0;
    static uint16_t usValue = 0;

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

        if(uxParameterNumber == 1)
        {
            usValue = autoStrtol(pcParameter);
            S2CRemoveCar(usValue);
            xReturn = pdFALSE;
        }
    }

    return xReturn;
}

BaseType_t prvShowStationStatus( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    const char *pcParameter;
    BaseType_t xParameterStringLength, xReturn;
    static UBaseType_t uxParameterNumber = 0;
    static uint16_t usValue = 0;

    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    ( void ) xWriteBufferLen;
    configASSERT( pcWriteBuffer );

    /* Command Process*/
    memset( pcWriteBuffer, 0x00, xWriteBufferLen );

    S2CShowRoadLog();
    S2CShowStationLog();

	xReturn = pdFALSE;


    return xReturn;
}
