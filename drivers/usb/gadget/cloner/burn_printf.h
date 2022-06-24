#ifndef __LOG_H__
#define __LOG_H__

typedef struct {
	int enable;
	//int (*printf)(const char *fmt, ...);
} log_t;

log_t log;

#define L	log
//#define CHECK_STATUS()		(L.enable && !!L.printf)
#define CHECK_STATUS()			(L.enable & 1)

#define BURNNER_PRI(fmt, ...) do {		\
	if(CHECK_STATUS())                      \
		printf(fmt, ##__VA_ARGS__);	\
} while (0)

#if 0
#define printf(fmt, ...) do {			\
	if(CHECK_STATUS())			\
	L.printf(fmt, ##__VA_ARGS__);		\
} while (0)
#endif
#endif /*__LOG_H__*/
