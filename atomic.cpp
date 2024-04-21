#include <atomic>
#include <thread>
#include <iostream>
#include <vector>
#include <chrono>

struct AtomicCounter
{
	std::atomic<int>	counter;

	AtomicCounter()
	{
		counter = 0;
	}

	void	increment(void)
	{
		++counter;
	}
	void	decrement(void)
	{
		counter--;
	}
};

struct Counter
{
	int	counter;

	Counter()
	{
		counter = 0;
	}

	void	increment(void)
	{
		++counter;
	}
	void	decrement(void)
	{
		counter--;
	}
};
#define countnum 1000000
int main()
{
	std::vector<std::thread>	non_atom;
	std::vector<std::thread>	atom;
	Counter						count;
	AtomicCounter				a_count;

	std::cout << "Count from 0 to " << countnum << " NON ATOMIC COUNTER in 5 threads" << std::endl;
	auto start_non_atomic = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 5; i++)
	{
		non_atom.push_back(std::thread([&count]
									   { 
			for (int j = 0; j < countnum ; j++)
				count.increment(); }));
	}
	for (int i = 0; i < 5; i++)
		non_atom[i].join();
	std::cout << "RESULT: " << count.counter << std::endl;
	auto end_non_atomic = std::chrono::high_resolution_clock::now();
	auto duration_non_atomic = std::chrono::duration_cast<std::chrono::milliseconds>(end_non_atomic - start_non_atomic);
	std::cout << "Time taken for non-atomic operations: " << duration_non_atomic.count() << " milliseconds" << std::endl;

	std::cout << "Count from 0 to 5*" << countnum << " ATOMIC COUNTER in 5 threads" << std::endl;
	auto start_atomic = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 5; i++)
	{
		atom.push_back(std::thread([&a_count]
								   {
			for (int j = 0; j < countnum ; j++)
				a_count.increment(); }));
	}
	for (int i = 0; i < 5; i++)
		atom[i].join();
	auto end_atomic = std::chrono::high_resolution_clock::now();
	auto duration_atomic = std::chrono::duration_cast<std::chrono::milliseconds>(end_atomic - start_atomic);
	std::cout << "Time taken for atomic operations: " << duration_atomic.count() << " milliseconds" << std::endl;
	std::cout << "RESULT: " << a_count.counter << std::endl;
	std::cin.get();
	return (0);
}
