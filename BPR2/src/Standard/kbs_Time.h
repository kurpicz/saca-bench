
#ifndef KBS_TIME_H
#define KBS_TIME_H


#include <sys/times.h> /* struct tms,times() */
#include "kbs_Error.h"
#include "kbs_Types.h"



typedef struct kbs_timestamp{
	struct timeval realtime;					/** real time in micro seconds */
	struct tms systemTime;            /** system time w.r.t. clock cycles */
}Kbs_TimeStamp;





/**
 * @return current time stamp
 */
Kbs_TimeStamp *kbs_get_TimeStamp();

/**
 * Print time stamp to stdout
 * @param tStamp time stamp to show on stdout
 */
void kbs_show_TimeStamp(Kbs_TimeStamp *tStamp);

/**
 * Computes difference in time values
 * @param first relative realtime value
 * @param second relative realtime value
 * @return real time between first and second
 */
struct timeval *kbs_get_DifferenceTimeval(struct timeval *first, struct timeval *second);

/**
 * Computes difference in time values
 * @param first relative system time value
 * @param second relative system time value
 * @return system time between first and second
 */
struct tms *kbs_get_DifferenceTms(struct tms *first, struct tms *second);

/**
 * Computes difference of time stamps
 * @param first relative system time value
 * @param second relative system time value
 * @return time stamp containing real and system time between first and second
 */
Kbs_TimeStamp *kbs_get_DifferenceTimeStamp(Kbs_TimeStamp *first, Kbs_TimeStamp *second);

#endif
