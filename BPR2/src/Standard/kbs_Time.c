
#include <stdlib.h>
#include <stdio.h>
#include <sys/times.h> /*struct tms,times() */
#include <sys/time.h>
#include <unistd.h>
#include "kbs_Time.h"
#include "kbs_Error.h"
#include "kbs_Types.h"


/*--------------------------------------------------------------------------*/
Kbs_TimeStamp *kbs_get_TimeStamp() {
	Kbs_Int check;
	Kbs_TimeStamp *newTimeStamp;
	newTimeStamp = (Kbs_TimeStamp *)malloc(sizeof(Kbs_TimeStamp));
	check = gettimeofday(&newTimeStamp->realtime, NULL);
	if (check != 0) {
		printf("Unable to get the time via gettimeofday\n");
	}
	check = times(&newTimeStamp->systemTime);
	if (check == -1) {
		printf("Unable to get the time via times\n");
	}
	//kbs_show_TimeStamp(newTimeStamp);
	return newTimeStamp;
}

/*--------------------------------------------------------------------------*/
void kbs_show_TimeStamp(Kbs_TimeStamp *tStamp){
	if (tStamp == NULL) {
		return;
	}
	printf("The real time: ");
	printf("\t%d.%06d seconds\n", (int)tStamp->realtime.tv_sec, (int)tStamp->realtime.tv_usec);
	printf("The CPU and System times:\n");
	printf("\t CPU time for this process:    %10.4f\n", (tStamp->systemTime.tms_utime)/(Kbs_Double)sysconf(_SC_CLK_TCK));
	printf("\t System time for this process: %10.4f\n", (tStamp->systemTime.tms_stime)/(Kbs_Double)sysconf(_SC_CLK_TCK));
	printf("\t CPU time for child processes: %10.4f\n", (tStamp->systemTime.tms_cutime)/(Kbs_Double)sysconf(_SC_CLK_TCK));
	printf("\t System time for child processes: %10.4f\n", (tStamp->systemTime.tms_cstime)/(Kbs_Double)sysconf(_SC_CLK_TCK));
}

/*--------------------------------------------------------------------------*/
struct timeval *kbs_get_DifferenceTimeval(struct timeval *first, struct timeval *second) {
	struct timeval *result;
	if (first == NULL || second == NULL) {
		KBS_ERROR(KBS_ERROR_NULLPOINTER);
		return NULL;
	}
	result = (struct timeval *) malloc(sizeof(struct timeval));
	if (result == NULL) {
		KBS_ERROR(KBS_ERROR_MALLOC);
	}
	if (first->tv_sec > second->tv_sec || (first->tv_sec == second->tv_sec && first->tv_usec > second->tv_usec)){
		result = kbs_get_DifferenceTimeval(second, first);
		return result;
	}
  /* If there is no carry*/
  if (first->tv_usec < second->tv_usec) {
		result->tv_sec = second->tv_sec - first->tv_sec;
		result->tv_usec = second->tv_usec - first->tv_usec;
		return result;
	}
  /* If there is a carry*/
  else {
		result->tv_usec = 1000000 - first->tv_usec + second->tv_usec;
		result->tv_sec = second->tv_sec - 1 - first->tv_sec;
		return result;
	}
}

/*--------------------------------------------------------------------------*/
struct tms *kbs_get_DifferenceTms(struct tms *first, struct tms *second) {
	struct tms *result;
	if (first == NULL || second == NULL) {
		KBS_ERROR(KBS_ERROR_NULLPOINTER);
		return NULL;
	}
	result = (struct tms *) malloc(sizeof(struct tms));
	if (result == NULL) {
		KBS_ERROR(KBS_ERROR_MALLOC);
	}
	if (first->tms_utime > second->tms_utime){
		result = kbs_get_DifferenceTms(second, first);
		return result;
	}
	result->tms_utime = second->tms_utime-first->tms_utime;
	result->tms_stime = second->tms_stime-first->tms_stime;
	result->tms_cutime = second->tms_cutime-first->tms_cutime;
	result->tms_cstime = second->tms_cstime-first->tms_cstime;
	return result;
}


/*--------------------------------------------------------------------------*/
Kbs_TimeStamp *kbs_get_DifferenceTimeStamp(Kbs_TimeStamp *first, Kbs_TimeStamp *second){
	Kbs_TimeStamp *result;
	if (first == NULL || second == NULL) {
		KBS_ERROR(KBS_ERROR_NULLPOINTER);
		return NULL;
	}
	result = (Kbs_TimeStamp *) malloc(sizeof(Kbs_TimeStamp));
	if (result == NULL) {
		KBS_ERROR(KBS_ERROR_MALLOC);
	}
	if (first->realtime.tv_sec > second->realtime.tv_sec ||
			(first->realtime.tv_sec == second->realtime.tv_sec &&
			 first->realtime.tv_usec > second->realtime.tv_usec)){
		result = kbs_get_DifferenceTimeStamp(second, first);
		return result;
	}
  /* If there is no carry*/
  if (first->realtime.tv_usec < second->realtime.tv_usec) {
		result->realtime.tv_sec = second->realtime.tv_sec - first->realtime.tv_sec;
		result->realtime.tv_usec = second->realtime.tv_usec - first->realtime.tv_usec;
	}
  /* If there is a carry*/
  else {
		result->realtime.tv_usec = 1000000 - first->realtime.tv_usec + second->realtime.tv_usec;
		result->realtime.tv_sec = second->realtime.tv_sec - 1 - first->realtime.tv_sec;
	}
	result->systemTime.tms_utime = second->systemTime.tms_utime-first->systemTime.tms_utime;
	result->systemTime.tms_stime = second->systemTime.tms_stime-first->systemTime.tms_stime;
	result->systemTime.tms_cutime = second->systemTime.tms_cutime-first->systemTime.tms_cutime;
	result->systemTime.tms_cstime = second->systemTime.tms_cstime-first->systemTime.tms_cstime;
	return result;
}

