/*#################################################################################
##
##   R package Copula by Jun Yan and Ivan Kojadinovic Copyright (C) 2008, 2009
##
##   This file is part of the R package copula.
##
##   The R package copula is free software: you can redistribute it and/or modify
##   it under the terms of the GNU General Public License as published by
##   the Free Software Foundation, either version 3 of the License, or
##   (at your option) any later version.
##
##   The R package copula is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##
##   You should have received a copy of the GNU General Public License
##   along with the R package copula. If not, see <http://www.gnu.org/licenses/>.
##
#################################################################################*/


/*****************************************************************************

  Goodness-of-fit tests for copulas 

*****************************************************************************/

#include <R.h>
#include <Rmath.h>
#include <R_ext/Applic.h>
#include "Anfun.h"

/***********************************************************************
  
  Testing the symmetry of A using the CFG or Pickands estimators
  Derivavtives based on Cn
 
***********************************************************************/

void evsymtest(double *U, double *V, int *n, double *t, int *m, 
	       int *CFG, int *N, double *s0)
{
  double *influ = Calloc((*n) * (*m), double);
  double *random = Calloc(*n, double);
 
  double *S = Calloc(*n, double);
  double *T = Calloc(*n, double);
  double *Sp = Calloc(*n, double);
  double *Tp = Calloc(*n, double);
  double *Sm = Calloc(*n, double);
  double *Tm = Calloc(*n, double);

  double sumt, sum1t, At, A1t, process, mean, 
    invsqrtn = 1.0/sqrt(*n), minTkSi, minSkTi, 
    lb = 1.0 / (*n + 1.0), ub = *n / (*n + 1.0),
    cA0, cA1;

  int i, j, k, l; 

  /* temporary arrays */
  for (i = 0; i < *n; i++)
    {
      S[i] = - log(U[i]);
      T[i] = - log(V[i]);
      Sp[i] = - log(MIN(U[i] + invsqrtn, ub));
      Tp[i] = - log(MIN(V[i] + invsqrtn, ub));
      Sm[i] = - log(MAX(U[i] - invsqrtn, lb));
      Tm[i] = - log(MAX(V[i] - invsqrtn, lb));
    }
  
  /* correction terms */
  if (*CFG)
    {
      cA0 = log_A_CFG(*n, S, T, 0.0);
      cA1 = log_A_CFG(*n, S, T, 1.0);
    }
  else
    {
      cA0 = inv_A_Pickands(*n, S, T, 0.0); 
      cA1 = inv_A_Pickands(*n, S, T, 1.0);
    }
  
  /* for each point of the grid */
  for (j = 0; j < *m; j++) 
    {
      if (*CFG)
	{
	  At = exp(log_A_CFG(*n, S, T, t[j])
		   - (1.0 - t[j]) * cA0 - t[j] * cA1);
	  A1t = exp(log_A_CFG(*n, S, T, 1.0 - t[j])
		   - t[j] * cA0 - (1.0 - t[j]) * cA1);
	}
      else
	{ 
	  At = 1.0 / (inv_A_Pickands(*n, S, T, t[j]) 
		      - (1.0 - t[j]) * (cA0  - 1.0)
		      - t[j] * (cA1 - 1.0));
	  A1t = 1.0 / (inv_A_Pickands(*n, S, T, 1.0 - t[j]) 
		       - t[j] * (cA0  - 1.0)
		       - (1.0 - t[j]) * (cA1 - 1.0));
	}
      
      for (i = 0; i < *n; i++) /* for each pseudo-obs */
	{ 
	  sumt = 0.0;
	  sum1t = 0.0;
	  for (k = 0; k < *n; k++)
	    { 
	      /* sum for t */
	      minTkSi = MIN(T[k] / t[j], S[i] / (1.0 - t[j]));
	      minSkTi = MIN(S[k] / (1.0 - t[j]), T[i] / t[j]);
	      if (*CFG)
		sumt += - log(MIN(Sm[k] / (1.0 - t[j]), minTkSi))
		  + log(MIN(Sp[k] / (1.0 - t[j]), minTkSi))
		  - log(MIN(Tm[k] / t[j], minSkTi))
		  + log(MIN(Tp[k] / t[j], minSkTi));
	      else
		sumt += MIN(Sm[k] / (1.0 - t[j]), minTkSi)
		  - MIN(Sp[k] / (1.0 - t[j]), minTkSi)
		  + MIN(Tm[k] / t[j], minSkTi)
		  - MIN(Tp[k] / t[j], minSkTi);
	      
	      /* sum for 1 - t */
	      minTkSi = MIN(T[k] / (1.0 - t[j]), S[i] / t[j]);
	      minSkTi = MIN(S[k] / t[j], T[i] / (1.0 - t[j]));
	      if (*CFG)
		sum1t += - log(MIN(Sm[k] / t[j], minTkSi))
		  + log(MIN(Sp[k] / t[j], minTkSi))
		  - log(MIN(Tm[k] / (1.0 - t[j]), minSkTi))
		  + log(MIN(Tp[k] / (1.0 - t[j]), minSkTi));
	      else
		sum1t += MIN(Sm[k] / t[j], minTkSi)
		  - MIN(Sp[k] / t[j], minTkSi) 
		  + MIN(Tm[k] / (1.0 - t[j]), minSkTi)
		  - MIN(Tp[k] / (1.0 - t[j]), minSkTi);	      
	    }
	  sumt *= invsqrtn / 2.0;
	  sum1t *= invsqrtn / 2.0;

	  /* influence term */
	  if (*CFG)
	    influ[i + j * (*n)] = 
	      At * (-log(MIN(S[i] / (1.0 - t[j]), T[i] / t[j])) - sumt)
	      - A1t * (-log(MIN(S[i] / t[j], T[i]/ (1.0 - t[j]))) - sum1t);
	  else
	    influ[i + j * (*n)] = 
	      - At * At * (MIN(S[i] / (1.0 - t[j]), T[i] / t[j]) - sumt)
	      + A1t * A1t * (MIN(S[i] / t[j], T[i]/ (1.0 - t[j])) - sum1t);
	  
	  influ[i + j * (*n)] *=  invsqrtn;
	}
    }

  GetRNGstate();

  /* generate N approximate realizations */
  for (l = 0; l < *N; l++)
    {
      /* generate n variates */
      mean = 0.0;
      for (i=0;i<*n;i++)
	{
	  random[i] = norm_rand(); /*(unif_rand() < 0.5) ? -1.0 : 1.0 ;*/
	  mean += random[i];
	}
      mean /= (double)(*n);
      
      /* realization number l */
      s0[l] = 0.0;
      for (j = 0; j < *m; j++)
	{ 
	  process = 0.0;
	  for (i = 0; i < *n; i++)
	    process += (random[i] - mean) * influ[i + j * (*n)];

	  s0[l] += process * process;
	}
      s0[l] /= (double)(*m); 
    }

  PutRNGstate();


  Free(influ);
  Free(random);
  Free(S);
  Free(T);
  Free(Sp);
  Free(Tp);
  Free(Sm);
  Free(Tm);
}

