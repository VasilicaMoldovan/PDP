//#include <iostream>
//#include <mutex>
//#include <condition_variable>
//#include <vector>
//
//void populate_vectors();
//void compute_product();
//void compute_sum();
//
//std::vector<int> vector1;
//std::vector<int> vector2;
//int current_product = 0;
//int scalar_product = 0;
//bool processed = false;
//bool ready = false;
//std::mutex m;
//std::condition_variable cv;
//
//int main() {
//	populate_vectors();
//
//	int n = vector1.size();
//	for (int i = 0; i < n; i++) {
//		std::cout << vector1[i] << " " << vector2[i] << "\n";
//	}
//
//	std::thread th1 = std::thread(compute_product);
//	std::thread th2 = std::thread(compute_sum);
//
//	ready = true;
//
//	th1.join();
//	th2.join();
//
//	std::cout << "Scalar product is " << scalar_product;
//
//	return 0;
//}
//
//void populate_vectors() {
//	int number_of_pairs = rand() % 10 + 2;
//	int x, y;
//
//	for (int i = 0; i < number_of_pairs; i++) {
//		x = rand() % 10;
//		y = rand() % 10;
//
//		vector1.push_back(x);
//		vector2.push_back(y);
//	}
//}
//
//void compute_product() {
//	for (int i = 0; i < vector1.size(); i++) {
//		std::unique_lock<std::mutex> lk(m);
//		cv.wait(lk, [] {return ready; });
//		
//		current_product = vector1[i] * vector2[i];
//		processed = true;
//		ready = false;
//
//		lk.unlock();
//		cv.notify_one();
//	}
//}
//
//void compute_sum() {
//	for (auto elem : vector2) {
//		std::unique_lock<std::mutex> lk(m);
//		cv.wait(lk, [] {return processed; });
//
//		scalar_product += current_product;
//		ready = true;
//		processed = false;
//
//		lk.unlock();
//		cv.notify_one();
//	}
//}