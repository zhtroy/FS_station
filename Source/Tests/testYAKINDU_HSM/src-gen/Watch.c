
#include "Watch.h"
#include "..\src\sc_types.h"

#include <stdlib.h>
#include <string.h>
/*! \file Implementation of the state machine 'Watch'
*/

/* prototypes of all internal functions */
static void enact_main_region_timekeeping(Watch* handle);
static void enact_main_region_timekeeping_r_time(Watch* handle);
static void enact_main_region_timekeeping_r_date(Watch* handle);
static void enseq_main_region_timekeeping_default(Watch* handle);
static void enseq_main_region_timekeeping_r_time_default(Watch* handle);
static void enseq_main_region_timekeeping_r_date_default(Watch* handle);
static void enseq_main_region_setting_default(Watch* handle);
static void enseq_main_region_setting_r_hour_default(Watch* handle);
static void enseq_main_region_setting_r_minute_default(Watch* handle);
static void enseq_main_region_setting_r_day_default(Watch* handle);
static void enseq_main_region_setting_r_month_default(Watch* handle);
static void enseq_main_region_init_default(Watch* handle);
static void enseq_main_region_default(Watch* handle);
static void enseq_main_region_timekeeping_r_default(Watch* handle);
static void shenseq_main_region_timekeeping_r(Watch* handle);
static void enseq_main_region_setting_r_default(Watch* handle);
static void exseq_main_region_timekeeping(Watch* handle);
static void exseq_main_region_timekeeping_r_time(Watch* handle);
static void exseq_main_region_timekeeping_r_date(Watch* handle);
static void exseq_main_region_setting(Watch* handle);
static void exseq_main_region_setting_r_hour(Watch* handle);
static void exseq_main_region_setting_r_minute(Watch* handle);
static void exseq_main_region_setting_r_day(Watch* handle);
static void exseq_main_region_setting_r_month(Watch* handle);
static void exseq_main_region_init(Watch* handle);
static void exseq_main_region(Watch* handle);
static void exseq_main_region_timekeeping_r(Watch* handle);
static void exseq_main_region_setting_r(Watch* handle);
static void react_main_region__entry_Default(Watch* handle);
static void react_main_region_timekeeping_r__entry_Default(Watch* handle);
static void react_main_region_setting_r__entry_Default(Watch* handle);
static sc_boolean react(Watch* handle, const sc_boolean try_transition);
static sc_boolean main_region_timekeeping_react(Watch* handle, const sc_boolean try_transition);
static sc_boolean main_region_timekeeping_r_time_react(Watch* handle, const sc_boolean try_transition);
static sc_boolean main_region_timekeeping_r_date_react(Watch* handle, const sc_boolean try_transition);
static sc_boolean main_region_setting_react(Watch* handle, const sc_boolean try_transition);
static sc_boolean main_region_setting_r_hour_react(Watch* handle, const sc_boolean try_transition);
static sc_boolean main_region_setting_r_minute_react(Watch* handle, const sc_boolean try_transition);
static sc_boolean main_region_setting_r_day_react(Watch* handle, const sc_boolean try_transition);
static sc_boolean main_region_setting_r_month_react(Watch* handle, const sc_boolean try_transition);
static sc_boolean main_region_init_react(Watch* handle, const sc_boolean try_transition);
static void clearInEvents(Watch* handle);
static void clearOutEvents(Watch* handle);


void watch_init(Watch* handle)
{
	sc_integer i;
	
	for (i = 0; i < WATCH_MAX_ORTHOGONAL_STATES; ++i)
	{
		handle->stateConfVector[i] = Watch_last_state;
	}
	
	for (i = 0; i < WATCH_MAX_HISTORY_STATES; ++i)
	{
		handle->historyVector[i] = Watch_last_state;
	}
	
	handle->stateConfVectorPosition = 0;
	
	clearInEvents(handle);
	clearOutEvents(handle);
	
}

void watch_enter(Watch* handle)
{
	/* Default enter sequence for statechart Watch */
	enseq_main_region_default(handle);
}

