
#ifndef WATCH_H_
#define WATCH_H_

#include "..\Watch_fun.h"
#include "..\src\sc_types.h"

#ifdef __cplusplus
extern "C" { 
#endif 

/*! \file Header of the state machine 'Watch'.
*/

#ifndef WATCH_EVENTQUEUE_BUFFERSIZE
#define WATCH_EVENTQUEUE_BUFFERSIZE 20
#endif
#ifndef SC_INVALID_EVENT_VALUE
#define SC_INVALID_EVENT_VALUE 0
#endif
/*! Define number of states in the state enum */

#define WATCH_STATE_COUNT 9

/*! Define dimension of the state configuration vector for orthogonal states. */
#define WATCH_MAX_ORTHOGONAL_STATES 1
/*! Define dimension of the state configuration vector for history states. */
#define WATCH_MAX_HISTORY_STATES 1


/*! Define indices of states in the StateConfVector */
#define SCVI_WATCH_MAIN_REGION_TIMEKEEPING 0
#define SCVI_WATCH_MAIN_REGION_TIMEKEEPING_R_TIME 0
#define SCVI_WATCH_MAIN_REGION_TIMEKEEPING_R_DATE 0
#define SCVI_WATCH_MAIN_REGION_SETTING 0
#define SCVI_WATCH_MAIN_REGION_SETTING_R_HOUR 0
#define SCVI_WATCH_MAIN_REGION_SETTING_R_MINUTE 0
#define SCVI_WATCH_MAIN_REGION_SETTING_R_DAY 0
#define SCVI_WATCH_MAIN_REGION_SETTING_R_MONTH 0
#define SCVI_WATCH_MAIN_REGION_INIT 0


/*! Enumeration of all states */ 
typedef enum
{
	Watch_last_state,
	Watch_main_region_timekeeping,
	Watch_main_region_timekeeping_r_time,
	Watch_main_region_timekeeping_r_date,
	Watch_main_region_setting,
	Watch_main_region_setting_r_hour,
	Watch_main_region_setting_r_minute,
	Watch_main_region_setting_r_day,
	Watch_main_region_setting_r_month,
	Watch_main_region_init
} WatchStates;



/*! Type definition of the data structure for the WatchIface interface scope. */
typedef struct
{
	sc_boolean ue_raised;
	EventType ue_value;
} WatchIface;



/*! Type definition of the data structure for the WatchInternal interface scope. */
typedef struct
{
	EventType userInput;
	timedate_t t;
} WatchInternal;




/*! 
 * Type definition of the data structure for the Watch state machine.
 * This data structure has to be allocated by the client code. 
 */
typedef struct
{
	WatchStates stateConfVector[WATCH_MAX_ORTHOGONAL_STATES];
	WatchStates historyVector[WATCH_MAX_HISTORY_STATES];
	sc_ushort stateConfVectorPosition; 
	
	WatchIface iface;
	WatchInternal internal;
} Watch;



/*! Initializes the Watch state machine data structures. Must be called before first usage.*/
extern void watch_init(Watch* handle);

/*! Activates the state machine */
extern void watch_enter(Watch* handle);

/*! Deactivates the state machine */
extern void watch_exit(Watch* handle);

/*! Performs a 'run to completion' step. */
extern void watch_runCycle(Watch* handle);


/*! Raises the in event 'ue' that is defined in the default interface scope. */ 
extern void watchIface_raise_ue(Watch* handle, EventType value);


/*!
 * Checks whether the state machine is active (until 2.4.1 this method was used for states).
 * A state machine is active if it was entered. It is inactive if it has not been entered at all or if it has been exited.
 */
extern sc_boolean watch_isActive(const Watch* handle);

/*!
 * Checks if all active states are final. 
 * If there are no active states then the state machine is considered being inactive. In this case this method returns false.
 */
extern sc_boolean watch_isFinal(const Watch* handle);

/*! Checks if the specified state is active (until 2.4.1 the used method for states was called isActive()). */
extern sc_boolean watch_isStateActive(const Watch* handle, WatchStates state);


#ifdef __cplusplus
}
#endif 

#endif /* WATCH_H_ */
