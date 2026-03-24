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

double reduce_num_integral(int n) {
    double res = 0;
    #pragma omp parallel for reduction(+:res)
    for (int i = 0; i < n; i++) {
        double xi = (i + 0.5)/n;
        res += f(xi)/n;
    }
    return res;
}



int find_n_accurate(int method) {
    double correct = 1.0;
    int n = 1;
    while (true) {
        double res;
        if (method == 0) res = simple_num_integral(n);
        else if (method == 1) res = parallel_num_integral(n);
        else res = reduce_num_integral(n);
        if (abs(correct - res) < 0.00000001)
            break;
        n++;
    }
    return n;
}

int main() {
    omp_set_num_threads(NUM_THREADS);
    Stopwatch stopwatch_simple;
    Stopwatch stopwatch_parallel;
    Stopwatch stopwatch_reduce;


    // Benchmark simple integration
    stopwatch_simple.start();
    int n_simple = find_n_accurate(0);
    stopwatch_simple.stop();

    // Benchmark hand-written reduce
    stopwatch_parallel.start();
    int n_parallel = find_n_accurate(1);
    stopwatch_parallel.stop();

    // Benchmark omp reduce
    stopwatch_reduce.start();
    int n_reduce = find_n_accurate(2);
    stopwatch_reduce.stop();


    std::cout << "Results for simple integration:\n";
    std::cout << "n = " << n_simple << "\n";
    std::cout << "Time elapsed: " << stopwatch_simple() << "s\n\n";

    std::cout << "Results for hand-written reduce:\n";
    std::cout << "n = " << n_parallel << "\n";
    std::cout << "Time elapsed: " << stopwatch_parallel() << "s\n\n";

    std::cout << "Results for omp reduce:\n";
    std::cout << "n = " << n_reduce << "\n";
    std::cout << "Time elapsed: " << stopwatch_reduce() << "s" << std::endl;
}