void watch_runCycle(Watch* handle)
{
	clearOutEvents(handle);
	for (handle->stateConfVectorPosition = 0;
		handle->stateConfVectorPosition < WATCH_MAX_ORTHOGONAL_STATES;
		handle->stateConfVectorPosition++)
		{
			
		switch (handle->stateConfVector[handle->stateConfVectorPosition])
		{
		case Watch_main_region_timekeeping_r_time:
		{
			main_region_timekeeping_r_time_react(handle, bool_true);
			break;
		}
		case Watch_main_region_timekeeping_r_date:
		{
			main_region_timekeeping_r_date_react(handle, bool_true);
			break;
		}
		case Watch_main_region_setting_r_hour:
		{
			main_region_setting_r_hour_react(handle, bool_true);
			break;
		}
		case Watch_main_region_setting_r_minute:
		{
			main_region_setting_r_minute_react(handle, bool_true);
			break;
		}
		case Watch_main_region_setting_r_day:
		{
			main_region_setting_r_day_react(handle, bool_true);
			break;
		}
		case Watch_main_region_setting_r_month:
		{
			main_region_setting_r_month_react(handle, bool_true);
			break;
		}
		case Watch_main_region_init:
		{
			main_region_init_react(handle, bool_true);
			break;
		}
		default:
			break;
		}
	}
	
	clearInEvents(handle);
}

void watch_exit(Watch* handle)
{
	/* Default exit sequence for statechart Watch */
	exseq_main_region(handle);
}

sc_boolean watch_isActive(const Watch* handle)
{
	sc_boolean result = bool_false;
	sc_integer i;
	
	for(i = 0; i < WATCH_MAX_ORTHOGONAL_STATES; i++)
	{
		result = result || handle->stateConfVector[i] != Watch_last_state;
	}
	
	return result;
}

/* 
 * Always returns 'false' since this state machine can never become final.
 */
sc_boolean watch_isFinal(const Watch* handle)
{
   return bool_false;
}

sc_boolean watch_isStateActive(const Watch* handle, WatchStates state)
{
	sc_boolean result = bool_false;
	switch (state)
	{
		case Watch_main_region_timekeeping :
			result = (sc_boolean) (handle->stateConfVector[SCVI_WATCH_MAIN_REGION_TIMEKEEPING] >= Watch_main_region_timekeeping
				&& handle->stateConfVector[SCVI_WATCH_MAIN_REGION_TIMEKEEPING] <= Watch_main_region_timekeeping_r_date);
			break;
		case Watch_main_region_timekeeping_r_time :
			result = (sc_boolean) (handle->stateConfVector[SCVI_WATCH_MAIN_REGION_TIMEKEEPING_R_TIME] == Watch_main_region_timekeeping_r_time
			);
			break;
		case Watch_main_region_timekeeping_r_date :
			result = (sc_boolean) (handle->stateConfVector[SCVI_WATCH_MAIN_REGION_TIMEKEEPING_R_DATE] == Watch_main_region_timekeeping_r_date
			);
			break;
		case Watch_main_region_setting :
			result = (sc_boolean) (handle->stateConfVector[SCVI_WATCH_MAIN_REGION_SETTING] >= Watch_main_region_setting
				&& handle->stateConfVector[SCVI_WATCH_MAIN_REGION_SETTING] <= Watch_main_region_setting_r_month);
			break;
		case Watch_main_region_setting_r_hour :
			result = (sc_boolean) (handle->stateConfVector[SCVI_WATCH_MAIN_REGION_SETTING_R_HOUR] == Watch_main_region_setting_r_hour
			);
			break;
		case Watch_main_region_setting_r_minute :
			result = (sc_boolean) (handle->stateConfVector[SCVI_WATCH_MAIN_REGION_SETTING_R_MINUTE] == Watch_main_region_setting_r_minute
			);
			break;
		case Watch_main_region_setting_r_day :
			result = (sc_boolean) (handle->stateConfVector[SCVI_WATCH_MAIN_REGION_SETTING_R_DAY] == Watch_main_region_setting_r_day
			);
			break;
		case Watch_main_region_setting_r_month :
			result = (sc_boolean) (handle->stateConfVector[SCVI_WATCH_MAIN_REGION_SETTING_R_MONTH] == Watch_main_region_setting_r_month
			);
			break;
		case Watch_main_region_init :
			result = (sc_boolean) (handle->stateConfVector[SCVI_WATCH_MAIN_REGION_INIT] == Watch_main_region_init
			);
			break;
		default:
			result = bool_false;
			break;
	}
	return result;
}

