#ifndef SPMAT_CLASS_H
#define SPMAT_CLASS_H


#include <iostream>

#include "arraytools.hpp"
#include "spvec.hpp"


template <typename INDEX, typename SCALAR>
class spmat
{
  public:
    spmat(int nrows_, int ncols_, int len_);
    ~spmat();
    
    void resize(int len_);
    
    void print(bool actual=false);
    int insert(const int col, const spvec<INDEX, SCALAR> &x);
    
    int nrows() const {return m;};
    int ncols() const {return n;};
    int get_nnz() const {return nnz;};
    int get_len() const {return len;};
    INDEX* index_ptr() {return I;};
    INDEX* index_ptr() const {return I;};
    INDEX* col_ptr() {return P;};
    INDEX* col_ptr() const {return P;};
    SCALAR* data_ptr() {return X;};
    SCALAR* data_ptr() const {return X;};
  
  protected:
    int m;
    int n;
    int nnz;
    int len;
    int plen;
    INDEX *I;
    INDEX *P;
    SCALAR *X;
  
  private:
    void cleanup();
    INDEX* csc2coo();
};



template <typename INDEX, typename SCALAR>
spmat<INDEX, SCALAR>::spmat(int nrows_, int ncols_, int len_)
{
  arraytools::zero_alloc(len_, &I);
  arraytools::zero_alloc(ncols_+1, &P);
  arraytools::zero_alloc(len_, &X);
  
  arraytools::check_allocs(I, P, X);
  
  m = nrows_;
  n = ncols_;
  
  nnz = 0;
  len = len_;
  plen = ncols_ + 1;
}



template <typename INDEX, typename SCALAR>
spmat<INDEX, SCALAR>::~spmat()
{
  cleanup();
}



template <typename INDEX, typename SCALAR>
void spmat<INDEX, SCALAR>::resize(int len_)
{
  arraytools::realloc(len_, &I);
  arraytools::realloc(len_, &X);
  
  arraytools::check_allocs(I, P, X);
  
  len = len_;
}



template <typename INDEX, typename SCALAR>
void spmat<INDEX, SCALAR>::print(bool actual)
{
  printf("## %dx%d sparse matrix with nnz=%d\n", m, n, nnz);
  
  if (actual)
  {
    printf("I: ");
    for (int ind=0; ind<len; ind++)
      std::cout << I[ind] << " ";
    
    printf("\nP: ");
    for (int ind=0; ind<plen; ind++)
      std::cout << P[ind] << " ";
    
    printf("\nX: ");
    for (int ind=0; ind<len; ind++)
      std::cout << X[ind] << " ";
    
    putchar('\n');
  }
  else
  {
    if (nnz == 0)
    {
      for (int i=0; i<m; i++)
      {
        for (int j=0; j<n; j++)
          std::cout << (SCALAR) 0 << " ";
        
        putchar('\n');
      }
    }
    else
    {
      INDEX* CI = csc2coo();
      for (int i=0; i<m; i++)
      {
        int ind = 0;
        for (int j=0; j<n; j++)
        {
          while (I[ind] < i || CI[ind] < j)
            ind++;
          
          if (I[ind] == i && CI[ind] == j)
            std::cout << X[ind++];
          else
            std::cout << (SCALAR) 0;
          
          putchar(' ');
        }
        
        putchar('\n');
      }
      
      arraytools::free(CI);
    }
  }
  
  putchar('\n');
}



template <typename INDEX, typename SCALAR>
int spmat<INDEX, SCALAR>::insert(const int col, const spvec<INDEX, SCALAR> &x)
{
  // check if a re-alloc is necessary
  if (x.get_nnz() > len - nnz)
    return x.get_nnz() - (len - nnz);
  
  // add the vector to the matrix
  const INDEX *xI = x.index_ptr();
  const SCALAR *xX = x.data_ptr();
  
  int ind = P[col];
  const int xnnz = x.get_nnz();
  for (int xind=0; xind<xnnz; xind++)
  {
    I[ind] = xI[xind];
    X[ind] = xX[xind];
    
    ind++;
  }
  
  for (int ind=col+1; ind<plen; ind++)
    P[ind] += xnnz;
  
  nnz += xnnz;
  
  return 0;
}



template <typename INDEX, typename SCALAR>
void spmat<INDEX, SCALAR>::cleanup()
{
  arraytools::free(I);
  I = NULL;
  
  arraytools::free(P);
  P = NULL;
  
  arraytools::free(X);
  X = NULL;
  
  m = 0;
  n = 0;
  
  nnz = 0;
  len = 0;
}



// convert "column pointer" P into column index
template <typename INDEX, typename SCALAR>
INDEX* spmat<INDEX, SCALAR>::csc2coo()
{
  int j = 0;
  int ind = 0;
  
  INDEX* CI;
  arraytools::alloc(len, &CI);
  if (CI == NULL)
  {
    cleanup();
    throw std::bad_alloc();
  }
  
  for (int c=0; c<plen; c++)
  {
    int diff = P[c+1] - P[c];
    
    while (diff > 0)
    {
      CI[ind] = j;
      
      ind++;
      diff--;
    }
    
    j++;
  }
  
  return CI;
}


#endif
