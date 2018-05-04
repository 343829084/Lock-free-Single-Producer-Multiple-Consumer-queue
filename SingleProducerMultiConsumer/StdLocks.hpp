/*************************************************************************/
/** Contains the definition of classes representing locks for the
standard's globals.

Copyright (C) 2017-2018 Zachariah The Magnificent.
<zachariahthemagnificent@gmail.com>.
**************************************************************************/
#pragma once
#include <mutex>

namespace zachariahs_world
{
	namespace parallelism
	{
		class OStreamLock
		{
		public:
			OStreamLock ( ) = default;

		private:
			static inline std::mutex cout_mutex;

			std::lock_guard<std::mutex> lock { cout_mutex };
		};
	}
}