static void clearInEvents(Watch* handle)
{
	handle->iface.ue_raised = bool_false;
}

static void clearOutEvents(Watch* handle)
{
}

void watchIface_raise_ue(Watch* handle, EventType value)
{
	handle->iface.ue_value = value;
	handle->iface.ue_raised = bool_true;
	
	watch_runCycle(handle);
}



/* implementations of all internal functions */

/* Entry action for state 'timekeeping'. */
static void enact_main_region_timekeeping(Watch* handle)
{
	/* Entry action for state 'timekeeping'. */
	tickTime(&(handle->internal.t));
}

/* Entry action for state 'time'. */
static void enact_main_region_timekeeping_r_time(Watch* handle)
{
	/* Entry action for state 'time'. */
	showTime(&(handle->internal.t));
}

/* Entry action for state 'date'. */
static void enact_main_region_timekeeping_r_date(Watch* handle)
{
	/* Entry action for state 'date'. */
	showDate(&(handle->internal.t));
}

/* 'default' enter sequence for state timekeeping */
static void enseq_main_region_timekeeping_default(Watch* handle)
{
	/* 'default' enter sequence for state timekeeping */
	enact_main_region_timekeeping(handle);
	enseq_main_region_timekeeping_r_default(handle);
}

/* 'default' enter sequence for state time */
static void enseq_main_region_timekeeping_r_time_default(Watch* handle)
{
	/* 'default' enter sequence for state time */
	enact_main_region_timekeeping_r_time(handle);
	handle->stateConfVector[0] = Watch_main_region_timekeeping_r_time;
	handle->stateConfVectorPosition = 0;
	handle->historyVector[0] = handle->stateConfVector[0];
}

/* 'default' enter sequence for state date */
static void enseq_main_region_timekeeping_r_date_default(Watch* handle)
{
	/* 'default' enter sequence for state date */
	enact_main_region_timekeeping_r_date(handle);
	handle->stateConfVector[0] = Watch_main_region_timekeeping_r_date;
	handle->stateConfVectorPosition = 0;
	handle->historyVector[0] = handle->stateConfVector[0];
}

/* 'default' enter sequence for state setting */
static void enseq_main_region_setting_default(Watch* handle)
{
	/* 'default' enter sequence for state setting */
	enseq_main_region_setting_r_default(handle);
}

/* 'default' enter sequence for state hour */
static void enseq_main_region_setting_r_hour_default(Watch* handle)
{
	/* 'default' enter sequence for state hour */
	handle->stateConfVector[0] = Watch_main_region_setting_r_hour;
	handle->stateConfVectorPosition = 0;
}

/* 'default' enter sequence for state minute */
static void enseq_main_region_setting_r_minute_default(Watch* handle)
{
	/* 'default' enter sequence for state minute */
	handle->stateConfVector[0] = Watch_main_region_setting_r_minute;
	handle->stateConfVectorPosition = 0;
}

/* 'default' enter sequence for state day */
static void enseq_main_region_setting_r_day_default(Watch* handle)
{
	/* 'default' enter sequence for state day */
	handle->stateConfVector[0] = Watch_main_region_setting_r_day;
	handle->stateConfVectorPosition = 0;
}

/* 'default' enter sequence for state month */
static void enseq_main_region_setting_r_month_default(Watch* handle)
{
	/* 'default' enter sequence for state month */
	handle->stateConfVector[0] = Watch_main_region_setting_r_month;
	handle->stateConfVectorPosition = 0;
}

/* 'default' enter sequence for state init */
static void enseq_main_region_init_default(Watch* handle)
{
	/* 'default' enter sequence for state init */
	handle->stateConfVector[0] = Watch_main_region_init;
	handle->stateConfVectorPosition = 0;
}

/* 'default' enter sequence for region main region */
static void enseq_main_region_default(Watch* handle)
{
	/* 'default' enter sequence for region main region */
	react_main_region__entry_Default(handle);
}

