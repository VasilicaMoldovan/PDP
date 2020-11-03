#include <iostream>
#include <thread>
#include "Inventory.cpp"

#define NO_OF_THREADS 250

long Bill::id = 0;
std::mutex Inventory::product_mutex;
Inventory inventory = Inventory("products1.txt");

void sale_operation(int no_of_operations);
void test_sales();

int main(void) {

	std::cout << "Laboratory 1 - PDP\n";
	std::cout << "No of products " << inventory.get_no_of_products() << "\n";
	
	int no_of_tests = rand() % 20 + 10;
	std::cout << "There will be " << no_of_tests << " tests\n";

	for (int i = 0; i < no_of_tests; i++) {
		test_sales();
	}
	
	int n;
	std::cin >> n;

	return 0;
}

void sale_operation(int no_of_operations) {
	for (int i = 0; i < no_of_operations; i++) {
		if (inventory.empty_store()) {
			break;
		}
		else {
			long no_of_products = inventory.get_no_of_product_for_sale();
			
			int check = rand() % 2;
			if (check) {
				if (!inventory.check_corectness()) {
					std::cout << "Ups...something went wrong\n";
				}
			}

			Bill bill = Bill();
			inventory.add_bill(bill);
			
			for (int j = 0; j < no_of_products; j++) {
				inventory.sell_product(bill);
			}
		}
	}
}


void test_sales() {
	auto start = std::chrono::high_resolution_clock::now();
	std::thread th[NO_OF_THREADS];
	int no_of_threads = rand() % 200 + 30; 
	int no_of_op = 0;

	for (int i = 0; i < no_of_threads; i++) {
		no_of_op = rand() % 80;
		th[i] = std::thread(sale_operation, no_of_op);
	}

	for (int i = 0; i < no_of_threads; i++) {
		th[i].join();
	}

	if (inventory.check_corectness()) {
		std::cout << "Correct transactions\n";
	}
	else {
		std::cout << "Something went wrong\n";
	}
	auto end = std::chrono::high_resolution_clock::now();

	std::cout << "For " << no_of_threads << " threads, the execution time is " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "\n";

}