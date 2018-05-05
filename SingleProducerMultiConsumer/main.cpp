/*************************************************************************/
/** Contains the entrypoint of the program.

Copyright (C) 2017-2018 Zachariah The Magnificent.
<zachariahthemagnificent@gmail.com>.
**************************************************************************/
#define DEBUG_THREADS
#define MULTI_THREADING
#include <iostream>
#include <string>
#include <array>
#include "StdLocks.hpp"
#include "Profiler.hpp"
#include "TaskBasedModel.hpp"
#include "ForkJoinModel.hpp"

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
	using zachariahs_world::parallelism::ForkJoinModel;
	using zachariahs_world::parallelism::OStreamLock;
	using namespace std::chrono_literals;

	try
	{
#if defined MULTI_THREADING
		ForkJoinModel parallelism_model;
#endif

#if defined DEBUG_THREADS
		constexpr auto num_tests = std::size_t { 10 };
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
			const auto num_objects_per_frame = game_objects.size ( ) / parallelism_model.num_threads;
			const auto game_objects_begin = game_objects.begin ( );

			parallelism_model.run ( [ game_objects_begin, num_objects_per_frame, delta_time ] ( const std::size_t thread_index )
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
			OStreamLock lock;
			std::cout << "Average: " << profile.mean << "ns\n";
			std::cout << "Highest: " << profile.highest << "ns\n";
			std::cout << "Lowest: " << profile.lowest << "ns\n";
			std::cout << "Median: " << profile.median << "ns\n";
			std::cout << "Standard deviation: " << profile.standard_deviation << "ns\n";
			std::system ( "pause" );
		}
	}
	catch ( const std::exception& exception )
	{
		std::cout << exception.what ( ) << '\n';
	}

	system ( "pause" );
	return EXIT_SUCCESS;
}