/* 'default' enter sequence for region r */
static void enseq_main_region_timekeeping_r_default(Watch* handle)
{
	/* 'default' enter sequence for region r */
	react_main_region_timekeeping_r__entry_Default(handle);
}

/* shallow enterSequence with history in child r */
static void shenseq_main_region_timekeeping_r(Watch* handle)
{
	/* shallow enterSequence with history in child r */
	/* Handle shallow history entry of r */
	switch(handle->historyVector[ 0 ])
	{
		case Watch_main_region_timekeeping_r_time :
		{
			enseq_main_region_timekeeping_r_time_default(handle);
			break;
		}
		case Watch_main_region_timekeeping_r_date :
		{
			enseq_main_region_timekeeping_r_date_default(handle);
			break;
		}
		default: break;
	}
}

/* 'default' enter sequence for region r */
static void enseq_main_region_setting_r_default(Watch* handle)
{
	/* 'default' enter sequence for region r */
	react_main_region_setting_r__entry_Default(handle);
}

/* Default exit sequence for state timekeeping */
static void exseq_main_region_timekeeping(Watch* handle)
{
	/* Default exit sequence for state timekeeping */
	exseq_main_region_timekeeping_r(handle);
}

/* Default exit sequence for state time */
static void exseq_main_region_timekeeping_r_time(Watch* handle)
{
	/* Default exit sequence for state time */
	handle->stateConfVector[0] = Watch_last_state;
	handle->stateConfVectorPosition = 0;
}

/* Default exit sequence for state date */
static void exseq_main_region_timekeeping_r_date(Watch* handle)
{
	/* Default exit sequence for state date */
	handle->stateConfVector[0] = Watch_last_state;
	handle->stateConfVectorPosition = 0;
}

/* Default exit sequence for state setting */
static void exseq_main_region_setting(Watch* handle)
{
	/* Default exit sequence for state setting */
	exseq_main_region_setting_r(handle);
}

/* Default exit sequence for state hour */
static void exseq_main_region_setting_r_hour(Watch* handle)
{
	/* Default exit sequence for state hour */
	handle->stateConfVector[0] = Watch_last_state;
	handle->stateConfVectorPosition = 0;
}

/* Default exit sequence for state minute */
static void exseq_main_region_setting_r_minute(Watch* handle)
{
	/* Default exit sequence for state minute */
	handle->stateConfVector[0] = Watch_last_state;
	handle->stateConfVectorPosition = 0;
}

/* Default exit sequence for state day */
static void exseq_main_region_setting_r_day(Watch* handle)
{
	/* Default exit sequence for state day */
	handle->stateConfVector[0] = Watch_last_state;
	handle->stateConfVectorPosition = 0;
}

/* Default exit sequence for state month */
static void exseq_main_region_setting_r_month(Watch* handle)
{
	/* Default exit sequence for state month */
	handle->stateConfVector[0] = Watch_last_state;
	handle->stateConfVectorPosition = 0;
}

/* Default exit sequence for state init */
static void exseq_main_region_init(Watch* handle)
{
	/* Default exit sequence for state init */
	handle->stateConfVector[0] = Watch_last_state;
	handle->stateConfVectorPosition = 0;
}

/* Default exit sequence for region main region */
static void exseq_main_region(Watch* handle)
{
	/* Default exit sequence for region main region */
	/* Handle exit of all possible states (of Watch.main_region) at position 0... */
	switch(handle->stateConfVector[ 0 ])
	{
		case Watch_main_region_timekeeping_r_time :
		{
			exseq_main_region_timekeeping_r_time(handle);
			break;
		}
		case Watch_main_region_timekeeping_r_date :
		{
			exseq_main_region_timekeeping_r_date(handle);
			break;
		}
		case Watch_main_region_setting_r_hour :
		{
			exseq_main_region_setting_r_hour(handle);
			break;
		}
		case Watch_main_region_setting_r_minute :
		{
			exseq_main_region_setting_r_minute(handle);
			break;
		}
		case Watch_main_region_setting_r_day :
		{
			exseq_main_region_setting_r_day(handle);
			break;
		}
		case Watch_main_region_setting_r_month :
		{
			exseq_main_region_setting_r_month(handle);
			break;
		}
		case Watch_main_region_init :
		{
			exseq_main_region_init(handle);
			break;
		}
		default: break;
	}
}

