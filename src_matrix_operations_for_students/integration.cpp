#include <cmath>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include "include/Stopwatch.hpp"
#include <vector>
#include <omp.h>

#define NUM_THREADS 8

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
    // int num_threads = omp_get_num_threads();
    int num_threads = NUM_THREADS;
    std::vector<double> res(num_threads, 0.0);
    // parallel calculation of f(x)
    #pragma omp parallel for shared(res)
    for (int i = 0; i < n; i++) {
        double xi = (i + 0.5)/n;
        int threadId = omp_get_thread_num();
        res[threadId] += f(xi)/n;
    }
    // wait for all calculaitons to finish
    #pragma omp barrier

    double result = 0.0;
    for (double partial : res) {
        result += partial;
    }
    return result;
}



int find_n_accurate() {
    double correct = 1.0;
    int n = 1;
    while (true) {
        double res = parallel_num_integral(n);
        if (abs(correct - res) < 0.00000001)
            break;
        n++;
    }
    return n;
}

int main() {
    omp_set_num_threads(NUM_THREADS);
    Stopwatch stopwatch;
    stopwatch.start();
    int n = find_n_accurate();
    stopwatch.stop();
    double elapsed_time = stopwatch();
    std::cout << "n = " << n << std::endl;
    std::cout << "Time elapsed: " << elapsed_time << "s" << std::endl;
}