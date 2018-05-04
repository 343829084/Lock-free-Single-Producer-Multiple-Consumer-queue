/*************************************************************************/
/** Contains the definition of the Profiler class from debugging.

Copyright (C) 2017-2018 Zachariah The Magnificent.
<zachariahthemagnificent@gmail.com>.
**************************************************************************/
#pragma once
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>

namespace zachariahs_world
{
	namespace debugging
	{
		class Profiler
		{
		public:
			using clock = std::chrono::steady_clock;
			using time_point = clock::time_point;
			using duration = clock::duration;
			using rep = duration::rep;

			struct Profile
			{
				Profile ( ) = default;
				explicit Profile ( const rep& lowest, const rep& highest, const rep& median, const rep& mean, const rep& standard_deviation ) noexcept;

				rep lowest { };
				rep highest { };;
				rep median { };
				rep mean { };
				rep standard_deviation { };
			};

			Profiler ( ) = default;
			explicit Profiler ( const size_t size );

			static Profiler& current ( ) noexcept;
			void make_current ( ) noexcept;

			void start ( );
			void end ( );

			Profile flush ( );

		private:
			static Profiler* global;

			std::vector<rep> data_points_;
			time_point last_time_point_;
		};
	}
}
