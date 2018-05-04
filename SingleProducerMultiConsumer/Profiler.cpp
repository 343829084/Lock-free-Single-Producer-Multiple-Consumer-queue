/*************************************************************************/
/** Contains the implementation of the Profiler class from debugging.

Copyright (C) 2017-2018 Zachariah The Magnificent.
<zachariahthemagnificent@gmail.com>.
**************************************************************************/
#include "Profiler.hpp"

namespace zachariahs_world
{
	namespace debugging
	{
		Profiler* Profiler::global = nullptr;

		Profiler::Profiler ( const size_t size )
		{
			data_points_.reserve ( size );
		}

		Profiler& Profiler::current ( ) noexcept
		{
			return *global;
		}

		void Profiler::make_current ( ) noexcept
		{
			global = this;
		}

		void Profiler::start ( )
		{
			last_time_point_ = clock::now ( );
		}

		void Profiler::end ( )
		{
			const auto curr_time_point = clock::now ( );
			const auto duration = curr_time_point - last_time_point_;
			const auto num_representation = duration.count ( );
			data_points_.push_back ( num_representation );
			last_time_point_ = curr_time_point;
		}

		Profiler::Profile Profiler::flush ( )
		{
			std::sort ( data_points_.begin ( ), data_points_.end ( ) );

			const auto begin = data_points_.cbegin ( );
			const auto end = data_points_.cend ( );
			const auto size = data_points_.size ( );

			const auto lowest = data_points_.front ( );
			const auto highest = data_points_.back ( );
			const auto median = data_points_ [ size / 2 ];
			const auto sum = std::accumulate ( begin, end, static_cast< rep >( 0 ) );
			const auto mean = sum / size;
			const auto standard_deviation = std::sqrt ( std::accumulate ( begin, end, static_cast< rep >( 0 ), [ mean ] ( const auto& total, const auto& point )
			{
				const auto difference = point - mean;
				return total + difference * difference;
			} ) / static_cast < long double > ( size ) );

			data_points_.clear ( );

			return Profile { static_cast< rep >( lowest ), static_cast< rep >( highest ), static_cast< rep >( median ), static_cast< rep >( mean ), static_cast< rep >( standard_deviation ) };
		}

		Profiler::Profile::Profile ( const rep& lowest, const rep& highest, const rep& median, const rep& mean, const rep& standard_deviation ) noexcept
			:
		lowest { lowest },
		highest { highest },
		median { median },
		mean { mean },
		standard_deviation { standard_deviation }
		{

		}
	}
}