/* Default exit sequence for region r */
static void exseq_main_region_timekeeping_r(Watch* handle)
{
	/* Default exit sequence for region r */
	/* Handle exit of all possible states (of Watch.main_region.timekeeping.r) at position 0... */
	switch(handle->stateConfVector[ 0 ])
	{
		case Watch_main_region_timekeeping_r_time :
		{
			exseq_main_region_timekeeping_r_time(handle);
			break;
		}
		case Watch_main_region_timekeeping_r_date :
		{
			exseq_main_region_timekeeping_r_date(handle);
			break;
		}
		default: break;
	}
}

/* Default exit sequence for region r */
static void exseq_main_region_setting_r(Watch* handle)
{
	/* Default exit sequence for region r */
	/* Handle exit of all possible states (of Watch.main_region.setting.r) at position 0... */
	switch(handle->stateConfVector[ 0 ])
	{
		case Watch_main_region_setting_r_hour :
		{
			exseq_main_region_setting_r_hour(handle);
			break;
		}
		case Watch_main_region_setting_r_minute :
		{
			exseq_main_region_setting_r_minute(handle);
			break;
		}
		case Watch_main_region_setting_r_day :
		{
			exseq_main_region_setting_r_day(handle);
			break;
		}
		case Watch_main_region_setting_r_month :
		{
			exseq_main_region_setting_r_month(handle);
			break;
		}
		default: break;
	}
}

/* Default react sequence for initial entry  */
static void react_main_region__entry_Default(Watch* handle)
{
	/* Default react sequence for initial entry  */
	enseq_main_region_init_default(handle);
}

/* Default react sequence for shallow history entry  */
static void react_main_region_timekeeping_r__entry_Default(Watch* handle)
{
	/* Default react sequence for shallow history entry  */
	/* Enter the region with shallow history */
	if (handle->historyVector[0] != Watch_last_state)
	{
		shenseq_main_region_timekeeping_r(handle);
	} else
	{
		enseq_main_region_timekeeping_r_time_default(handle);
	} 
}

/* Default react sequence for initial entry  */
static void react_main_region_setting_r__entry_Default(Watch* handle)
{
	/* Default react sequence for initial entry  */
	enseq_main_region_setting_r_hour_default(handle);
}

static sc_boolean react(Watch* handle, const sc_boolean try_transition) {
	/* State machine reactions. */
	handle->internal.userInput = NONE;
	if (handle->iface.ue_raised == bool_true)
	{ 
		handle->internal.userInput = handle->iface.ue_value;
	} 
	return bool_false;
}

static sc_boolean main_region_timekeeping_react(Watch* handle, const sc_boolean try_transition) {
	/* The reactions of state timekeeping. */
	sc_boolean did_transition = try_transition;
	if (try_transition == bool_true)
	{ 
		if ((react(handle, try_transition)) == (bool_false))
		{ 
			if ((handle->internal.userInput) == (TICK))
			{ 
				exseq_main_region_timekeeping(handle);
				enseq_main_region_timekeeping_default(handle);
			}  else
			{
				if ((handle->internal.userInput) == (SET))
				{ 
					exseq_main_region_timekeeping(handle);
					enseq_main_region_setting_default(handle);
				}  else
				{
					did_transition = bool_false;
				}
			}
		} 
	} 
	if ((did_transition) == (bool_false))
	{ 
	} 
	return did_transition;
}

static sc_boolean main_region_timekeeping_r_time_react(Watch* handle, const sc_boolean try_transition) {
	/* The reactions of state time. */
	sc_boolean did_transition = try_transition;
	if (try_transition == bool_true)
	{ 
		if ((main_region_timekeeping_react(handle, try_transition)) == (bool_false))
		{ 
			if ((handle->internal.userInput) == (MODE))
			{ 
				exseq_main_region_timekeeping_r_time(handle);
				enseq_main_region_timekeeping_r_date_default(handle);
			}  else
			{
				did_transition = bool_false;
			}
		} 
	} 
	if ((did_transition) == (bool_false))
	{ 
	} 
	return did_transition;
}

