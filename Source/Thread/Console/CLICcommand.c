
/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"


extern const CLI_Command_Definition_t xMemeoryWrite;
extern const CLI_Command_Definition_t xMemeoryRead;
extern const CLI_Command_Definition_t xResetCPU;

/*-----------------------------------------------------------*/

void vRegisterSampleCLICommands( void )
{
	/* Register all the command line commands defined immediately above. */
    FreeRTOS_CLIRegisterCommand( &xResetCPU );
    FreeRTOS_CLIRegisterCommand( &xMemeoryWrite );
    FreeRTOS_CLIRegisterCommand( &xMemeoryRead );
}
/*-----------------------------------------------------------*/

