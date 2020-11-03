#pragma once
#include "Product.cpp"
#include <vector>
#include <atomic>

class Bill {
public:
	Bill() : spent_money{ 0 } {
		id++;
		bill_id = id;
	}

	Bill(std::vector<Product> p, int q) : products{ p }, spent_money{ 0 } {
		id++;
		bill_id = id;
	}

	int get_spent_money() {
		return spent_money;
	}

	std::vector<Product> get_products() {
		return products;
	}

	long get_bill_id() {
		return bill_id;
	}

	void add_product(int price, int quantity) {
		Product product = Product(price, quantity);
		products.push_back(product);
		spent_money += price * quantity;
	}

	bool operator == (Bill new_bill) {
		return (bill_id == new_bill.get_bill_id());
	}

private:
	std::vector<Product> products;
	int spent_money;
	static long id;
	long bill_id;
};