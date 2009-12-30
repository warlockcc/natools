#include "cnash.h"
#include "calculus.h"

/* calculates de derivate of f at f(x0)
 *             _______
 * |         _/
 * |       _/
 * |      /
 * |     /
 * |______________________
 *    x0-h   x0    x0+h
 * */
int derivate_3p(function_t *f, double x0, double h, double *r) {
  double p0, p1;
    
  if (evaluate_function(f, x0 - h, &p0)) return 1;
  if (evaluate_function(f, x0 + h, &p1)) return 1;

  *r = (p1 - p0) / (2.0 * h);
  return 0;
}

int derivate_5p(function_t *f, double x0, double h, double *r) {
  double p0, p1, p2, p3;
    
  if (evaluate_function(f, x0 - 2.0 * h, &p0)) return 1;
  if (evaluate_function(f, x0 - 1.0 * h, &p1)) return 1;
  if (evaluate_function(f, x0 + 1.0 * h, &p2)) return 1;
  if (evaluate_function(f, x0 + 2.0 * h, &p3)) return 1;

  *r = (p0 + 8.0 * (p2 - p1) - p3) / (12.0 * h);
  return 0;
}

/* composite simpson's rule:
 * n: the resolution points used to calculate
 * actually half of the points) */
int integrate_simpson(function_t *f, interval_t *i, int n, double *r) {
  double h = (i->x1 - i->x0) / (2.0 * n);
  double p0, pm, pi, pj, fj;
  int j;

  if (evaluate_function(f, i->x0, &p0)) return 1;
  if (evaluate_function(f, i->x1, &pm)) return 1;

  for (j = 1, pj = 0.0; j < n; j++) {
    if (evaluate_function(f, i->x0 + h * (2*j), &fj)) return 1;
    pj += fj;
  }
  for (j = 1, pi = 0.0; j <= n; j++) {
    if (evaluate_function(f, i->x0 + h * (2*j-1), &fj)) return 1;
    pi += fj;
  }

  *r = (p0 + 2.0 * pj + 4.0 * pi + pm) * h / 3.0;
  return 0;
}

/* vim: set sw=2 sts=2 : */
