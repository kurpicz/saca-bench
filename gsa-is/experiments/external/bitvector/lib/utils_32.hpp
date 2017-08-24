#ifndef UTILS_H
#define UTILS_H

#include <cstdarg>
#include <cerrno>
#include <cstring>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <ctime>
#include <cstdint>
#include <cinttypes>
#include <cmath>

#ifndef M64
	#define M64 0
#endif

#if M64
	typedef int64_t  int_t;
	typedef uint64_t uint_t;
	#define PRIdN	 PRId64
	#define U_MAX 	 UINT64_MAX
	#define I_MAX 	 INT64_MAX
	#define I_MIN 	 INT64_MIN
#else
	typedef int32_t  int_t;
	typedef uint32_t uint_t;
	#define PRIdN	 PRId32
	#define U_MAX 	 UINT32_MAX
	#define I_MAX	 INT32_MAX
	#define I_MIN	 INT32_MIN
#endif

typedef uint32_t int_text;


/**********************************************************************/

#define swap(a,b) do { typeof(a) aux_a_b = (a); (a) = (b); (b) = aux_a_b; } while (0)

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

void   time_start(time_t *t_time, clock_t *c_clock);
double time_stop(time_t t_time, clock_t c_clock);

void die(const char* where);
void dies(const char* where, char* format, ...);

/**********************************************************************/

#endif
