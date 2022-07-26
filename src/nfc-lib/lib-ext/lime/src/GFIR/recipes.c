/* *******************************************************************
FILE:		recipes.c
DESCRIPTION:	Routines for vector and matrix allocation/free.
		Linear system solving by LU decomposition.
		All are from "Numerical Recipes in C".

CONTAINTS:	
		double  *vector(nl,nh)
		int    nl,nh;

		int  *ivector(nl,nh)
		int  nl,nh;

		void free_vector(v,nl,nh)
		double *v;
		int   nl,nh;

		void free_ivector(v,nl,nh)
		int *v;
		int   nl,nh;

		double  **matrix(nrl,nrh,ncl,nch)
		int  nrl,nrh,ncl,nch;

		void free_matrix(m,nrl,nrh,ncl,nch)
		double **m;
		int   nrl,nrh,ncl,nch;

		nrerror(error_text)
		char error_text[];

		void ludcmp(a,n,indx,d)
		int n, *indx;
		double **a,*d;

		void lubksb(a,n,indx,b)
		double **a,b[];
		int n,indx[];

DATE:		
AUTHOR:		From "Numerical Recipes in C"
REVISIONS:	February 01, 1994: File created.
		Febryary 24, 2000:
			This file is taken out from my PhD sources, Srdjan.
			Unnecessary functions are removed.
			All floats are converted to doubles.
  ********************************************************************* */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "recipes.h"

#define TINY 1.0e-20

void nrerror(char * error_text); 

/*****************************************************/
/*** Allocates a double vector with range [nl..nh]. ***/
/*****************************************************/
double  *vector(nl,nh)
int    nl,nh;
{
	double *v;

	v = (double *) malloc((unsigned) (nh-nl+1)*sizeof(double));
	if (!v) nrerror("Allocation failure in vector().\n");
	return (v-nl);
}

/*****************************************************/
/*** Allocates an int vector with range [nl..nh].  ***/
/*****************************************************/
int  *ivector(nl,nh)
int  nl,nh;
{
	int *v;

	v = (int *) calloc((unsigned) (nh-nl+1),sizeof(int));
	if (!v) nrerror("Allocation failure in vector().\n");
	return (v-nl);
}

/****************************************************/
/*** Frees a double vector allocated by vector().  ***/
/****************************************************/
void free_vector(v,nl,nh)
double *v;
int   nl,nh;
{
	free((char*) (v+nl));
}

/****************************************************/
/*** Frees an int vector allocated by ivector().  ***/
/****************************************************/
void free_ivector(v,nl,nh)
int *v;
int   nl,nh;
{
	free((char*) (v+nl));
}

/*******************************************/
/****  Allocates a double matrix with    ****/
/****  range [nrl..nrh] [ncl..nch].     ****/
/*******************************************/
double  **matrix(nrl,nrh,ncl,nch)
int  nrl,nrh,ncl,nch;
{
	int    i;
	double  **m;

	/*** Allocate pointers to rows.   ****/
	m = (double**) calloc((unsigned) (nrh-nrl+1),sizeof(double*));
	if (!m) nrerror("Allocation failure #1 in matrix().");  
	m -= nrl;

   
	/***  Allocate rows and pointers to them.  ***/
	for (i=nrl; i<=nrh; i++)  {
		m[i] = (double *) malloc((unsigned) (nch-ncl+1)*sizeof(double));
		if (!m[i]) nrerror("Allocation failure #2 in matrix().");
		m[i] -= ncl;
	}
	return(m);
}

/******************************************/
/****   Frees a matrix allocated by ... ***/
/****   matrix().                       ***/
/******************************************/
void free_matrix(m,nrl,nrh,ncl,nch)
double **m;
int   nrl,nrh,ncl,nch;
{
	int i;
 
	for (i=nrh; i>=nrl; i--)  free((char*) (m[i]+ncl));
	free ((char*) (m+nrl));
}

/*************************************************/
/*** Numerical Recipes standard error handler. ***/
/*************************************************/
void nrerror(error_text)
char error_text[];
{
	void exit();

	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"Forced to exit.\n");
	exit(1);
}

/* ******************************************************************** */
/* LU Decomposition of a matrix						*/
/*	a[1:n][1:n]		system matrix,				*/
/*	n			problem dimension,			*/
/*	indx[1:n]		temporary storage,			*/
/*	d			temporary storage.			*/
/* ******************************************************************** */
int ludcmp(a,n,indx,d)
int n, *indx;
double **a,*d;
{
	int i,imax,j,k;
	double big,dum,sum,temp;
	double *vv,*vector();
	void free_vector();

	imax = 0; 

	vv=vector(1,n);
	(*d)=1.0;
	for (i=1;i<=n;i++) {
		big=0.0;
		for (j=1;j<=n;j++)
			if ((temp=fabs(a[i][j])) > big) big=temp;
		if (!big) nrerror("Singular matrix in routine LUDCMP");
		vv[i]=1.0/big;
	}
	for (j=1;j<=n;j++) {
		for (i=1;i<j;i++) {
			sum=a[i][j];
			for (k=1;k<i;k++) sum -= a[i][k]*a[k][j];
			a[i][j]=sum;
		}
		big=0.0;
		for (i=j;i<=n;i++) {
			sum=a[i][j];
			for (k=1;k<j;k++)
				sum -= a[i][k]*a[k][j];
			a[i][j]=sum;
			if ((dum=vv[i]*fabs(sum))>=big) {
				big=dum;
				imax=i;
			}
		}
		if (j!=imax) {
			for (k=1;k<=n;k++) {
				dum=a[imax][k];
				a[imax][k]=a[j][k];
				a[j][k]=dum;
			}
			(*d)= -(*d);
			vv[imax]=vv[j];
		}
		indx[j]=imax;
		if(fpclassify(a[j][j]) == FP_ZERO) {
			a[j][j]=TINY;
		} 
		if (j!=n) {
			dum=1.0/(a[j][j]);
			for (i=j+1;i<=n;i++) a[i][j] *= dum;
		}
        }
	free_vector(vv,1,n);
	return(0);

} /* end of ludcmp() */

/* ******************************************************************** */
/* Solve system of equation						*/
/* ******************************************************************** */
void lubksb(a,n,indx,b)
double **a,b[];
int n,indx[];
{
	int i,ii=0,ip,j;
	double sum;

	for (i=1;i<=n;i++) {
		ip=indx[i];
		sum=b[ip];
		b[ip]=b[i];
		if (ii) {
			for (j=ii;j<=i-1;j++) sum -= a[i][j]*b[j];
		} else if (sum) {
			ii=i;
		}
		b[i]=sum;
	}
	for (i=n;i>=1;i--) {
		sum=b[i];
		for (j=i+1;j<=n;j++) sum -= a[i][j]*b[j];
		b[i]=sum/a[i][i];
	}

}   /*  end of lubksb() */

