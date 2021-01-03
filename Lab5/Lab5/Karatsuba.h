#pragma once
#include <vector>
#include <memory>

std::vector<int> karatsuba_sequential(const std::vector<int>& A, const std::vector<int>& B) {
	std::vector<int> product = std::vector<int>(2 * B.size(), 0);

	if (B.size() == 1) {
		product[0] = A[0] * B[0];
		return product;
	}

	int half_size = A.size() / 2;
	int odd = half_size;
	if (A.size() % 2 == 1) {
		odd++;
	}

	auto a_low = std::vector<int>(odd, 0);
	auto a_high = std::vector<int>(odd, 0);
	auto b_low = std::vector<int>(odd, 0);
	auto b_high = std::vector<int>(odd, 0);

	auto a_low_high = std::vector<int>(odd, 0);
	auto b_low_high = std::vector<int>(odd, 0);

	for (int i = 0; i < half_size; ++i) {
		a_low[i] = A[i];
		a_high[i] = A[half_size + i];
		a_low_high[i] = a_high[i] + a_low[i];

		b_low[i] = B[i];
		b_high[i] = B[half_size + i];
		b_low_high[i] = b_high[i] + b_low[i];
	}

	auto product_low = karatsuba_sequential(a_low, b_low);
	auto product_high = karatsuba_sequential(a_high, b_high);
	auto product_low_high = karatsuba_sequential(a_low_high, b_low_high);

	auto product_middle = std::vector<int>(A.size(), 0);
	for (int i = 0; i < A.size(); i++) {
		product_middle[i] = product_low_high[i] - product_low[i] - product_high[i];
	}

	int middle = A.size() / 2;
	for (int i = 0; i < A.size(); ++i) {
		product[i] += product_low[i];
		product[i + A.size()] += product_high[i];
		product[i + middle] += product_middle[i];
	}

	return product;
}

void compute_difference(const std::shared_ptr<std::vector<int>>& a, const std::shared_ptr<std::vector<int>>& B) {
	int carry = 0;
	for (int i = 0; i < a->size(); i++) {
		if (i >= B->size()) {
			if (carry == 0) {
				return;
			}
			else {
				if ((*a)[i] == 0) {
					carry = -1;
					continue;
				}
				else {
					(*a)[i] = (*a)[i] + carry;
					break;
				}
			}
		}
		else {
			(*a)[i] = (*a)[i] + carry;
			carry = 0;
			if ((*a)[i] >= (*B)[i]) {
				(*a)[i] = (*a)[i] - (*B)[i];
			}
			else {
				(*a)[i] = (*a)[i] - (*B)[i] + 10;
				carry = -1;
			}
		}
	}
}

std::shared_ptr<std::vector<int>> compute_sum(
	const std::shared_ptr<std::vector<int>>& arr,
	int start, int stop
) {

	int middle = (stop + start) / 2;
	std::shared_ptr<std::vector<int>> result = std::make_shared<std::vector<int>>(
		std::max(middle - start, stop - middle), 0);
	for (int i = start; i < middle; i++)
	{
		(*result)[i - start] = (*arr)[i];
	}
	for (int i = middle; i < stop; i++)
	{
		(*result)[i - middle] += (*arr)[i];
	}
	return result;
}

void compute_product_sequential(
	const std::shared_ptr<std::vector<int>>& a, int start_a, int stop_a,
	const std::shared_ptr<std::vector<int>>& b, int start_b, int stop_b,
	std::shared_ptr<std::vector<int>>& result)
{

	std::vector<int> res = std::vector<int>((stop_a - start_a) * 2, 0);

	for (int i = start_b; i < stop_b; i++)
	{
		int carry = 0;
		for (int j = start_a; j < stop_a; j++)
		{
			int prod = (*a)[j] * (*b)[i] + carry;

			res[i + j - start_a - start_b] += prod;
		}
	}

	result = std::make_shared<std::vector<int>>(res);
}

