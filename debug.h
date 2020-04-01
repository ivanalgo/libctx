#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef DEBUG
#define debug_log(fmt, ...)	printf(fmt, __VA_ARGS__)
#else
#define debug_log(fmt, ...)
#endif
#endif /* _DEBUG_H */
