#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <memory>
#include <vector>
#include "ThreadPool.h"
#include "Karatsuba.h"

std::vector<int> polyn1;
std::vector<int> polyn2;
std::vector<int> result;
std::mutex mutex;

void construct_polynomials(int n, int m);
void sequencial_multiply(int m, int n);
void compute_coefficient(int coeff);
void compute_multiplication(int no_of_threads, int thread_number);
void parallelized_multiply(int no_of_threads);
void karatsuba_algorithm_parallel(int no_of_threads);


int main() {
	std::cout << "Give size of polynomials: ";
	int n, m;
	std::cin >> n >> m;

	construct_polynomials(n, m);

	for (int i = 0; i < n; i++) {
		std::cout << polyn1[i] << " ";
	}
	std::cout << "\n";
	for (int i = 0; i < m; i++) {
		std::cout << polyn2[i] << " ";
	}
	auto start = std::chrono::high_resolution_clock::now();

	sequencial_multiply(n, m);
	//parallelized_multiply(10);
	//result = karatsuba_sequential(polyn1, polyn2);
	karatsuba_algorithm_parallel(3);

	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "\ntime " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 1000000;
	std::cout << "\n";
	for (auto elem : result) {
		std::cout << elem;
		std::cout << " ";
	}

	return 0;
}

void construct_polynomials(int n, int m) {
	polyn1 = std::vector<int>(n, 0);
	polyn2 = std::vector<int>(m, 0);

	result = std::vector<int>(n + m - 1, 0);

	for (int i = 0; i < n; i++) {
		polyn1[i] = rand() % 10;
	}

	for (int i = 0; i < m; i++) {
		polyn2[i] = rand() % 10;
	}
}

void sequencial_multiply(int m, int n)
{
	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < n; j++)
			result[i + j] += polyn1[i] * polyn2[j];
	}
}

void parallelized_multiply(int no_of_threads) {
	std::thread threads[100];

	for (int i = 0; i < no_of_threads; i++) {
		threads[i] = std::thread(compute_multiplication, no_of_threads, i);
	}

	for (int i = 0; i < no_of_threads; i++) {
		threads[i].join();
	}
}

void compute_coefficient(int coeff) {
	for (int i = 0; i <= coeff; i++) {
		for (int j = 0; j <= coeff; j++) {
			if (i + j == coeff && i < polyn1.size() && j < polyn2.size()) {
				mutex.lock();
				result[coeff] += polyn1[i] * polyn2[j];
				mutex.unlock();
			}
		}
	}
}

void compute_multiplication(int no_of_threads, int thread_number) {
	int no_of_coeff = (polyn1.size() + polyn2.size() - 1) / no_of_threads;
	int index = thread_number * no_of_coeff;
	int carry = (polyn1.size() + polyn2.size() - 1) % no_of_threads;

	int upper_bound = (thread_number + 1) * no_of_coeff;
	if (thread_number == no_of_threads - 1)
		upper_bound += carry;

	for (int i = index; i < upper_bound; i++) {
		compute_coefficient(i);
	}
}

void karatsuba_algorithm_parallel(int no_of_threads) {
	auto p1 = std::make_shared<std::vector<int>>(polyn1);
	auto p2 = std::make_shared<std::vector<int>>(polyn2);
	auto res = std::make_shared<std::vector<int>>(2 * polyn1.size(), 0);
	auto poolPtr = std::make_unique<ThreadPool>(no_of_threads);

	multiply_karatsuba(poolPtr, p1, 0, p1->size(), p2, 0, p2->size(), res);

	for (int i = 0; i < result.size(); i++) {
		result[i] = (*res)[i];
	}
}