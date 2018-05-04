/*************************************************************************/
/** Contains the entrypoint of the program.

Copyright (C) 2017-2018 Zachariah The Magnificent.
<zachariahthemagnificent@gmail.com>.
**************************************************************************/
//#define DEBUG_THREADS
//#define MULTI_THREADING
#include <iostream>
#include <string>
#include <array>
#include "Profiler.hpp"
#include "SPMPQueue.hpp"
#include "ThreadPool.hpp"

struct Vector : std::array<float, 3>
{
	static constexpr auto num_dimensions = 3;

	Vector ( ) = default;
	constexpr Vector ( const std::array<float, 3> arr ) noexcept
		:
	array ( arr )
	{

	}

	constexpr Vector operator+ ( const Vector vec ) const noexcept
	{
		auto new_vector = Vector { };

		for ( auto i = size_t { }; i < num_dimensions; ++i )
		{
			new_vector [ i ] = ( *this ) [ i ] + vec [ i ];
		}

		return new_vector;
	}
	constexpr Vector operator* ( const float multiplier ) const noexcept
	{
		auto new_vector = Vector { };

		for ( auto i = size_t { }; i < num_dimensions; ++i )
		{
			new_vector [ i ] = ( *this ) [ i ] * multiplier;
		}

		return new_vector;
	}

	Vector& operator+= ( const Vector vec ) noexcept
	{
		for ( auto i = size_t { }; i < num_dimensions; ++i )
		{
			( *this ) [ i ] += vec [ i ];
		}

		return *this;
	}
};

class GameObject
{
public:
	GameObject ( ) = default;

	constexpr GameObject ( const Vector position, const Vector velocity ) noexcept
		:
	position ( position ),
		velocity ( velocity )
	{

	}

	void update ( const float delta_time ) noexcept
	{
		position += velocity * delta_time;
	}

private:
	Vector position;
	Vector velocity;
};

void UpdateGameObjects ( const std::vector<GameObject>::iterator game_objects, const std::size_t size, const float delta_time ) noexcept
{
	for ( auto i = std::size_t { }; i < size; ++i )
	{
		game_objects [ i ].update ( delta_time );
	}
}

int main ( )
{
	using zachariahs_world::debugging::Profiler;
	using zachariahs_world::parallelism::ThreadPool;
	using zachariahs_world::parallelism::Queue;

	//constexpr auto size = sizeof ( zachariahs_world::parallelism::Queue );
	//constexpr auto alignment = alignof ( zachariahs_world::parallelism::Queue );
	//constexpr auto num_cache_lines = size / zachariahs_world::cache_line_size;

	//Queue queue;

	//// Test if it works for single threaded programs first.

	//queue.push ( 3 );
	//queue.push ( 2 );
	//queue.push ( 1 );
	//auto [ result1, value1 ] = queue.pop ( );
	//auto [ result2, value2 ] = queue.pop ( );
	//auto [ result3, value3 ] = queue.pop ( );

	//std::cout << value1 << '\n';
	//std::cout << value2 << '\n';
	//std::cout << value3 << '\n';
	
	ThreadPool thread_pool;

#if defined DEBUG_THREADS
	constexpr auto num_tests = std::size_t { 2 };
#else
	constexpr auto num_tests = std::size_t { 10000 };
#endif
	constexpr auto num_objects = std::size_t { 10000 };
	constexpr auto delta_time = 0.001f;
	Profiler profiler;

	std::vector<GameObject> game_objects { num_objects };

	for ( auto i = std::size_t { }; i < num_tests; ++i )
	{
		profiler.start ( );
#if defined MULTI_THREADING
		const auto num_objects_per_frame = game_objects.size ( ) / thread_pool.size ( );
		const auto game_objects_begin = game_objects.begin ( );

		thread_pool.run ( [ game_objects_begin, num_objects_per_frame, delta_time ] ( const std::size_t thread_index )
		{
			const auto start_index = num_objects_per_frame * thread_index;
			UpdateGameObjects ( game_objects_begin + start_index, num_objects_per_frame, delta_time );
		} );
#else
		UpdateGameObjects ( game_objects.begin ( ), game_objects.size ( ), delta_time );
#endif
		profiler.end ( );
	}

	auto profile = profiler.flush ( );

	{
#if defined DEBUG_THREADS
		std::lock_guard<std::mutex> cout_guard { zachariahs_world::parallelism::cout_mutex };
#endif
		std::cout << "Average: " << profile.mean << "ns\n";
		std::cout << "Highest: " << profile.highest << "ns\n";
		std::cout << "Lowest: " << profile.lowest << "ns\n";
		std::cout << "Median: " << profile.median << "ns\n";
		std::cout << "Standard deviation: " << profile.standard_deviation << "ns\n";
		std::system ( "pause" );
	}
	return EXIT_SUCCESS;
}
