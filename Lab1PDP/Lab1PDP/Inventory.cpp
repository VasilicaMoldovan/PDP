#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include <chrono>
#include "Product.cpp"
#include "Bill.cpp"

class Inventory {
public:
	Inventory(std::string file_name) 
		: file_name{ file_name }, money{ 0 }, total_value{ 0 } {
		read_inventory_from_file();
		initial_amount_of_money = total_value;
	}

	void add_bill(Bill &new_bill) {
		product_mutex.lock();
		bills.push_back(new_bill);
		product_mutex.unlock();
	}

	long get_no_of_product_for_sale() {
		std::lock_guard<std::mutex> guard_products(product_mutex);
		if (products.size())
			return rand() % products.size();
		return 0;
	}

	void read_inventory_from_file() {
		std::ifstream fin(file_name);
		char* p;
		char* product_string = new char[16];

		while (fin >> product_string) {
			p = strtok(product_string, ",");

			Product product = Product();
			product.set_price(atoi(p));

			p = strtok(nullptr, ", \n");
			product.set_quantity(atoi(p));

			products.push_back(product);

			total_value += product.get_price() * product.get_quantity();
			no_of_products += product.get_quantity();
		}

		fin.close();
	}

	void sell_product(Bill bill) {
		int price, quantity;

		product_mutex.lock();
		if (products.size() == 0) {
			product_mutex.unlock();
			return;
		}

		int product_index = rand() % products.size();
		quantity = rand() % products[product_index].get_quantity();
		if (!quantity)
			quantity = 1;

		price = products[product_index].get_price();
		
		products[product_index].decrease_quantity(quantity);
		if (products[product_index].get_quantity() == 0)
			delete_product(product_index);
		
		add_product_to_bill(bill, price, quantity);
		product_mutex.unlock();

		total_value -= price * quantity;
		money += price * quantity;
	}

	void delete_product(int product_index) {
		products.erase(products.begin() + product_index);
	}

	bool check_corectness() {
		product_mutex.lock();
		bool return_val = false;

		if (money + total_value == initial_amount_of_money && money == get_money_from_bills())
			return_val = true;

		product_mutex.unlock();

		return return_val;
	}

	void add_product_to_bill(Bill bill, int price, int quantity) {
		for (int i = 0; i < bills.size(); i++) {
			if (bills[i] == bill)
				bills[i].add_product(price, quantity);
		}
	}

	int get_money_from_bills() {
		int sum = 0;

		for(auto b : bills) {
			sum += b.get_spent_money();
		}
		return sum;
	}

	bool empty_store() {
		product_mutex.lock();
		int size = products.size();
		product_mutex.unlock();

		if (size == 0)
			return true;
		return false;
	}

	long get_no_of_products() {
		return no_of_products;
	}

private:
	std::string file_name;
	int money;
	int total_value;
	int initial_amount_of_money;
	int no_of_products = 0;
	std::vector<Bill> bills;
	std::vector<Product> products;
	static std::mutex product_mutex;
};