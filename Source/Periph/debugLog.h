#ifndef _DEBUG_LOG_H
#define _DEBUG_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "logLib.h"

#define DEBUG_GENERAL	(0x00000001)   
#define DEBUG_INFO	    (0x00000002)    
#define DEBUG_WARNING   (0x00000004)
#define DEBUG_ERROR     (0x00000008)

#define FSZX_DEBUG_ERROR

#if defined (FSZX_DEBUG_GENERAL)
#define FSZX_DBG_CURRENT_TYPES ((DEBUG_INFO) | (DEBUG_GENERAL) | (DEBUG_WARNING) | (DEBUG_ERROR))
#elif defined (FSZX_DEBUG_INFO)
#define FSZX_DBG_CURRENT_TYPES ((DEBUG_INFO) | (DEBUG_GENERAL) | (DEBUG_WARNING))
#elif defined (FSZX_DEBUG_WARNING)
#define FSZX_DBG_CURRENT_TYPES ((DEBUG_INFO) | (DEBUG_GENERAL))
#elif defined (FSZX_DEBUG_ERROR)
#define FSZX_DBG_CURRENT_TYPES (DEBUG_ERROR)
#else
#define FSZX_DBG_CURRENT_TYPES 0
#endif

#define FSZX_DEBUG_LOG(type,fmt,arg1,arg2,arg3,arg4,arg5,arg6) \
		if (((type) & FSZX_DBG_CURRENT_TYPES))  {logMsg (fmt,arg1,arg2,arg3,arg4,arg5,arg6); }


#ifdef __cplusplus
}
#endif

#endif /* _DEBUG_LOG_H */
