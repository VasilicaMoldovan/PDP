#pragma once
#include <vector>
#include <string>

class Polynomial
{
public:
	Polynomial(int new_size) {
		degree = new_size;
		new_size++;
		size = new_size;
		coeff = std::vector<int>(new_size, 0);
	}

	Polynomial() {
		degree = 0;
		coeff = std::vector<int>(size, 0);
	}

	int get_coeff(int index) {
		return coeff[index];
	}

	std::vector<int> get_all_coeff() {
		return coeff;
	}

	void set_coeff(int index, int value) {
		coeff[index] = value;
	}

	void set_all_coeff(std::vector<int> new_coeff) {
		coeff = new_coeff;
	}

	int get_degree() {
		return degree;
	}

	int get_size() {
		return size;
	}

	void generate_polynomial() {
		for (int i = 0; i < size; i++) {
			coeff[i] = rand() % 20 - 10;
			if (i == size - 1) {
				while (coeff[i] == 0) {
					coeff[i] = rand() % 20 - 10;
				}
			}
		}
	}
	Polynomial get_first(int m) {
		Polynomial result = Polynomial(m - 1);

		int k = 0;

		for (int i = size - m; i < size; i++) {
			result.coeff[k] = coeff[i];
			k++;
		}

		return result;
	}

	Polynomial get_last(int m) {
		Polynomial result = Polynomial(m - 1);

		for (int i = 0; i < m; i++) {
			result.coeff[i] = coeff[i];
		}

		return result;
	}

	Polynomial sum(Polynomial b)
	{
		int size1 = size;
		int size2 = b.size;

		int sizeMax = (size1 > size2) ? size1 : size2;

		Polynomial result = Polynomial(sizeMax - 1);

		for (int i = 0; i < sizeMax; i++) {
			int res = 0;
			if (i < size1) {
				res = res + coeff[i];
			}
			if (i < size2) {
				res = res + b.get_coeff(i);
			}
			result.set_coeff(i, res);
		}

		return result;
	}

	Polynomial add_zeros_left(int v)
	{
		std::vector<int> newCoef = std::vector<int>(size + v, 0);

		for (int i = 0; i < size; i++) {
			newCoef[i] = coeff[i];
		}
		
		coeff = newCoef;
		size = coeff.size();

		return *this;
	}

	Polynomial add_zeros_right(int v)
	{
		Polynomial result = Polynomial(size + v - 1);

		for (int i = v; i < size + v; i++) {
			result.coeff[i] = coeff[i - v];
		}

		return result;
	}

	Polynomial difference(Polynomial b)
	{
		int size1 = size;
		int size2 = b.size;

		int sizeMax = (size1 > size2) ? size1 : size2;

		Polynomial result = Polynomial(sizeMax - 1);

		for (int i = 0; i < sizeMax; i++)
		{
			int res = 0;
			if (i < size1)
			{
				res = coeff[i];
			}
			if (i < size2)
			{
				res = res - b.get_coeff(i);
			}
			result.set_coeff(i, res);
		}

		return result;
	}

	std::string to_string()
	{
		std::string str = "";

		for (int i = size - 1; i >= 0; i--) {
			if (coeff[i] != 0) {
				if (coeff[i] < 0) {
					str += std::to_string(coeff[i]);
				}
				else if (coeff[i] > 0) {
					if (i < size - 1) {
						str += "+";
					}
					str += std::to_string(coeff[i]);
				}

				if (i == 1) {
					str += "*";
					str += "X";
				}
				else if (i != 0) {
					str += "*"; 
					str += "X";
					str += std::to_string(i);
				}

			}
		}

		return str;
	}

private:
	int degree;
	std::vector<int> coeff;
	int size;
};
