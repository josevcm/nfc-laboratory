/* ************************************************************************ 
   FILE:	lms.c
   COMMENT:	Optimal FIR filter design by LMS algorithm.
   CONTENT:
		double Case1F(double w, int i)
		double Case2F(double w, int i)
		double Case3F(double w, int i)
		double Case4F(double w, int i)

   AUTHOR:	Lime Microsystems
   DATE:	Feb 24, 2000
   REVISION:
   ************************************************************************ */
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif 

#include "lms.h"
#include "recipes.h"
#include "rounding.h"

/* Filter parity constants */
#define EVEN    0
#define ODD     1

/* Declare functions from Numerical Recipes that are used here */
double  *vector();
int  *ivector();
double  **matrix();
    
/* ************************************************************************ 
 *  Trigonometric functions for CASE1, CASE2, CASE3 and CASE4 filters
 *
 *  INPUTS:
 *  double	w	- Normalised frequency [0, 0.5]
 *  int		i
 *
 *  RETURN VALUE:
 *  Value of trigonometric function
 * ************************************************************************ */
double Case1F(double w, int i)
{
	return( cos( 2.0*M_PI*w*((double)(i)-1.0)) );
}

double Case2F(double w, int i)
{
	return( cos( 2.0*M_PI*w*((double)(i)-0.5)) );
}

double Case3F(double w, int i)
{
	return( sin( 2.0*M_PI*w*(double)(i)) );
}

double Case4F(double w, int i)
{
	return( sin( 2.0*M_PI*w*((double)(i)-0.5)) );
}

/* ************************************************************************ 
 *	OUTPUT:
 *	double 	hr[n]		- Filter impulse response
 *	double 	hi[n]		- Filter impulse response rounded to integer
 *	double 	hcsd[n]		- Filter impulse response rounded to CSD
 *
 *	INPUTS:
 *	int 	n		- Number of taps
 *	double	w[p]		- Frequency points
 *	double	des[p]		- Filter desired response
 *	double	weight[p]	- Wighting function
 *	int	p		- Number of points on a frequency scale,
 *	int	cprec		- Desired coefficients precision,
 *	int	csdprec		- CSD coefficients precision,
 *	int	symmetry	- Filter response symmetry
 *
 *	RETURN VALUE:
 *	0			- if everything is OK
 *	-1			- otherwise
 * ************************************************************************ */
int lms(hr, hi, hcsd, n, w, des, weight, p, cprec, csdprec, symmetry,
	bincode, csdcode, csdcoder)
double *hr, *hi, *hcsd;
int n;
double *w;
double *des;
double *weight;
int p;
int cprec;
int csdprec;
int symmetry;
int **bincode, **csdcode, **csdcoder;
{

	/* All this is for  solving a linear system */
	/* of equations by LU decomposition */
	double **A, d;
	int *index;

	/* Parameters of real function Hr(w) */
	double *a; 		/* Coefficients */
	int L;			/* Number of terms in Hr(w) sum */
	double (*f)();		/* Trigonometric function in Hr(w) */
	
	int parity;		/* Parity of the filter (ODD or EVEN) */
	int i, j, k;		/* Loop counters */
	double fwki, fwkj;	/* Used for equation formulation */

	/* Check the correctness of inputs */
	if( (hr == NULL) || (w == NULL) || 
	    (des == NULL) || (weight == NULL)) return(-1);
	if( n == 0) return(-1);
	if( (symmetry != POSITIVE) && (symmetry != NEGATIVE) ) return(-1);

	/* Determine parity */
	parity = EVEN;
	if( n % 2 ) parity = ODD;

	/* Calculate L. If we use integer arithmetic, L is n/2 */
	/* for all cases except for CASE1 filters */
	L = n/2;
	if( (symmetry == POSITIVE) && (parity == ODD) ) L++;

	/* Find which trigonometric function to use depending on filter type */
	if( (symmetry == POSITIVE) && (parity == ODD) ) { 		/* Case 1 */
		f = Case1F; 
	} else if( (symmetry == POSITIVE) && (parity == EVEN) ) {	/* Case 2 */
		f = Case2F;
	} else if( (symmetry == NEGATIVE) && (parity == ODD) ) {	/* Case 2 */
		f = Case3F;
	} else if( (symmetry == NEGATIVE) && (parity == EVEN) ) {	/* Case 2 */
		f = Case4F;
	} else {	/* This should never happen but ... */
		return(-1);
	}

	/* Allocate memory for a[1:L], A[1:L][1:L] and index[1:L]. */
	/* Functions from Numerical Recipes are used because I hate  */
	/* FORTRAN like indexing (starting from 1, not 0). */
	a = vector(1, L);
	A = matrix(1, L, 1, L);
	index = ivector(1, L);

	/* Set A, a and index all to zeroes */
	for(j=1; j <= L; j++) {
		a[j] = 0.0;
		index[j] = 0;
		for(i=1; i <= L; i++) A[i][j] = 0.0;
	}

	/* OK, ready to fill up the equations */
	for(k=0; k<p; k++) {
		for(j=1; j <= L; j++) {
			fwkj = (f)(w[k], j);
			A[j][j] += weight[k]*fwkj*fwkj;
			a[j] += weight[k]*des[k]*fwkj;
			for(i=j+1; i <= L; i++) {
				fwki = (f)(w[k], i);
				A[i][j] += weight[k]*fwki*fwkj;
				A[j][i] += weight[k]*fwkj*fwki;
			}
		}
	}

	/* Solve the equations */
	ludcmp(A, L, index, &d); 
	lubksb(A, L, index, a);

	/* Calculate impulse response h[] from a[] */
	for(i=0; i<n; i++) hr[i] = 0.0;
	for(i=0; i<L; i++) hr[i] = 0.5*a[L-i];

	/* Resolve CASE1 for i=L-1 */
	if((symmetry == POSITIVE) && (parity == ODD)) hr[L-1] = a[1];

	/* Construct other half of hr[] using symmetry */
	for(i=0; i<n/2; i++) hr[n-i-1] = symmetry * hr[i];

	/* Round the filter coefficients to the nearest integer value */
	round2int(hr, hi, n, cprec);

	/* Round the filter coefficients to the nearest CSD code */
	round2csd(hr, hcsd, n, cprec, csdprec, bincode, csdcode, csdcoder);

	/* Free allocated memory */
	free_vector(a, 1, L);
	free_matrix(A, 1, L, 1, L);
	free_ivector(index, 1, L);

	/* That's all, let's go home */
	return(0);
}
