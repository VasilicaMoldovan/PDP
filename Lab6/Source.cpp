#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>

#define THREAD_NO 2

void hamiltonian_cycle_sequential();
bool find_hamiltonian_cycle(std::vector<int>& path, int position);
bool check_vertex(int vertex, std::vector<int> path, int position);
void hamiltonian_cycle_parallel(int no_of_threads);
void read_graph();
void construct_graph();

std::vector<std::vector<int>> graph;
std::vector<int> path_parallel;
std::mutex path_mutex;

int main() {
	construct_graph();
	auto start = std::chrono::high_resolution_clock::now();

	hamiltonian_cycle_sequential();

	//hamiltonian_cycle_parallel(THREAD_NO);

	auto end = std::chrono::high_resolution_clock::now();

	std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	return 0;
}

void read_graph() {
	int n, m;
	std::cin >> n >> m;
	graph = std::vector<std::vector<int>>(n + 1);
	
	int x, y;
	for (int i = 0; i < m; i++) {
		std::cin >> x >> y;
		graph[x].push_back(y);
		//graph[y].push_back(x);
	}
}

void construct_graph() {
	//ex of hamiltonian graph
	// cycle will be :3 -> 1 -> 2 -> 4 -> 5 -> 3
	std::vector<int> aux;
	graph.push_back(aux); // empty vector for node 0
	aux.push_back(2);
	graph.push_back(aux); // [2] for node 1 
	aux = std::vector<int>(0);
	aux.push_back(4);
	graph.push_back(aux); // [4] for node 2 
	aux = std::vector<int>(0);
	aux.push_back(1);
	graph.push_back(aux); // [1] for node 3 
	aux = std::vector<int>(0);
	aux.push_back(5);
	graph.push_back(aux); // [5] for node 4
	aux = std::vector<int>(0);
	aux.push_back(3);
	graph.push_back(aux); // [3] for node 4


	////ex of non-hamiltonian graph
	//std::vector<int> aux;
	//graph.push_back(aux); // empty vector for node 0
	//aux.push_back(2);
	//graph.push_back(aux); // [2] for node 1 
	//aux = std::vector<int>(0);
	//aux.push_back(1);
	//graph.push_back(aux); // [1] for node 3 
	//aux = std::vector<int>(0);
	//aux.push_back(5);
	//graph.push_back(aux); // [5] for node 4
}

bool check_vertex(int vertex, std::vector<int> path, int position) {
	if (std::find(graph[path[position - 1]].begin(), graph[path[position - 1]].end(), vertex) == graph[path[position - 1]].end())
		return false;

	for (int i = 0; i < position; i++)
		if (path[i] == vertex)
			return false;

	return true;
}

bool find_hamiltonian_cycle(std::vector<int>& path, int position) {
	if (position == graph.size())
	{
		if (graph[path[position - 1]][path[0]] == 1)
			return true;
		else
			return false;
	}

	for (int v = 2; v < graph.size(); v++)
	{
		if (check_vertex(v, path, position))
		{
			path[position] = v;

			if (find_hamiltonian_cycle(path, position + 1) == true)
				return true;

			path[position] = -1;
		}
	}

	return false;
}

void hamiltonian_cycle_sequential() {
	std::vector<int> path = std::vector<int>(graph.size(), -1);

	path[0] = 0;
	path[1] = 1;
	
	if (find_hamiltonian_cycle(path, 2) == false)
	{
		std::cout << "Solution does not exist";
		return;
	}

	for (int i = 1; i < path.size(); i++) {
		std::cout << path[i] << " ";
	}
}

bool find_ham_cycle_parallel(int position) {
	if (position == graph.size())
	{
		if (graph[path_parallel[position - 1]][path_parallel[0]] == 1)
			return true;
		else
			return false;
	}

	for (int v = 2; v < graph.size(); v++)
	{
		if (check_vertex(v, path_parallel, position))
		{
			path_mutex.lock();
			path_parallel[position] = v;
			path_mutex.unlock();

			if (find_hamiltonian_cycle(path_parallel, position + 1) == true)
				return true;

			path_mutex.lock();
			path_parallel[position] = -1;
			path_mutex.unlock();
		}
	}

	return false;
}

void hamiltonian_cycle_parallel(int no_of_threads) {
	std::thread th[100];

	path_parallel = std::vector<int>(graph.size() + 1, -1);
	path_parallel[0] = 0;
	path_parallel[1] = 1;

	for (int i = 0; i < no_of_threads; i++) {
		th[i] = std::thread(find_ham_cycle_parallel, i + 1);
	}

	for (int i = 0; i < no_of_threads; i++) {
		th[i].join();
	}
	if (path_parallel[2] == -1) {
		std::cout << "Solution does not exist";
	}
	else {
		for (int i = 1; i < path_parallel.size(); i++) {
			std::cout << path_parallel[i] << " ";
		}
	}
}