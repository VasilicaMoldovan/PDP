#pragma once
#include <string>

class Product
{
public:
	Product() : price{ 0 }, quantity{ 0 } {

	}

	Product(int new_price, int quantity) : 
		price{ new_price }, quantity{quantity} {
	}

	int get_price() {
		return price;
	}

	int get_quantity() {
		return quantity;
	}

	void set_quantity(int new_quantity) {
		quantity = new_quantity;
	}

	void decrease_quantity(int sold_quantity) {
		quantity -= sold_quantity;
	}

	void set_price(int new_price) {
		price = new_price;
	}

private:
	int price;
	int quantity;
};
