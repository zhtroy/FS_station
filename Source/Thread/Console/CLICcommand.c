
/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"



void vRegisterSampleCLICommands( void );

extern BaseType_t prvMemroyWrite( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
extern BaseType_t prvMemroyRead( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

extern const CLI_Command_Definition_t xMemeoryWrite;
extern const CLI_Command_Definition_t xMemeoryRead;
extern const CLI_Command_Definition_t xResetCPU;
/*-----------------------------------------------------------*/

void vRegisterSampleCLICommands( void )
{
	/* Register all the command line commands defined immediately above. */
    FreeRTOS_CLIRegisterCommand( &xMemeoryWrite );
    FreeRTOS_CLIRegisterCommand( &xMemeoryRead );
    FreeRTOS_CLIRegisterCommand( &xResetCPU );
}
/*-----------------------------------------------------------*/

