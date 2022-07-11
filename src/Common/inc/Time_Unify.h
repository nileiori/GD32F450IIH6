#ifndef ____TIME_UNIFY_H____
#define ____TIME_UNIFY_H____

#include "time.h"

#define PI	3.14159265358979323846


typedef struct {	/* time struct */
	time_t time;       /* time (s) expressed by standard time_t */
	double sec;        /* fraction of second under 1 s */
} gtime_t;


int str2time(const char *s, int i, int n, gtime_t *t);
gtime_t epoch2time(const double *ep);
void time2epoch(gtime_t t, double *ep);
gtime_t gpst2time(int week, double sec);
double time2gpst(gtime_t t, int *week);
gtime_t gst2time(int week, double sec);
double time2gst(gtime_t t, int *week);
gtime_t bdt2time(int week, double sec);
double time2bdt(gtime_t t, int *week);
gtime_t timeadd(gtime_t t, double sec);
double timediff(gtime_t t1, gtime_t t2);
gtime_t gpst2utc(gtime_t t);
gtime_t utc2gpst(gtime_t t);
gtime_t gpst2bdt(gtime_t t);
gtime_t bdt2gpst(gtime_t t);
double time2sec(gtime_t time, gtime_t *day);
double utc2gmst(gtime_t t, double ut1_utc);
void time2str(gtime_t t, char *s, int n);
void Bdt2UtcTime(int iBdsWeek, double dBdsTow, double *pUtcTimeArr);

#endif // !____TIME_UNIFY_H____
