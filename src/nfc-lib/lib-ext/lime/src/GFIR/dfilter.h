/* ************************************************************************ 
   FILE:	dfilter.h
   COMMENT:	Define some structures used in DSP calculations.
   CONTENT:
   AUTHOR:	Lime Microsystems
   DATE:	Feb 8, 2000
   REVISION:
   ************************************************************************ */
#define POINTS  256	/* Maximum number of points to calculate */
//#define POINTS  40000	/* Maximum number of points to calculate */

/* Digital filter transfer function parameters */
struct dfilter {
	int m;		/* Polinomial order of the denominator */
	int n;		/* Polinomial order of the nominator */
	double *a;	/* Array of denominator coefficients */
	double *b;	/* Array of nominator coefficients */
	double *w;	/* Points on a frequency grid */
	int nw;		/* Number of points in *w */
	double amplitude[POINTS];	/* To store calculated data */
	double phase[POINTS];
	double logamp[POINTS];		/* Amplitude in log scale */
	double max;			/* Maximum of amplitude */
};