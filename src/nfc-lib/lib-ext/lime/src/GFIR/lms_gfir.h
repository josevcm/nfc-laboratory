/* 
 * File:   lms_gfir.h
 * Author: ignas
 *
 * Created on January 18, 2016, 1:01 PM
 */

#ifndef LMS_GFIR_H
#define	LMS_GFIR_H

#ifdef	__cplusplus
extern "C" {
#endif

void GenerateFilter(int n, double w1, double w2, double a1, double a2, double *coefs);



#ifdef	__cplusplus
}
#endif

#endif	/* LMS_GFIR_H */

