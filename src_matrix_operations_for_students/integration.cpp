#include <cmath>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include "include/Stopwatch.hpp"
#include <vector.h>

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
    vector<double> x(n);
    vector<double> res(n);
    
    //for (int i = 0; i < n; i++) {
    #pragma omp parallel
    {
        for (int i = 0; i < n; i++) {
            double xi = (i + 0.5)/n;
            res[i] += f(xi)/n;
        }
    }
    thread_barrier_wait():
    int stride = 2;
    while(stride == num_threads){ //maybe threads?
    if(threadId % stride == 0):
        x[threadId] = x[threadId] + x[threadId + (stride-1)]
    stride *= 2;
    thread_barrier_wait():
    }
    res += f(xi)/n;
    return res;
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