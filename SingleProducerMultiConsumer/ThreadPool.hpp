/*************************************************************************/
/** Contains the header for the Thread Pool class from parallelism.

Copyright (C) 2017-2018 Zachariah The Magnificent.
<zachariahthemagnificent@gmail.com>.
**************************************************************************/
#pragma once
#include <functional>
#include <thread>
#include <atomic>
#if defined DEBUG_THREADS
#include <mutex>
#endif

namespace zachariahs_world
{
	namespace parallelism
	{

#if defined DEBUG_THREADS
		std::mutex cout_mutex;
#endif

		class ThreadPool
		{
		public:
			using Task = std::function<void ( const std::size_t thread_index )>;

			ThreadPool ( )
			{
				threads.reserve ( num_threads - 1 );

				for ( auto i = static_cast< std::size_t > ( 1 ); i < num_threads; ++i )
				{
					threads.push_back ( std::thread { [ this, i ]
					{
						while ( !exit.load ( std::memory_order_relaxed ) )
						{
							if ( running_threads_left.load ( std::memory_order_acquire ) > 0 )
							{
								run_task ( i );
							}
						}

#if defined DEBUG_THREADS
						std::lock_guard<std::mutex> d { cout_mutex };
						std::cout << "Thread " << i << ": Destorying myself lol!\n";
#endif
					} } );
				}
			}
			~ThreadPool ( )
			{
				exit.store ( true, std::memory_order_relaxed );

				for ( auto it = threads.begin ( ), end = threads.end ( ); it != end; ++it )
				{
					it->join ( );
				}

#if defined DEBUG_THREADS
				std::cout << "Thread 0: Destroying myself lol!\n";
				system ( "pause" );
#endif
			}

			void run ( const Task& task )
			{
				constexpr auto thread_index = 0;
				this->task = &task;

				// Make sure that all threads are avaliable before running the task.
				while ( available_threads.load ( std::memory_order_relaxed ) != num_threads )
				{
				}
				available_threads.store ( 0, std::memory_order_relaxed );
				running_threads_left.store ( num_threads, std::memory_order_release );

				run_task ( thread_index );

#if defined DEBUG_THREADS
				{
					std::lock_guard<std::mutex> d { cout_mutex };
					std::cout << "Thread " << thread_index << ": Returning to main thread!\n";
				}
#endif
			}

			std::size_t size ( ) const
			{
				return num_threads;
			}

		private:
			void run_task ( const std::size_t thread_index )
			{
#if defined DEBUG_THREADS
				{
					std::lock_guard<std::mutex> d { cout_mutex };
					std::cout << "Thread " << thread_index << ": Running task!\n";
				}
#endif
				( *task ) ( thread_index );
#if defined DEBUG_THREADS
				{
					std::lock_guard<std::mutex> d { cout_mutex };
					std::cout << "Thread " << thread_index << ": Task has finished!\n";
				}
#endif

				// If we are the last thread to finish.
				if ( running_threads_left.load ( std::memory_order_acquire ) == 1 )
				{
#if defined DEBUG_THREADS
					{
						std::lock_guard<std::mutex> d { cout_mutex };
						std::cout << "Thread " << thread_index << ": All tasks done! Joining!\n";
					}
#endif
					join ( );
					running_threads_left.store ( 0, std::memory_order_release );
				}
				else
				{
					running_threads_left.fetch_sub ( 1, std::memory_order_release );
					while ( running_threads_left.load ( std::memory_order_acquire ) != 0 )
					{
					}
#if defined DEBUG_THREADS
					{
						std::lock_guard<std::mutex> d { cout_mutex };
						std::cout << "Thread " << thread_index << ": All tasks done! Exiting!\n";
					}
#endif
				}

				available_threads.fetch_add ( 1, std::memory_order_relaxed );
			}
			void join ( )
			{
			}

			const std::size_t num_threads = std::thread::hardware_concurrency ( );

			std::vector<std::thread> threads;

			std::atomic_bool exit { false };
			std::atomic_size_t running_threads_left { 0 };
			std::atomic_size_t available_threads { num_threads };

			const Task* task;
		};
	}
}
