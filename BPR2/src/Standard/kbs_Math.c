
#include "kbs_Math.h"
#include "kbs_Error.h"


/*--------------------------------------------------------*/
Kbs_Ulong kbs_power_Ulong(Kbs_Ulong base, Kbs_Ulong exp) {
	long p;
	if (exp == 0) {
		return 1;
	}
	if (exp == 1) {
		return base;
	}
	if (base == 4) {
		if (exp > 15) {
			KBS_ERROR(KBS_ERROR_VALUEOVERFLOW);
		}
		return 4<<(2*(exp-1));
	}
	p=1;
	for (; exp>0; --exp) {
		p = p*base;
	}
	return p;
}

/*--------------------------------------------------------*/
Kbs_Long kbs_getExp_Ulong(const Kbs_Ulong base, const Kbs_Ulong value){
	Kbs_Long exp = 0;
	Kbs_Ulong tmpValue = 1;
	while (tmpValue < value) {
		tmpValue *= base;
		exp++;
	}
	if (tmpValue == value) {
		return exp;
	}
	else {
		return -1;
	}
}