void multiply_karatsuba(
	const std::unique_ptr<ThreadPool>& thread_pool,
	std::shared_ptr<std::vector<int>> a, int start_a, int stop_a,
	std::shared_ptr<std::vector<int>> b, int start_b, int stop_b,
	std::shared_ptr<std::vector<int>>& result
) {
	if (stop_a - start_a < 5 || stop_b - stop_a < 5 || thread_pool->available_threads() == 0) {
		compute_product_sequential(a, start_a, stop_a, b, start_b, stop_b, result);
		return;
	}

	int middle_a = (stop_a + start_a) / 2;
	int middle_b = (stop_b + start_b) / 2;

	// 1. Add A0 * B0, with 2 * (n/2) offset
	std::shared_ptr<std::atomic<bool>> flag1;
	auto res1 = std::make_shared<std::vector<int>>((stop_a - middle_a) * 2, 0);
	if (thread_pool->available_threads() > 0) {
		flag1 = thread_pool->enqueue([&a, middle_a, stop_a, &b, middle_b, stop_b, &res1, &thread_pool]() {
			multiply_karatsuba(
				thread_pool,
				a, middle_a, stop_a,
				b, middle_b, stop_b,
				res1);
			});
	}
	else {
		multiply_karatsuba(
			thread_pool,
			a, middle_a, stop_a,
			b, middle_b, stop_b,
			res1);
	}

	// 3. Add A1 * B1, with no offset
	std::shared_ptr<std::atomic<bool>> flag3;
	auto res3 = std::make_shared<std::vector<int>>((middle_a - start_a) * 2, 0);
	if (thread_pool->available_threads() > 0) {
		flag3 = thread_pool->enqueue([&thread_pool, &a, start_a, middle_a, &b, start_b, middle_b, &res3]() {
			multiply_karatsuba(
				thread_pool,
				a, start_a, middle_a,
				b, start_b, middle_b,
				res3
			);
			});
	}
	else {
		multiply_karatsuba(
			thread_pool,
			a, start_a, middle_a,
			b, start_b, middle_b,
			res3
		);
	}

	// Step 2:
	//A1 + A0
	std::shared_ptr<std::vector<int>> sum_a1_0 = compute_sum(a, start_a, stop_a);
	//B1 + B0
	std::shared_ptr<std::vector<int>> sum_b1_0 = compute_sum(b, start_b, stop_b);
	//(A1 + A0) * (B1 + B0)
	auto prod2 = std::make_shared<std::vector<int>>(2 * std::max(sum_a1_0->size(), sum_b1_0->size()), 0);

	std::shared_ptr<std::atomic<bool>> flag2 = nullptr;
	if (thread_pool->available_threads() > 0) {
		flag2 = thread_pool->enqueue([&thread_pool, &sum_a1_0, sum_b1_0, &prod2]() {
			multiply_karatsuba(
				thread_pool,
				sum_a1_0, 0, sum_a1_0->size(),
				sum_b1_0, 0, sum_b1_0->size(),
				prod2
			);
			});
	}
	else {
		multiply_karatsuba(
			thread_pool,
			sum_a1_0, 0, sum_a1_0->size(),
			sum_b1_0, 0, sum_b1_0->size(),
			prod2
		);
	}
	// result = productStepTwo - (A0 * B0 + A1 * B1)
	// Wait for step one and step three to finish
	bool check1 = false;
	bool check3 = false;
	while (!check1 && !check3) {
		check1 = (!flag1) || (flag1->load());
		check3 = (!flag3) || (flag3->load());
	}

	compute_difference(prod2, res1);
	compute_difference(prod2, res3);

	// Sum the individual results of each step into the result array
	auto res = std::make_shared<std::vector<int>>((stop_a - start_a) * 2, 0);

	// Sum step1
	for (int i = middle_a * 2; i - middle_a * 2 < res1->size(); i++) {
		(*res)[i] += (*res1)[i - middle_a * 2];
	}

	// Sum step2
	for (int i = middle_a; i - middle_a < prod2->size(); i++) {
		(*res)[i] += (*prod2)[i - middle_a];
	}

	// Sum step3
	for (int i = 0; i < res3->size(); i++) {
		(*res)[i] += (*res3)[i];
	}

	*result = *res;
}

