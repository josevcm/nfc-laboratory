/* ************************************************************************ 
   FILE:	rounding.h
   COMMENT:	clean up C isms and fix compiler warnings. 
   CONTENT:
   AUTHOR:	M. H. Reilly 
   DATE:	May 17, 2017
   REVISION:
   ************************************************************************ */
#ifndef ROUND_GFIR_HDR
#define ROUND_GFIR_HDR

void round2int(double * a, double * b, int n, int cprec); 

void round2csd(double * a, double * b, int n, int cprec, int csdprec, 
	       int ** bincode, int ** csdcode, int ** csdcoder);

#endif
