/* ************************************************************************ 
   FILE:	rounding.c
   COMMENT:	Functions related to filter coefficients rounding to
		integer values or CSD form.
   CONTENT:
   AUTHOR:	Lime Microsystems
   DATE:	Jun 18, 2000
   REVISION:	Nov 30, 2007:	Tiger project
   ************************************************************************ */
#include <math.h>
#include <stdio.h>

#include "rounding.h"

void int2csd(int a, int cprec, int csdprec, int * bincode, int * csdcode, int * csdcoder); 
int csd2int(int cprec, int *code);

/* ************************************************************************ 
   ************************************************************************ */
void round2int(a, b, n, cprec)
double *a, *b;
int n, cprec;
{
	int i,k;

	for(i=0; i<n; i++) {
		k = a[i] > 0.0 ? 1 : -1;
		b[i] = (int)(a[i]*(1<<cprec) + k*0.5);
		b[i] /= (double)(1<<cprec);
	}
}

/* ************************************************************************ 
   ************************************************************************ */
void round2csd(a, b, n, cprec, csdprec, bincode, csdcode, csdcoder)
double *a, *b;
int n, cprec, csdprec;
int **bincode, **csdcode, **csdcoder;
{
	int i,k, ia;

	for(i=0; i<n; i++) {
		k = a[i] > 0.0 ? 1 : -1;
		ia = (int)(a[i]*(1<<cprec) + k*0.5);
		int2csd(ia, cprec, csdprec, bincode[i], csdcode[i], csdcoder[i]);
		ia = csd2int(cprec, csdcoder[i]);
		b[i] = (double)(ia)/(double)(1<<cprec);
	}
}

/* ************************************************************************ 
   ************************************************************************ */
void printcode(int ** code, int n, int cprec)
{
	int i, j;
	double sumh, sume, sumo;
	int ih; double h;
	int negative, sign, shift, csdprec;
	int symmetry;

	/* Find maximum nonzero bits per coefficient */
	csdprec = 0;
	for(i=0; i<n; i++) {
		negative = 0;
		for(j=0; j<= cprec; j++) if(code[i][j] != 0) negative++;
		if(negative > csdprec) csdprec = negative;
	}

	/* Check symmetry of the filter */
	if( csd2int(cprec, code[0]) == csd2int(cprec,code[n-1]) ) symmetry = 1;
	else symmetry = -1;

	sumh=0.0; sume=0.0; sumo=0.0;
	for(i=0; i<n; i++) {
		ih = csd2int(cprec, code[i]);
		h = (double)ih/(double)(1<<cprec);
		sumh += fabs(h);
		if(i%2) sumo += fabs(h);
		else sume += fabs(h);

		if ((ih != 0) && (i<(n+1)/2) ) {
			negative = 0;
			for(j=0; j<=cprec; j++) {
				if(code[i][j] == -1) negative ++;
				if(code[i][j] != 0 ) shift = j;
			}

			if(negative <= (csdprec-1)) sign = 1;
			else sign = -1;

			shift = cprec - shift;
			if(fabs(h)*(1<<shift) > 1.0) shift --;

			printf("h(%2d) = %11lg = %2d x (", i, h, sign);
			for(j=cprec; j>=0; j--) {
				if(sign*code[i][j] == 1) {
					// printf(" +1/2^%d", cprec-j-shift);
					printf(" +1/2^%d", cprec-j);
				} else if(sign*code[i][j] == -1) {
					// printf(" -1/2^%d", cprec-j-shift);
					printf(" -1/2^%d", cprec-j);
				}
			}
			// printf(" ) / ( 2^%d )\n", shift);
			printf(" )\n");
		}else if (ih != 0) {
			printf("h(%2d) = %11lg = %2d x h(%2d)\n", 
				i, h, symmetry, n-1-i);
		} else {
			printf("h(%2d) = %11lg\n", i, 0.0);
		}
	}
	printf("Sum of all abs(coefficients): %lg\n", sumh);
	printf("Sum of even coefficients: %lg\n", sume);
	printf("Sum of odd  coefficients: %lg\n\n", sumo);
}

/* ************************************************************************ 
   Print CSD code in the form of two common sub-expressions sharing 
   ************************************************************************ */
