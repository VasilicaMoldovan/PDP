#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include "ThreadPool.h"

#define NO_OF_THREADS 50

void construct_matrices(int type, int n, int m);
void test_tasks_threads(int no_of_tasks);
void test_thread_pool(int thread_pool_size, int no_of_tasks);
void compute_one_element(int index);
void compute_task1(int task_number, int no_of_tasks);
void compute_task2(int task_number, int no_of_tasks);
void compute_task3(int task_number, int no_of_tasks);
std::vector<std::vector<int>> compute_matrix_product(int type, int no_of_tasks);

std::vector<std::vector<int>> m1;
std::vector<std::vector<int>> m2;
std::vector<std::vector<int>> p;

int main() {
	int n, m;
	std::cout << "First matrix dimensions: ";
	std::cin >> n >> m;
	construct_matrices(1, n, m);

	std::cout << "Second matrix dimensions: ";
	std::cin >> n >> m;
	construct_matrices(2, n, m);

	p = std::vector<std::vector<int>>(m1.size() + 1);
	for (int i = 0; i < m1.size(); i++) {
		p[i] = std::vector<int>(m2[0].size() + 1, 0);
	}

	std::cout << "1. First approach(threads number = tasks number)\n";
	std::cout << "2. Second approach(thread pool)\n";
	
	int approach = 0;
	std::cin >> approach;

	if (approach == 1) {
		//Approach1
		std::cout << "Number of tasks/threads ";
		std::cin >> approach;
		test_tasks_threads(approach);
	}
	else {
		//Approach2
		std::cout << "Number of tasks ";
		std::cin >> approach;
		std::cout << "Size of thread pool ";
		int s_pool = 0;
		std::cin >> s_pool;
		test_thread_pool(s_pool, approach);
	}

	return 0;
}

void construct_matrices(int type, int n, int m) {
	std::vector<int> aux;
	if (type == 1) {
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < m; j++) {
				aux.push_back(j + 1);
			}
			m1.push_back(aux);
			aux.clear();
		}
	}
	else {

		for (int i = 0; i < n; i++) {
			for (int j = 0; j < m; j++) {
				aux.push_back(j + 1);
			}
			m2.push_back(aux);
			aux.clear();
		}
	}
}

void test_tasks_threads(int no_of_tasks) {
	auto start = std::chrono::high_resolution_clock::now();
	std::thread th[NO_OF_THREADS];
	for (int i = 0; i < no_of_tasks; i++) {
		th[i] = std::thread(compute_task3, i, no_of_tasks);
	}

	for (int i = 0; i < no_of_tasks; i++) {
		th[i].join();
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "For " << no_of_tasks << " threads, where there is a thread for each task, the execution time is " 
		<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "\n";

	for (int i = 0; i < m1.size(); i++) {
		for (int j = 0; j < m2[0].size(); j++) {
			std::cout << p[i][j] << " ";
		}
		std::cout << "\n";
	}
}

void test_thread_pool(int thread_pool_size, int no_of_tasks) {
	auto start = std::chrono::high_resolution_clock::now();
	ThreadPool& pool = ThreadPool::getInstance(thread_pool_size); //create pool with 4 threads

	auto returnValue = pool.push(compute_matrix_product, 1, no_of_tasks);

	auto product = returnValue.get();

	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "For " << thread_pool_size << " threads, and " <<  no_of_tasks << " tasks, the execution time is " 
		<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "\n";

	for (int i = 0; i < m1.size(); i++) {
		for (int j = 0; j < m2[0].size(); j++) {
			std::cout << product[i][j] << " ";
		}
		std::cout << "\n";
	}

}

void compute_one_element(int index) { 
	int element = 0;

	int row = index / m2[0].size();
	int column = index % m2[0].size();


	for (int i = 0; i < m2.size(); i++) {
		element += m1[row][i] * m2[i][column];
	}

	p[row][column] = element;
}

void compute_task1(int task_number, int no_of_tasks) {
	int no_of_elems = m1.size() * m2[0].size() / no_of_tasks;

	int i = no_of_elems * task_number;
	if (task_number == no_of_tasks - 1)
		no_of_elems += m1.size() * m2[0].size() % no_of_tasks;

	int j = 0;
	while (j < no_of_elems) {
		compute_one_element(i);
		j++;
		i++;
	}

}

void compute_task2(int task_number, int no_of_tasks) {
	int no_of_elems = m1.size() * m2[0].size() / no_of_tasks;

	int i = no_of_elems * task_number;
	i = (i % m1.size()) * m2[0].size() + i / m1.size();
	if (task_number == no_of_tasks - 1)
		no_of_elems += m1.size() * m2[0].size() % no_of_tasks;

	int j = 0;
	while (j < no_of_elems) {
		compute_one_element(i);
		j++;
		i += m2[0].size();
		if (i  != m1.size() * m2[0].size() - 1)
			i = i % (m1.size() * m2[0].size() - 1);
	}
}

void compute_task3(int task_number, int no_of_tasks) {
	int no_of_elems = m1.size() * m2[0].size() / no_of_tasks;
	int i = task_number;

	if (task_number < m1.size() * m2[0].size() % no_of_tasks)
		no_of_elems++;

	int j = 0;
	while (j < no_of_elems) {
		compute_one_element(i);
		j++;
		i += no_of_tasks;
	}
}

std::vector<std::vector<int>> compute_matrix_product(int type, int no_of_tasks) {
	for (int i = 0; i < no_of_tasks; i++) {
		if (type == 1)
			compute_task1(i, no_of_tasks);
		else if (type == 2)
			compute_task2(i, no_of_tasks);
		else
			compute_task3(i, no_of_tasks);
	}
	return p;
}