/***********************************************************************
  
  Testing the symmetry of A using the CFG or Pickands estimators
  Derivatives based on An
 
***********************************************************************/

double intgrd(double x, double At, double A1t, double termUt, double termVt, 
	      double termU1t, double termV1t, double powUt, double powVt, 
	      double powU1t, double powV1t, double U, double V, double t, double n) 
{
  double x1t = R_pow(x,1-t), xt = R_pow(x,t);
  double indUt = (U <= x1t) - (int)(x1t * (n+1)) / n;
  double indVt = (V <= xt) - (int)(xt * (n+1)) / n;
  double indU1t = (U <= xt) - (int)(xt * (n+1)) / n; 
  double indV1t = (V <= x1t) - (int)(x1t * (n+1)) / n;
  double xlogx = x * log(x), res = 0.0;
  if (indUt) res +=  At * termUt * R_pow(x,powUt) * indUt / xlogx;
  if (indVt) res += At * termVt * R_pow(x,powVt) * indVt / xlogx;
  if (indU1t) res -=  A1t * termU1t * R_pow(x,powU1t) * indU1t / xlogx;
  if (indV1t) res -= A1t * termV1t * R_pow(x,powV1t) * indV1t / xlogx;
  return res;
} 

void vec_intgrd(double *x, int n, void *ex) 
{
  int i;
  double *arg = ex;
  for (i = 0; i < n; i++) 
    x[i] = intgrd(x[i], arg[0], arg[1], arg[2], arg[3], 
		  arg[4], arg[5], arg[6], arg[7], arg[8],
		   arg[9], arg[10], arg[11], arg[12], arg[13]);
  return;
}

