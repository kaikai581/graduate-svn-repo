#include <cstdlib>
#include <iostream>
#include <omp.h>

using namespace std;

int main(int argc, char* argv[])
{
  int thread_count;
  if(argc == 1) thread_count = 1;
  else thread_count = atoi(argv[1]);
  cout << "number of threads: " << thread_count << endl;
  int x = 5;
# pragma omp parallel num_threads(thread_count) private(x)
  {
    int my_rank = omp_get_thread_num();
# pragma omp critical
    cout << "Thread " << my_rank << " > before initialization, x = " << x << endl;
    x = 2*my_rank + 2;
# pragma omp critical
    cout << "Thread " << my_rank << " > after initialization, x = " << x << endl;
  }
  cout << "After parallel block, x = " << x << endl;
}
