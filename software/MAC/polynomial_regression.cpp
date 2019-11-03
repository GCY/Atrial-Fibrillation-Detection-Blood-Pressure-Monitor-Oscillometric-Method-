#include "polynomial_regression.h"

std::vector<double> PolynomialRegression(std::vector<double> &x,std::vector<double> &y,unsigned int degree)
{
   std::vector<double> a(degree + 1);

   std::vector<double> X(2 * degree + 1);                        //Array that will store the values of sigma(xi),sigma(xi^2),sigma(xi^3)....sigma(xi^2n)
   for (int i = 0;i < 2 * degree + 1 ;++i){
      X[i] = 0;
      for (int j = 0;j < x.size() ;++j){
	 X[i] = X[i] + std::pow(x[j],i);        //consecutive positions of the array will store N,sigma(xi),sigma(xi^2),sigma(xi^3)....sigma(xi^2n)
      }
   }

   std::vector<std::vector<double> > B(degree + 1,std::vector<double>(degree + 2));            //B is the Normal matrix(augmented) that will store the equations, 'a' is for value of the final coefficients
   for(int i = 0;i <= degree;++i){
      for(int j = 0;j <= degree;++j){
	 B[i][j] = X[i+j];            //Build the Normal matrix by storing the corresponding coefficients at the right positions except the last column of the matrix
      }
   }

   std::vector<double> Y(degree + 1);                    //Array to store the values of sigma(yi),sigma(xi*yi),sigma(xi^2*yi)...sigma(xi^n*yi)
   for(int i = 0;i < degree + 1;++i){    
      Y[i] = 0;
      for(int j = 0;j < x.size();++j){
	 Y[i] = Y[i] + std::pow(x[j],i) * y[j];        //consecutive positions will store sigma(yi),sigma(xi*yi),sigma(xi^2*yi)...sigma(xi^n*yi)
      }
   }
   for(int i = 0;i <= degree;++i){
      B[i][degree + 1] = Y[i];                //load the values of Y as the last column of B(Normal Matrix but augmented)
   }

   degree += 1;                //n is made n+1 because the Gaussian Elimination part below was for n equations, but here n is the degree of polynomial and for n degree we get n+1 equations

   for(int i=0;i < degree;++i){                   //From now Gaussian Elimination starts(can be ignored) to solve the set of linear equations (Pivotisation)
      for(int k = i + 1;k < degree;++k){
	 if(B[i][i] < B[k][i]){
	    for(int j = 0;j <= degree;++j){
	       double temp = B[i][j];
	       B[i][j] = B[k][j];
	       B[k][j] = temp;
	    }
	 }
      }
   }


   for(int i = 0;i < degree - 1;++i){            //loop to perform the gauss elimination
      for(int k = i + 1;k < degree;++k){
	 double t = B[k][i] / B[i][i];
	 for(int j = 0;j <= degree;++j){
	    B[k][j] = B[k][j] - t * B[i][j];    //make the elements below the pivot elements equal to zero or elimnate the variables
	 }
      }   
   }

   for(int i = degree - 1;i >= 0;--i){                //back-substitution
      //x is an array whose values correspond to the values of x,y,z..
      a[i] = B[i][degree];                //make the variable to be calculated equal to the rhs of the last equation
      for(int j = 0;j < degree;++j){
	 if(j != i){            //then subtract all the lhs values except the coefficient of the variable whose value                                   is being calculated
	    a[i] = a[i] - B[i][j] * a[j];
	 }
      }
      a[i] = a[i] / B[i][i];            //now finally divide the rhs by the coefficient of the variable to be calculated
   }

   return a;
}