void evsymtest_derA(double *U, double *V, int *n, double *t, int *m, 
		    int *CFG, int *N, double *s0)
{
  double *influ = Calloc((*n) * (*m), double);
  double *influ2 = Calloc((*n) * (*m), double);
  double *random = Calloc(*n, double);
 
  double *S = Calloc(*n, double);
  double *T = Calloc(*n, double);

  double At, A1t, Atm, Atp, A1tm, A1tp, dAt, dA1t, 
    tj, tjp, tjm, termUt, termVt, termU1t, termV1t,
    powUt, powVt, powU1t, powV1t, process, mean, 
    invsqrtn = 1.0/sqrt(*n), cA0, cA1;

  int i, j, l; 

  /* for numerical integration: begin */ 
  double result, abserr;
  int last, neval, ier;
  int limit=100;
  double reltol=0.0001;
  double abstol=0.0001;
  int lenw = 4 * limit;
  int *iwork = Calloc(limit, int);
  double *work = Calloc(lenw, double);
  double ex[14], lower = 0.0, upper = 1.0; 
  /* lower = 1.0 / (*n + 1), upper = *n / (*n + 1.0); */
  /* for numerical integration: end */ 

  /* temporary arrays */
  for (i = 0; i < *n; i++)
    {
      S[i] = - log(U[i]);
      T[i] = - log(V[i]);
    }
  
  /* correction terms */
  if (*CFG)
    {
      cA0 = log_A_CFG(*n, S, T, 0.0);
      cA1 = log_A_CFG(*n, S, T, 1.0);
    }
  else
    {
      cA0 = inv_A_Pickands(*n, S, T, 0.0); 
      cA1 = inv_A_Pickands(*n, S, T, 1.0);
    }
  
  /* for each point of the grid */
  for (j = 0; j < *m; j++) 
    {
      if (*CFG)
	{
	  /* t terms */
	  tj = t[j];
	  tjp = tj + invsqrtn;
	  tjm = tj - invsqrtn;
	  At = exp(log_A_CFG(*n, S, T, tj)
		   - (1.0 - tj) * cA0 - tj * cA1);
	  Atp = exp(log_A_CFG(*n, S, T, tjp)
		   - (1.0 - tjp) * cA0 - tjp * cA1);
	  Atm = exp(log_A_CFG(*n, S, T, tjm)
		   - (1.0 - tjm) * cA0 - tjm * cA1);
	  dAt = (Atp - Atm) / (2.0 * invsqrtn);
	  termUt = At - tj * dAt;
	  termVt = At + (1.0 - tj) * dAt;
	  powUt = At + tj - 1.0;
	  powVt = At - tj;

	  /* 1 - t terms */
	  tj = 1.0 - t[j];
	  tjp = tj + invsqrtn;
	  tjm = tj - invsqrtn;
	  A1t = exp(log_A_CFG(*n, S, T, tj)
		   - (1.0 - tj) * cA0 - tj * cA1);
	  A1tp = exp(log_A_CFG(*n, S, T, tjp)
		   - (1.0 - tjp) * cA0 - tjp * cA1);
	  A1tm = exp(log_A_CFG(*n, S, T, tjm)
		   - (1.0 - tjm) * cA0 - tjm * cA1);
	  dA1t = (A1tp - A1tm) / (2.0 * invsqrtn);
	  termU1t = A1t - tj * dA1t;
	  termV1t = A1t + (1.0 - tj) * dA1t;
	  powU1t = A1t + tj - 1.0;
	  powV1t = A1t - tj;
	}
      else /* Pickands */
	{ 
	  /* t terms */
	  tj = t[j];
	  tjp = tj + invsqrtn;
	  tjm = tj - invsqrtn;
	  
	  At = 1.0 / (inv_A_Pickands(*n, S, T, tj) 
		      - (1.0 - tj) * (cA0  - 1.0)
		      - t[j] * (cA1 - 1.0));
	  Atp = 1.0 / (inv_A_Pickands(*n, S, T, tjp) 
		       - (1.0 - tjp) * (cA0  - 1.0)
		       - tjp * (cA1 - 1.0));
	  Atm = 1.0 / (inv_A_Pickands(*n, S, T, tjm) 
		       - (1.0 - tjm) * (cA0  - 1.0)
		       - tjm * (cA1 - 1.0));
	  dAt = (Atp - Atm) / (2.0 * invsqrtn);
	  termUt = (At - tj * dAt) / (At + tj - 1.0);
	  termVt = (At + (1.0 - tj) * dAt) / (At - tj);
	  powUt = (At + tj - 1.0) / (1.0 - tj);
	  powVt = (At - tj) / tj;
	  
	  /* 1 - t terms */
	  tj = 1.0 - t[j];
	  tjp = tj + invsqrtn;
	  tjm = tj - invsqrtn;
	  A1t = 1.0 / (inv_A_Pickands(*n, S, T, tj) 
		      - (1.0 - tj) * (cA0  - 1.0)
		      - t[j] * (cA1 - 1.0));
	  A1tp = 1.0 / (inv_A_Pickands(*n, S, T, tjp) 
		       - (1.0 - tjp) * (cA0  - 1.0)
		       - tjp * (cA1 - 1.0));
	  A1tm = 1.0 / (inv_A_Pickands(*n, S, T, tjm) 
		       - (1.0 - tjm) * (cA0  - 1.0)
		       - tjm * (cA1 - 1.0));
	  dA1t = (A1tp - A1tm) / (2.0 * invsqrtn);
	  termU1t = (A1t - tj * dA1t) / (A1t + tj - 1.0);
	  termV1t = (A1t + (1.0 - tj) * dA1t) / (A1t - tj);
	  powU1t = (A1t + tj - 1.0) / (1.0 - tj);
	  powV1t = (A1t - tj) / tj;
	}
      
      for (i = 0; i < *n; i++) /* for each pseudo-obs */
	{ 
	  if (*CFG)
	    {
	      ex[0] = At; ex[1] = A1t;
	      ex[2] = termUt; ex[3] = termVt; 
	      ex[4] = termU1t; ex[5] = termV1t;
	      ex[6] = powUt; ex[7] = powVt;
	      ex[8] = powU1t; ex[9] = powV1t;
	      ex[10] = U[i]; ex[11] = V[i];
	      ex[12] = t[j]; ex[13] = *n;
	      
	      Rdqags(vec_intgrd, (void *)ex, &lower, &upper,
		     &abstol,  &reltol,
		     &result,  &abserr,  &neval,  &ier,
		     &limit,  &lenw, &last,
		     iwork, work);

	      /*if (ier)
		Rprintf("%lf %lf %d\n", result, abserr, ier);*/
	     
	      /* influence matrices */	      
	      influ[i + j * (*n)] = - At * log(MIN(S[i] / (1.0 - t[j]), T[i] / t[j]))
		+ A1t * log(MIN(S[i] / t[j], T[i]/ (1.0 - t[j]))); 
	      
	      influ2[i + j * (*n)] =  - result; 
	    }
	  else /* Pickands */
	    influ[i + j * (*n)] = 
	      - At * At * (MIN(S[i] / (1.0 - t[j]), T[i] / t[j]) 
			   - termUt * (1.0 - R_pow(U[i],powUt))
			   - termVt * (1.0 - R_pow(V[i],powVt)))
	      + A1t * A1t * (MIN(S[i] / t[j], T[i]/ (1.0 - t[j])) 
			     - termU1t * (1.0 - R_pow(U[i],powU1t))
			     - termV1t * (1.0 - R_pow(V[i],powV1t)));
	  
	  influ[i + j * (*n)] *=  invsqrtn;
	  influ2[i + j * (*n)] *=  invsqrtn;
	}
    }
      
  GetRNGstate();
  
  /* generate N approximate realizations */
  for (l = 0; l < *N; l++)
    {
      /* generate n variates */
      mean = 0.0;
      for (i=0;i<*n;i++)
	{
	  random[i] = norm_rand(); /*(unif_rand() < 0.5) ? -1.0 : 1.0 ;*/
	  mean += random[i];
	}
      mean /= (double)(*n);
      
      /* realization number l */
      s0[l] = 0.0;
      for (j = 0; j < *m; j++)
	{ 
	  process = 0.0;
	  for (i = 0; i < *n; i++)
	    if (*CFG)
	      process += (random[i] - mean) * influ[i + j * (*n)] 
		+ random[i] * influ2[i + j * (*n)];
	    else
	      process += (random[i] - mean) * influ[i + j * (*n)];

	  s0[l] += process * process;
	}
      s0[l] /= (double)(*m); 
    }

  PutRNGstate();


  Free(influ);
  Free(influ2);
  Free(random);
  Free(S);
  Free(T);
  Free(iwork);
  Free(work);
}

