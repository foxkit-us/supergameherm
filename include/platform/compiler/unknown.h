#ifndef __UNKNOWN_H__
#define __UNKNOWN_H__

#ifndef UNUSED
#	define UNUSED
#endif

#ifndef unlikely
#	define unlikely(x) (x)
#endif

#ifndef likely
#	define likely(x) (x)
#endif

#warning "Your compiler is unknown to us, but we're trying anyway. Please report your compiler to us."
#warning "Everything should work out for the most part, but you may get suboptimal results"

#endif /*__UNKNOWN_H__*/
