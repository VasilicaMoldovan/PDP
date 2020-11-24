#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>

std::vector<int> polyn1;
std::vector<int> polyn2;
std::vector<int> result;

void construct_polynomials(int n, int m);
void compute_coefficient(int coeff);
void compute_multiplication(int no_of_threads);
void karatsuba_algorithm();

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

	return 0;
}

void construct_polynomials(int n, int m) {
	polyn1 = std::vector<int>(n, 0);
	polyn2 = std::vector<int>(m, 0);

	result = std::vector<int>(n + m, 0);

	for (int i = 0; i < n; i++) {
		polyn1[i] = rand() % 10;
	}

	for (int i = 0; i < m; i++) {
		polyn2[i] = rand() % 10;
	}
}