/***********************************************************************
  
  Test statistic for testing the symmetry of A 

***********************************************************************/
    

void evsymtest_stat(double *S, double *T, int *n, double *t, int *m, 
		    int *CFG, double *stat)
{
  int j;
  double s = 0.0, diff, cA0, cA1;

  if (*CFG)
    {
      /* correction terms */
      cA0 = log_A_CFG(*n, S, T, 0.0);
      cA1 = log_A_CFG(*n, S, T, 1.0);
      
      for (j = 0; j < *m; j++)
	{ 
	  diff = exp(log_A_CFG(*n, S, T, t[j])
		     - (1.0 - t[j]) * cA0 - t[j] * cA1)
	    - exp(log_A_CFG(*n, S, T, 1.0 - t[j])
		  - t[j] * cA0 - (1.0 - t[j]) * cA1);
	  s += diff * diff;
	}
    }
  else
    {
      /* correction terms */
      cA0 = inv_A_Pickands(*n, S, T, 0.0); 
      cA1 = inv_A_Pickands(*n, S, T, 1.0);
     
      for (j = 0; j < *m; j++)
	{ 
	  diff = 1.0/(inv_A_Pickands(*n, S, T, t[j])
		      - (1.0 - t[j]) * (cA0  - 1.0)
		      - t[j] * (cA1 - 1.0)) 
	    - 1.0/(inv_A_Pickands(*n, S, T, 1.0 - t[j])
		   - t[j] * (cA0  - 1.0)
		   - (1.0 - t[j]) * (cA1 - 1.0)); 
	  s += diff * diff;
	}
    }
        
  *stat = s * (double)(*n) / (double)(*m);
}

