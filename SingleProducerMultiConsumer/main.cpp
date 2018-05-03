#include <iostream>
#include <string>
#include "Profiler.hpp"
#include "SPMPQueue.hpp"

int main ( )
{
	constexpr auto size = sizeof ( zachariahs_world::parallelism::Queue );
	constexpr auto alignment = alignof ( zachariahs_world::parallelism::Queue );
	constexpr auto num_cache_lines = size / zachariahs_world::cache_line_size;

	using zachariahs_world::parallelism::Queue;
	Queue queue;

	// Test if it works for single threaded programs first.

	queue.push ( 3 );
	queue.push ( 2 );
	queue.push ( 1 );
	auto [ result1, value1 ] = queue.pop ( );
	auto [ result2, value2 ] = queue.pop ( );
	auto [ result3, value3 ] = queue.pop ( );

	std::cout << value1 << '\n';
	std::cout << value2 << '\n';
	std::cout << value3 << '\n';

	system ( "pause" );
	return EXIT_SUCCESS;
}