void print_cses_code(xpx, xmx, x, n, cprec)
int **xpx, **xmx, **x, n, cprec;
{
	int i, j;
	int symmetry;
	int ixpx, ixmx, ix;
	double h;

	/* Check symmetry of the filter */
	if( (csd2int(cprec, xpx[0]) == csd2int(cprec,xpx[n-1])) &&
	    (csd2int(cprec, xmx[0]) == csd2int(cprec,xmx[n-1])) &&
	    (csd2int(cprec,   x[0]) == csd2int(cprec,x[n-1])) ) symmetry = 1;
	else symmetry = -1;

	for(i=0; i<n; i++) {
		ixpx = csd2int(cprec, xpx[i]);
		ixmx = csd2int(cprec, xmx[i]);
		ix   = csd2int(cprec, x[i]);
		h = 	(1.0+1.0/4.0)*(double)ixpx/(double)(1<<cprec) +
			(1.0-1.0/4.0)*(double)ixmx/(double)(1<<cprec) + 
			(double)ix/(double)(1<<cprec);
		
		if ((fpclassify(h) != FP_ZERO) && (i<(n+1)/2) ) {
			printf("h(%2d) = %11lg = ", i, h);
			if( ixpx ) {
				printf("(1+1/4)x(");
				for(j=cprec; j>=0; j--) {
					if(xpx[i][j] == 1) {
						printf(" +1/2^%d", cprec-j);
					} else if(xpx[i][j] == -1) {
						printf(" -1/2^%d", cprec-j);
					}
				}
				printf(" )");
			}

			if( ixmx ) {
				if( ixpx ) printf(" + (1-1/4)x(");
				else printf("(1-1/4)x(");

				for(j=cprec; j>=0; j--) {
					if(xmx[i][j] == 1) {
						printf(" +1/2^%d", cprec-j);
					} else if(xmx[i][j] == -1) {
						printf(" -1/2^%d", cprec-j);
					}
				}
				printf(" )");
			}

			if( ix ) {
				if( ixpx || ixmx ) printf(" + (");
				else printf("(");

				for(j=cprec; j>=0; j--) {
					if(x[i][j] == 1) {
						printf(" +1/2^%d", cprec-j);
					} else if(x[i][j] == -1) {
						printf(" -1/2^%d", cprec-j);
					}
				}
				printf(" )");
			}

			printf("\n");
		} else if (fpclassify(h) != FP_ZERO) {
			printf("h(%2d) = %11lg = %2d x h(%2d)\n", 
				i, h, symmetry, n-1-i);
		} else {
			printf("h(%2d) = %11lg\n", i, 0.0);
		}
	}
}

/* ************************************************************************ 
   ************************************************************************ */
void int2csd(a, cprec, csdprec, bincode, csdcode, csdcoder)
int a;		/* Input integer to be converted into CSD code */
int cprec;	/* Integer precision */
int csdprec;	/* CSD precistion */
int *bincode;	/* Binary code */
int *csdcode;	/* CSD code */
int *csdcoder;	/* CSD code rounded to 'csdprec' nonzero bits */
{
	int i, sign, ci, ci1, nzeroes;

	if( a < 0 ) {
		a *= -1;
		sign = -1; 
	} else {
		sign = 1;
	}

	/* Generate binary code of input */
	for(i=0; i<cprec; i++) {
		if( a & (1<<i) ) bincode[i] = 1;
		else bincode[i] = 0;
	}
	bincode[cprec] = 0;
	
	/* Construct CSD code */
	ci = 0;
	for(i=0; i<cprec; i++) {
		if( (ci + bincode[i] + bincode[i+1]) > 1 ) ci1 = 1;
		else ci1 = 0;

		csdcode[i] = sign*(bincode[i] + ci -2*ci1);
		bincode[i] *= sign;
		ci = ci1;
	}
	csdcode[cprec] = sign*ci;

	/* Round CSD code */
	nzeroes = 0;
	for(i=cprec; i >= 0 ; i--) {
		if(csdcode[i] != 0) nzeroes ++;

		if(nzeroes <= csdprec) csdcoder[i] = csdcode[i];
		else csdcoder[i] = 0;
	}

}

/* ************************************************************************ 
   ************************************************************************ */
int csd2int(cprec, code)
int cprec, *code;
{
	int i, a;
	
	a = 0;
	for(i=cprec; i>=0; i--) a = a*2 + code[i];

	return(a);
}

/* ************************************************************************ 
	Extract x+x>>2 and x-x>>2 subexpressions from the CSD code
   ************************************************************************ */
void csesh(code, n, cprec, xpx, xmx, x )
int n, cprec;
int **code, **xpx, **xmx, **x;
{
	int i, k;

	/* Set code matrices to zero */
	for(i = 0; i < n; i++) {
		for(k=0; k <= cprec; k++) {
			xpx[i][k] = 0;
			xmx[i][k] = 0;
			x[i][k] = 0;
		}
	}
			
	/* Extract two common subexpressions from all filter coefficients */
	for(i = 0; i <n; i++) {
		k = cprec;
		while(1) {
			/* Find next nonzero element in CSD code */
			for(; k >= 0; k--) if(code[i][k] != 0) break;

			if(k == -1) {		/* There are no more nonzero digits */
				break;
			} else if(k == 0) {	/* It is the last digit */
				x[i][0] = code[i][0];
				break;
			} else if(k == 1) {	/* Two more digits left */
				x[i][0] = code[i][0];
				x[i][1] = code[i][1];
				break;
			} else {
				if((code[i][k] == 1) && (code[i][k-2] == 1) ) {
					xpx[i][k] = 1;
					code[i][k] = 0;
					code[i][k-2] = 0;
				} else if((code[i][k] == -1) && (code[i][k-2] == -1) ) { 
					xpx[i][k] = -1;
					code[i][k] = 0;
					code[i][k-2] = 0;
				} else if((code[i][k] ==  1) && (code[i][k-2] == -1) ) { 
					xmx[i][k] = 1;
					code[i][k] = 0;
					code[i][k-2] = 0;
				} else if((code[i][k] == -1) && (code[i][k-2] ==  1) ) { 
					xmx[i][k] = -1;
					code[i][k] = 0;
					code[i][k-2] = 0;
				} else {
					x[i][k] = code[i][k];
					code[i][k] = 0;
				}
			} 
		}
	}
}
