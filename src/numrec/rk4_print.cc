/* =================================================================	*/
/* Numerical Recipes Runge Kutta routine (with all single precision
   floats changed to double precision doubles).				*/
/* =================================================================	*/
/* Given values for the variables y[1..n] and their derivatives dydx[1..n]
   known at x, use the fourth-order Runge-Kutta method to advance the solution
   over an interval h and return the incremented variables as yout[1..n],
   which need not be a distinct array from y.  The user supplies the
   routine derivs(x,y,dydx) which returns derivatives dydx at x.	*/
/* =================================================================	*/

void rk4(double y[], double dydx[], int n, double x, double h, double yout[],
	 void (*derivs)(double, double [], double []))
{
    int i;
    double xh,hh,h6,*dym,*dyt,*yt;


    printf("At checkpoint 1 inside rk4.cc \n");
    printf("n = %d \n",n);
    for (i=1; i<=n; i++)
    {
       printf("i = %d   y[i] = %f   dydx[i] = %f \n",
              i-1,y[i],dydx[i]);
    }
    

    dym=vector(1,n);
    dyt=vector(1,n);
    yt=vector(1,n);
    hh=h*0.5;
    h6=h/6.0;
    xh=x+hh;

    printf("At checkpoint 2 inside rk4.cc \n");
    for (i=1;i<=n;i++) 
    {
       yt[i]=y[i]+hh*dydx[i];
       printf("i = %d   y[i] = %f   dydx[i] = %f  yt[i] = %f \n",
              i-1,y[i],dydx[i],yt[i]);
    }
    (*derivs)(xh,yt,dyt);
    printf("After first call to derivs from within rk4.cc \n");
    for (i=1; i<=n; i++)
    {
       printf("rk4 i = %d  yt[rk4 i] = %f  dyt[rk4 i] = %f \n",
              i,yt[i],dyt[i]);
    }

    printf("At checkpoint 3 inside rk4.cc \n");
    for (i=1;i<=n;i++) 
    {
       yt[i]=y[i]+hh*dyt[i];
       printf("i = %d   y[i] = %f   dydx[i] = %f  yt[i] = %f \n",
              i-1,y[i],dydx[i],yt[i]);
    }
    (*derivs)(xh,yt,dym);

    printf("At checkpoint 4 inside rk4.cc \n");
    for (i=1;i<=n;i++)
    {
	yt[i]=y[i]+h*dym[i];
	dym[i] += dyt[i];
        printf("i = %d   y[i] = %f   dydx[i] = %f  yt[i] = %f \n",
               i-1,y[i],dydx[i],yt[i]);
    }
    (*derivs)(x+h,yt,dyt);

    printf("At checkpoint 5 inside rk4.cc \n");
    for (i=1;i<=n;i++)
    {
       yout[i]=y[i]+h6*(dydx[i]+dyt[i]+2.0*dym[i]);
       printf("i = %d   y[i] = %f   dydx[i] = %f  yout[i] = %f \n",
               i-1,y[i],dydx[i],yout[i]);
    }
    
    free_vector(yt,1,n);
    free_vector(dyt,1,n);
    free_vector(dym,1,n);
}






