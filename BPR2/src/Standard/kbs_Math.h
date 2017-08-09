
#ifndef KBS_MATH_H
#define KBS_MATH_H

#include "kbs_Error.h"
#include "kbs_Types.h"

#define MAX(a,b)							(((a) > (b)) ? (a) : (b))
#define MIN(a,b)							(((a) < (b)) ? (a) : (b))

#define KBS_ABS(a)					(((a) < 0) ? (-a) : (a))


/**
 * Computes base^exp
 * @param base - base
 * @param exp  - exponent
 * @return base^exp
 */
Kbs_Ulong kbs_power_Ulong(Kbs_Ulong base, Kbs_Ulong exp);

/**
 * Computes exp of base^exp = value
 * @param base   - base
 * @param value  - value of base^exp
 * @return exp of base^exp = value, if not integer -1
 */
Kbs_Long kbs_getExp_Ulong(const Kbs_Ulong base, const Kbs_Ulong value);


#endif
