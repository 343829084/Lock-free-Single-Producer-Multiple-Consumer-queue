/*************************************************************************/
/** Contains the definition of the ForkJoinModel class from parallelism.

Copyright (C) 2017-2018 Zachariah The Magnificent.
<zachariahthemagnificent@gmail.com>.
**************************************************************************/
#pragma once
#include <functional>
#include <thread>
#include <atomic>
#if defined DEBUG_THREADS
#include "StdLocks.hpp"
#endif

namespace zachariahs_world
{
	namespace parallelism
	{
		class ForkJoinModel
		{
		public:
			using Algorithm = std::function<void ( const std::size_t thread_index )>;

			inline static const std::size_t num_threads = std::thread::hardware_concurrency ( );

			ForkJoinModel ( )
			{
				// Number of threads minus the main thread.
				threads.reserve ( num_threads - 1 );

				for ( auto i = static_cast< std::size_t > ( 1 ); i < num_threads; ++i )
				{
					threads.push_back ( std::thread { [ this, i ]
					{
						// No need for synchronisation. We synchronise at join() instead.
						while ( !exit.load ( std::memory_order_relaxed ) )
						{
							// If there are tasks to be done.
							if ( tasks_left.load ( std::memory_order_acquire ) > 0 )
							{
								run_task ( i );
							}
						}

#if defined DEBUG_THREADS
						OStreamLock lock;
						std::cout << "Thread " << i << ": Destorying myself lol!\n";
#endif
					} } );
				}
			}
			~ForkJoinModel ( )
			{
#if defined DEBUG_THREADS
				std::cout << "Thread 0: Destroying myself lol!\n";
#endif

				// No need for synchronisation. join() already synchronises for us.
				exit.store ( true, std::memory_order_relaxed );

				for ( auto it = threads.begin ( ), end = threads.end ( ); it != end; ++it )
				{
					it->join ( );
				}
			}

			void run ( const Algorithm& algorithm )
			{
				constexpr auto thread_index = 0;

				// Okay to change the algorithm here since all threads should be done with the last algorithm after the previous run() returns.
				this->algorithm = &algorithm;

				// Make sure that all threads are available before creating more tasks.
				// No need for synchronisation since we have synchronised through tasks_left and we are going to synchronise through it again later.
				while ( available_threads.load ( std::memory_order_relaxed ) != num_threads )
				{
				}
				// Reset.
				available_threads.store ( 0, std::memory_order_relaxed );
				// Create tasks for the threads to run.
				tasks_left.store ( num_threads, std::memory_order_release );

				// Run a task ourselves and then return only if all threads have finished with their tasks.
				run_task ( thread_index );

#if defined DEBUG_THREADS
				{
					OStreamLock lock;
					std::cout << "Thread " << thread_index << ": Returning to main thread!\n";
				}
#endif
			}

		private:
			void run_task ( const std::size_t thread_index )
			{
#if defined DEBUG_THREADS
				{
					OStreamLock lock;
					std::cout << "Thread " << thread_index << ": Running task!\n";
				}
#endif
				( *algorithm ) ( thread_index );
#if defined DEBUG_THREADS
				{
					OStreamLock lock;
					std::cout << "Thread " << thread_index << ": Task has finished!\n";
				}
#endif

				// If there are still some tasks left.
				if ( tasks_left.fetch_sub ( 1, std::memory_order_acq_rel ) > 1 )
				{
					while ( tasks_left.load ( std::memory_order_acquire ) != 0 )
					{
					}
				}
#if defined DEBUG_THREADS
				{
					OStreamLock lock;
					std::cout << "Thread " << thread_index << ": All tasks done! Exiting!\n";
				}
#endif

				// Signal to the main thread that we are avaliable again.
				// No need to synchronise since we have already synchronised through tasks_left.
				available_threads.fetch_add ( 1, std::memory_order_relaxed );
			}

			std::vector<std::thread> threads;

			std::atomic_bool exit { false };
			std::atomic_size_t tasks_left { 0 };
			std::atomic_size_t available_threads { num_threads };

			const Algorithm* algorithm;
		};
	}
}
