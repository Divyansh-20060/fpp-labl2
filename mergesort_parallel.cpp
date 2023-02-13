/*

speed up for 1 = 0.482
speed up for 2 = 0.8344
speed up for 3 = 1.061
speed up for 4 = 1.23

*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define PARALLEL 1
#ifdef PARALLEL
#include "hclib_cpp.h"
#endif
/*
 * The integer array to sort
 */
int* array = NULL;
/*
 * The buffer used for copying during merge step
 */
int* buffer = NULL;
/*
 * Size of the array to sort
 */
int size = 0;

/*
 * Used for measuring the execution time
 */
long get_usecs () {
  struct timeval t;
  gettimeofday(&t,NULL);
  return t.tv_sec*1000000+t.tv_usec;
}

void merge(int M, int mid, int N) {
  // make a copy in buffer[M:N]
  for (int k = M; k <= N; k++) {
    buffer[k] = array[k];
  }

  // merge from buffer to array
  int i = M;
  int j = mid + 1;
  for (int k = M; k <= N; k++) {
    if (i > mid) {
      array[k] = buffer[j++];
    } else if (j > N) {
      array[k] = buffer[i++];
    } else if (buffer[j] < buffer[i]) {
      array[k] = buffer[j++];
    } else {
      array[k] = buffer[i++];
    }
  }
}

/*
 * Mergesort implementation using recursion
 */
void msort(int M, int N) {
  if (M < N) {
    int mid = M + (N - M) / 2;
    
#ifdef PARALLEL
    HCLIB_FINISH {
    hclib::async([&]() {msort(M, mid);});
    msort(mid + 1, N);
    }
    
#else
    msort(M, mid);
    msort(mid + 1, N);
#endif
    merge(M, mid, N);
  }
}

int main(int argc, char** argv) {
#ifdef PARALLEL
  hclib::launch([&]() {
#endif
  // For this lab evaluation we would be using size = 1000000
  size = argc>1?atoi(argv[1]):1000000;

  // allocate array
  array = new int[size];
  // allocate buffer used for copying during merge step
  buffer = new int[size];

  // initialize array elements with random numbers
  srand(1);
  for(int i=0; i<size; i++) {
    array[i] = (int) rand();
  }

  long start = get_usecs();
  // start mergesort
  msort(0, size-1);
  long end = get_usecs();
  // calculate execution time
  double dur = ((double)(end-start))/1000000;

  // test if sort was success
  bool fail = false;
  for(int i=1; i<size; i++) {
    if(array[i] < array[i-1]) {
      fail = true;
      break;
    }
  }

  if(fail) {
    printf("MergeSort: FAILED\n");
  }
  else {
    printf("MergeSort was SUCCESS. Total time taken = %.3f seconds\n",dur);
  }

  // release memory
  delete(buffer);
  delete(array);
#ifdef PARALLEL
  });
#endif
  return 0;
}