/***********************************************************************
  
 Exchangeability test based on Cn
 
***********************************************************************/

double Cn(double *U, double *V, int n, double u, double v)
{
  int i;
  double res = 0.0;
  
  for (i = 0; i < n; i++)
      res += (U[i] <= u) * (V[i] <= v);
    return res/n;
}

double der1Cn(double *U, double *V, int n, double u, double v)
{
  double invsqrtn = 1.0 / sqrt(n);
  return (Cn(U, V, n, u + invsqrtn, v) - Cn(U, V, n, u - invsqrtn, v)) 
    / (2.0 * invsqrtn);
}

double der2Cn(double *U, double *V, int n, double u, double v)
{
  double invsqrtn = 1.0 / sqrt(n);
  return (Cn(U, V, n, u ,v + invsqrtn) - Cn(U, V, n, u ,v - invsqrtn)) 
    / (2.0 * invsqrtn);
}

void exchtestCn(double *U, double *V, int *n, double *u, double *v, 
		int *m, int *N, double *s0)
{
  double *influ = Calloc((*n) * (*m), double);
  double *random = Calloc(*n, double);

  double d1uv, d2uv, d1vu, d2vu, process, mean;

  int i, j, l; 

  /* for each point of the grid */
  for (j = 0; j < *m; j++) 
    {
      d1uv = der1Cn(U, V, *n, u[j], v[j]);
      d2uv = der2Cn(U, V, *n, u[j], v[j]);
      
      d1vu = der1Cn(U, V, *n, v[j], u[j]);
      d2vu = der2Cn(U, V, *n, v[j], u[j]);
      
      for (i = 0; i < *n; i++) /* for each pseudo-obs */
	{
	  influ[i + j * (*n)] = (U[i] <= u[j]) * (V[i] <= v[j]) 
	    - d1uv * (U[i] <= u[j]) - d2uv * (V[i] <= v[j])
	    - (U[i] <= v[j]) * (V[i] <= u[j]) 
	    + d1vu * (U[i] <= v[j]) + d2vu * (V[i] <= u[j]);
	  
	  influ[i + j * (*n)] /= sqrt(*n);
	}
    }

  GetRNGstate();

  /* generate N approximate realizations */
  for (l = 0; l < *N; l++)
    {
      /* generate n variates */
      mean = 0.0;
      for (i=0;i<*n;i++)
	{
	  random[i] = norm_rand(); /*(unif_rand() < 0.5) ? -1.0 : 1.0 ;*/
	  mean += random[i];
	}
      mean /= *n;
      
      /* realization number l */
      s0[l] = 0.0;
      for (j = 0; j < *m; j++)
	{ 
	  process = 0.0;
	  for (i = 0; i < *n; i++)
	    process += (random[i] - mean) * influ[i + j * (*n)];

	  s0[l] += process * process;
	}
      s0[l] /= *m; 
    }

  PutRNGstate();


  Free(influ);
  Free(random);
}

/***********************************************************************
  
  Statistic for the test of exchangeability based on Cn

***********************************************************************/
 

    
void exchtestCn_stat(double *U, double *V, int *n, double *u, double *v, 
		     int *m, double *stat)
{
  int j;
  double s = 0.0, diff;
  
  for (j = 0; j < *m; j++)
    { 
      diff = Cn(U, V, *n, u[j], v[j]) - Cn(U, V, *n, v[j], u[j]);
      s += diff * diff;
    }
  
  *stat = s * (*n) / *m;
}
