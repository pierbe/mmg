/* =============================================================================
**  This file is part of the mmg software package for the tetrahedral
**  mesh modification.
**  Copyright (c) Inria - IMB (Université de Bordeaux) - LJLL (UPMC), 2004- .
**
**  mmg is free software: you can redistribute it and/or modify it
**  under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  mmg is distributed in the hope that it will be useful, but WITHOUT
**  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
**  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
**  License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License and of the GNU General Public License along with mmg (in
**  files COPYING.LESSER and COPYING). If not, see
**  <http://www.gnu.org/licenses/>. Please read their terms carefully and
**  use this copy of the mmg distribution only if you accept them.
** =============================================================================
*/
#include "mmg2d.h"

extern int ddebug;
/*simultaneous reduction*/
int simred(double *m1,double *m2,double *m) {
  double  dd,lambda[2],hh[2],det;
  double  maxd1,maxd2,ex,ey,ez,m1i[3],n[4],p[4],pi[4];
  int     i,j,k;
  
 
  /* check diag matrices */
  if ( fabs(m1[1]) < EPSD && fabs(m2[1]) < EPSD ) {
    m[0] = M_MAX(m1[0],m2[0]);
    m[2] = M_MAX(m1[2],m2[2]);
    m[1] = 0.0;
    return(1);
  }
  if ( !MMG2_invmat(m1,m1i) )  return(0); 
  if(ddebug) printf("inv %e %e %e\n",m1[0]*m1i[0]+m1[1]*m1i[1],m1[0]*m1i[1]+m1[1]*m1i[2],
                          m1[1]*m1i[1]+m1[2]*m1i[2]);
  /* eigenvectors */ 
  det = (m1[0] - m1[2])*(m1[0] - m1[2]) + 4*m1[1]*m1[1];
  if(det < EPSD) return(0);
  det = sqrt(det);
  lambda[0] = 0.5*(m1[0] + m1[2] - det); 
  lambda[1] = 0.5*(m1[0] + m1[2] + det);  
  if(ddebug) printf(" m1 lambda : %e %e -- %e %e\n",lambda[0],lambda[1],1./sqrt(lambda[0]),
                                1./sqrt(lambda[1]));
  /* eigenvectors */ 
  det = (m2[0] - m2[2])*(m2[0] - m2[2]) + 4*m2[1]*m2[1];
  if(det < EPSD) return(0);
  det = sqrt(det);
  lambda[0] = 0.5*(m2[0] + m2[2] - det); 
  lambda[1] = 0.5*(m2[0] + m2[2] + det);  
  if(ddebug) printf(" m2 lambda : %e %e -- %e %e\n",lambda[0],lambda[1],1./sqrt(lambda[0]),
                                1./sqrt(lambda[1]));


  /* n = (m1)^-1*m2 : stocke en ligne*/
  n[0] = m1i[0]*m2[0] + m1i[1]*m2[1];
  n[1] = m1i[0]*m2[1] + m1i[1]*m2[2];
  n[2] = m1i[1]*m2[0] + m1i[2]*m2[1];
  n[3] = m1i[1]*m2[1] + m1i[2]*m2[2];
  
  /* eigenvectors */ 
  det = (n[0] - n[3])*(n[0] - n[3]) + 4*n[1]*n[2];
  if(det < EPSD) return(0);
  det = sqrt(det);
  lambda[0] = 0.5*(n[0] + n[3] + det); 
  lambda[1] = 0.5*(n[0] + n[3] - det);  
  if(ddebug) printf("lambda : %e %e -- %e %e\n",lambda[0],lambda[1],0.5*((n[0]+n[3])+
                                sqrt((n[0]+n[3])*(n[0]+n[3])-4*(n[0]*n[3]-n[1]*n[2])))
                                ,0.5*((n[0]+n[3])-
                                sqrt((n[0]+n[3])*(n[0]+n[3])-4*(n[0]*n[3]-n[1]*n[2]))));
  
  if(ddebug) printf("l0 %e\n",lambda[0]*lambda[0]-lambda[0]*(n[0]+n[3])+n[0]*n[3]-n[1]*n[2]);
  if(ddebug) printf("l1 %e\n",lambda[1]*lambda[1]-lambda[1]*(n[0]+n[3])+n[0]*n[3]-n[1]*n[2]);
  if ( fabs(lambda[0]-lambda[1]) < EPSD ) {
    m[0] = m[2] = lambda[0];
    m[1] = 0.0;
    return(1);
  }
  else {
    /* matrix of passage */
    p[0] = n[1]; 
    p[1] = -n[0] + lambda[0]; 
    if(ddebug) printf("n %e %e %e %e\n",n[0],n[1],n[2],n[3]);
    det = p[0]*p[0] + p[1]*p[1];
    det = 1./sqrt(det);
    p[0] *= det;
    p[1] *= det;  
    if(ddebug) printf("%e == %e -- %e == %e \n",n[0]*p[0]+n[1]*p[1],lambda[0]*p[0]
                             ,n[2]*p[0]+n[3]*p[1],lambda[0]*p[1]  ) ;
      
    p[2] = n[1];
    p[3] = -n[0] + lambda[1];
    det = p[2]*p[2] + p[3]*p[3];
    det = 1./sqrt(det);
    p[2] *= det;
    p[3] *= det;
    if(ddebug) printf("%e == %e -- %e == %e \n",n[0]*p[2]+n[1]*p[3],lambda[1]*p[2]
                             ,n[2]*p[2]+n[3]*p[3],lambda[1]*p[3]  ) ;
    
    
    det = p[0]*p[3]-p[1]*p[2];
    if(det < EPSD) return(0);
    det = 1./det;
    pi[0] = det*p[3];
    pi[1] = -det*p[1];
    pi[2] = -det*p[2];
    pi[3] = det*p[0];
    if(ddebug) printf("inv2 %e %e %e %e\n",p[0]*pi[0]+p[2]*pi[1],p[0]*pi[2]+p[2]*pi[3],
                             p[1]*pi[0]+p[3]*pi[1],p[1]*pi[2]+p[3]*pi[3]);  
    if(ddebug) printf("check %e %e %e %e\n",p[0]*lambda[0]*pi[0] + p[2]*lambda[1]*pi[1],
                          p[0]*lambda[0]*pi[2] + p[2]*lambda[1]*pi[3],   
                          p[1]*lambda[0]*pi[0] + p[3]*lambda[1]*pi[1],   
                          p[1]*lambda[0]*pi[2] + p[3]*lambda[1]*pi[3]);  
   if(ddebug) printf("check %e %e %e %e\n",n[0],n[1],n[2],n[3]);  
    /*eigenvalues*/
    ex = p[0];
    ey = p[1];
    maxd1 = ex*(m1[0]*ex+m1[1]*ey) 
          + ey*(m1[1]*ex+m1[2]*ey);
    maxd2 = ex*(m2[0]*ex+m2[1]*ey) 
          + ey*(m2[1]*ex+m2[2]*ey);
    hh[0] = M_MAX(maxd1,maxd2);
    ex = p[2];
    ey = p[3];
    maxd1 = ex*(m1[0]*ex+m1[1]*ey) 
          + ey*(m1[1]*ex+m1[2]*ey);
    maxd2 = ex*(m2[0]*ex+m2[1]*ey) 
          + ey*(m2[1]*ex+m2[2]*ey);
    hh[1] = M_MAX(maxd1,maxd2);
    
    if(ddebug) printf("----------------- hh : %e %e -- %e %e\n",hh[0],hh[1],1./sqrt(hh[0]),1./sqrt(hh[1]));
  
    /* compose matrix tP^-1*lambda*P^-1 */
    m[0] = pi[0]*hh[0]*pi[0] + pi[1]*hh[1]*pi[1];
    m[1] = pi[0]*hh[0]*pi[2] + pi[1]*hh[1]*pi[3];
    m[2] = pi[2]*hh[0]*pi[2] + pi[3]*hh[1]*pi[3];
    
    if(ddebug) printf("sym ? %e == %e\n",m[1],pi[2]*hh[0]*pi[0] + pi[3]*hh[1]*pi[1]);
    //exit(0);
  
    return(1);
  }   
  
  
  
}