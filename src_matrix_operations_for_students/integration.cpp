#include <cmath>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include "include/Stopwatch.hpp"
#include <vector>
#include <omp.h>

#define NUM_THREADS 4
double f(double x) {
    return 3*x*x;
}

double simple_num_integral(int n) {
    double res = 0;
    for (int i = 0; i < n; i++) {
        double xi = (i + 0.5)/n;
        res += f(xi)/n;
    }
    return res;
}

double parallel_num_integral(int n) {
    std::vector<double> res(n);
    if (omp_get_max_threads() < n) std::cerr << "Max threads exceeded" << std::endl;
    omp_set_num_threads(n);
    // parallel calculation of f(x)
    #pragma omp parallel shared(res)
    {
        int num_thread = omp_get_thread_num();
        int x = (num_thread + 0.5) / n;
        res[num_thread] = f(x) / n;
    }
    // wait for all calculaitons to finish
    #pragma omp barrier
    // reduce 
    int stride = 1;
    while (stride < n) {
        #pragma omp parallel shared(res) 
        {
            int num_thread = omp_get_thread_num();
            if (num_thread % stride == 0) {
                if (num_thread + (stride - 1) < n) {
                    res[num_thread] = res[num_thread] + res[num_thread + (stride - 1)];
                } else {
                    res[num_thread] = res[num_thread];
                }
            }
        }
        stride *= 2;
        #pragma omp barrier
    }
    return res[0];
}



int find_n_accurate() {
    double correct = 1.0;
    int n = 1;
    while (true) {
        double res = simple_num_integral(n);
        if (abs(correct - res) < 0.00000001)
            break;
        n++;
    }
    return n;
}

int main() {
    Stopwatch stopwatch;
    stopwatch.start();
    int n = find_n_accurate();
    stopwatch.stop();
    double elapsed_time = stopwatch();
    std::cout << "n = " << n << std::endl;
    std::cout << "Time elapsed: " << elapsed_time << "s" << std::endl;
}