static sc_boolean main_region_timekeeping_r_date_react(Watch* handle, const sc_boolean try_transition) {
	/* The reactions of state date. */
	sc_boolean did_transition = try_transition;
	if (try_transition == bool_true)
	{ 
		if ((main_region_timekeeping_react(handle, try_transition)) == (bool_false))
		{ 
			if ((handle->internal.userInput) == (MODE))
			{ 
				exseq_main_region_timekeeping_r_date(handle);
				enseq_main_region_timekeeping_r_time_default(handle);
			}  else
			{
				did_transition = bool_false;
			}
		} 
	} 
	if ((did_transition) == (bool_false))
	{ 
	} 
	return did_transition;
}

static sc_boolean main_region_setting_react(Watch* handle, const sc_boolean try_transition) {
	/* The reactions of state setting. */
	sc_boolean did_transition = try_transition;
	if (try_transition == bool_true)
	{ 
		if ((react(handle, try_transition)) == (bool_false))
		{ 
			did_transition = bool_false;
		} 
	} 
	if ((did_transition) == (bool_false))
	{ 
	} 
	return did_transition;
}

static sc_boolean main_region_setting_r_hour_react(Watch* handle, const sc_boolean try_transition) {
	/* The reactions of state hour. */
	sc_boolean did_transition = try_transition;
	if (try_transition == bool_true)
	{ 
		if ((main_region_setting_react(handle, try_transition)) == (bool_false))
		{ 
			if ((handle->internal.userInput) == (SET))
			{ 
				exseq_main_region_setting_r_hour(handle);
				enseq_main_region_setting_r_minute_default(handle);
			}  else
			{
				did_transition = bool_false;
			}
		} 
	} 
	if ((did_transition) == (bool_false))
	{ 
	} 
	return did_transition;
}

static sc_boolean main_region_setting_r_minute_react(Watch* handle, const sc_boolean try_transition) {
	/* The reactions of state minute. */
	sc_boolean did_transition = try_transition;
	if (try_transition == bool_true)
	{ 
		if ((main_region_setting_react(handle, try_transition)) == (bool_false))
		{ 
			if ((handle->internal.userInput) == (SET))
			{ 
				exseq_main_region_setting_r_minute(handle);
				enseq_main_region_setting_r_day_default(handle);
			}  else
			{
				did_transition = bool_false;
			}
		} 
	} 
	if ((did_transition) == (bool_false))
	{ 
	} 
	return did_transition;
}

static sc_boolean main_region_setting_r_day_react(Watch* handle, const sc_boolean try_transition) {
	/* The reactions of state day. */
	sc_boolean did_transition = try_transition;
	if (try_transition == bool_true)
	{ 
		if ((main_region_setting_react(handle, try_transition)) == (bool_false))
		{ 
			if ((handle->internal.userInput) == (SET))
			{ 
				exseq_main_region_setting_r_day(handle);
				enseq_main_region_setting_r_month_default(handle);
			}  else
			{
				did_transition = bool_false;
			}
		} 
	} 
	if ((did_transition) == (bool_false))
	{ 
	} 
	return did_transition;
}

static sc_boolean main_region_setting_r_month_react(Watch* handle, const sc_boolean try_transition) {
	/* The reactions of state month. */
	sc_boolean did_transition = try_transition;
	if (try_transition == bool_true)
	{ 
		if ((main_region_setting_react(handle, try_transition)) == (bool_false))
		{ 
			if ((handle->internal.userInput) == (SET))
			{ 
				exseq_main_region_setting(handle);
				enseq_main_region_timekeeping_default(handle);
			}  else
			{
				did_transition = bool_false;
			}
		} 
	} 
	if ((did_transition) == (bool_false))
	{ 
	} 
	return did_transition;
}

static sc_boolean main_region_init_react(Watch* handle, const sc_boolean try_transition) {
	/* The reactions of state init. */
	sc_boolean did_transition = try_transition;
	if (try_transition == bool_true)
	{ 
		if ((react(handle, try_transition)) == (bool_false))
		{ 
			exseq_main_region_init(handle);
			initTime(&(handle->internal.t));
			enseq_main_region_timekeeping_default(handle);
		} 
	} 
	if ((did_transition) == (bool_false))
	{ 
	} 
	return did_transition;
}


