/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"

#define LTE_WORK_MODE (1)
#define LTE_TEST_MODE (0)

int8_t lteMode = LTE_WORK_MODE;

extern unsigned int UART2Puts(char *pTxBuffer, int numBytesToWrite);
BaseType_t prvLTESend( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
BaseType_t prvLTESenda( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
BaseType_t prvLTESetMode( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
BaseType_t prvLTEGetMode( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );




const CLI_Command_Definition_t xLTESend =
{
	"lsd",
	"\r\n\r\n=========LTE Command===========\r\n \
lsd <data>:Send LTE DATA. \r\n \
<data>: LTE DATA\r\n \
ie, lsd test\r\n",
	prvLTESend, /* The function to run. */
	1 /* Three parameters are expected, which can take any value. */
};

const CLI_Command_Definition_t xLTESenda =
{
	"lsa",
	"\r\n \
lsa <param>:Send LTE AT Command.\r\n \
param:AT Command\r\n \
ie, lsa AT+VER\r\n",
	prvLTESenda, /* The function to run. */
	1 /* Three parameters are expected, which can take any value. */
};


const CLI_Command_Definition_t xLTESetMode =
{
	"lsm",
	"\r\n \
lsm <mode>:Set LTE Work Mode.\r\n \
<mode>: Work Mode<APP or TEST>\r\n \
ie, lsm APP\r\n",
	prvLTESetMode, /* The function to run. */
	1 /* Three parameters are expected, which can take any value. */
};


const CLI_Command_Definition_t xLTEGetMode =
{
	"lgm",
	"\r\n \
lgm:Get LTE Work Mode.\r\n",
	prvLTEGetMode, /* The function to run. */
	0 /* Three parameters are expected, which can take any value. */
};


BaseType_t prvLTESend( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    const char *pcParameter;
    BaseType_t xParameterStringLength, xReturn;
    static UBaseType_t uxParameterNumber = 0;


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
							pcCommandString,		/* The command string itself. */
							uxParameterNumber,		/* Return the next parameter. */
							&xParameterStringLength	/* Store the parameter string length. */
						);
        
        if(uxParameterNumber == 1)     /*Type */
        {
            if( lteMode == LTE_TEST_MODE)
            {
                strncpy(pcWriteBuffer,pcParameter,xParameterStringLength);
                UART2Puts(pcWriteBuffer, xParameterStringLength);
                memset( pcWriteBuffer, 0x00, xWriteBufferLen );
            }
            uxParameterNumber = 0;
            xReturn = pdFALSE;
        }
    }
       
	return xReturn;
}

BaseType_t prvLTESenda( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    const char *pcParameter;
    BaseType_t xParameterStringLength, xReturn;
    static UBaseType_t uxParameterNumber = 0;


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
							pcCommandString,		/* The command string itself. */
							uxParameterNumber,		/* Return the next parameter. */
							&xParameterStringLength	/* Store the parameter string length. */
						);
        
        if(uxParameterNumber == 1)     /*Type */
        {
            if( lteMode == LTE_TEST_MODE)
            {
                strncpy(pcWriteBuffer,pcParameter,xParameterStringLength);
                UART2Puts(pcWriteBuffer, xParameterStringLength);
                UART2Puts("\r", 1);
                memset( pcWriteBuffer, 0x00, xWriteBufferLen );

            }
            uxParameterNumber = 0;
            xReturn = pdFALSE;
        }
    }
       
	return xReturn;
}

BaseType_t prvLTESetMode( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    const char *pcParameter;
    BaseType_t xParameterStringLength, xReturn;
    static UBaseType_t uxParameterNumber = 0;


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
							pcCommandString,		/* The command string itself. */
							uxParameterNumber,		/* Return the next parameter. */
							&xParameterStringLength	/* Store the parameter string length. */
						);
        
        if(uxParameterNumber == 1)     /*Type */
        {
            if(strncmp(pcParameter,"APP",3) == 0)
            {
                lteMode = LTE_WORK_MODE;
                strcpy(pcWriteBuffer,"\r\nLTE Enter Application Mode\r\n");
            }
            else if(strncmp(pcParameter,"TEST",4) == 0)
            {
                lteMode = LTE_TEST_MODE;
                strcpy(pcWriteBuffer,"\r\nLTE Enter TEST Mode\r\n");
            }
            else;
            uxParameterNumber = 0;
            xReturn = pdFALSE;
        }
    }
       
	return xReturn;
}

BaseType_t prvLTEGetMode( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    BaseType_t xReturn;
    static UBaseType_t uxParameterNumber = 0;


	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );

	if( uxParameterNumber == 0 )
	{	
        /* Command Process*/
		
        memset( pcWriteBuffer, 0x00, xWriteBufferLen );
        if(lteMode == LTE_WORK_MODE)
            strcpy(pcWriteBuffer,"\r\nLTE is Application Mode\r\n");
        else
            strcpy(pcWriteBuffer,"\r\nLTE is TEST Mode by External UART\r\n");
        
        uxParameterNumber = 0U;
		xReturn = pdFALSE;
	}
    else;
       
	return xReturn;
}


