/*************************************************************************/
/** Contains the header for the SPMPQueue class from parallelism.

Copyright (C) 2017-2018 Zachariah The Magnificent.
<zachariahthemagnificent@gmail.com>.
**************************************************************************/
#pragma once
#include <atomic>

namespace zachariahs_world
{
	static constexpr auto cache_line_size = static_cast<std::size_t> ( 64 );

	namespace parallelism
	{
		using Type = int;
		static constexpr auto capacity = static_cast<std::size_t> ( 1024 );

		// Final class will be templated. Use aliases while testing first.
		class SPMCQueue
		{
		public:
			template<typename...Types>
			bool emplace ( Types&&...parameters ) noexcept
			{
				const auto back = back_;

				// If the queue is full.
				if ( meta_data_ [ back ].occupied.load ( std::memory_order_acquire ) )
				{
					return false;
				}

				// Place the object at the back of the queue.
				data_ [ back ] = Type { std::forward<Types> ( parameters )... };

				back_ = increment ( back );
				// Release the changes to the consumer threads.
				meta_data_ [ back ].occupied.store ( true, std::memory_order_relaxed );
				meta_data_ [ back ].available.store ( true, std::memory_order_release );

				return true;
			}

			bool push ( const Type& object ) noexcept
			{
				return emplace ( object );
			}
			bool push ( Type&& object ) noexcept
			{
				return emplace ( std::move ( object ) );
			}

			std::pair<bool, Type> pop ( ) noexcept
			{
				// Get the current front and increment it for the other consumer threads.
				const auto front = front_.fetch_add ( 1, std::memory_order_acq_rel );
				// Wrap front back into the buffer's range if it is out of range.
				const auto wrapped_front = wrap ( front );

				// If the index has gone out of bounds of our capacity and we are chosen to move it back in.
				if ( front >= capacity && wrapped_front == 0 )
				{
					// Move the index back into range. If it is still out of range, another thread with wrapped_front == 0 will also move it back.
					front_.fetch_sub ( capacity, std::memory_order_release );
				}

				// Try to take the object. Otherwise, wait for the producer thread to give us one. Should never happen.
				while ( !meta_data_ [ wrapped_front ].available.exchange ( false, std::memory_order_acq_rel ) )
				{
					// If we are being evicted.
					if ( evict_.load ( std::memory_order_relaxed ) )
					{
						return std::pair<bool, Type> { false, Type { } };
					}
				}

				auto object = std::move ( data_ [ wrapped_front ] );
				// Release the changes we made to the other threads.
				meta_data_ [ wrapped_front ].occupied.store ( false, std::memory_order_release );

				return std::pair<bool, Type> { true, std::move ( object ) };
			}

			void evict_waiting_consumers ( ) noexcept
			{
				evict_.store ( true, std::memory_order_relaxed );
			}

		private:
			struct MetaData
			{
				std::atomic_bool occupied { };
				std::atomic_bool available { };
			};

			constexpr std::size_t increment ( const std::size_t index ) const noexcept
			{
				const auto incremented_index = index + 1;

				// If we reached the end.
				if ( incremented_index == capacity )
				{
					return 0;
				}

				return incremented_index;
			}
			constexpr std::size_t wrap ( const std::size_t index ) const noexcept
			{
				return index % capacity;
			}

			alignas ( cache_line_size ) std::atomic_bool evict_ { };
			alignas ( cache_line_size ) std::size_t back_ { };
			alignas ( cache_line_size ) std::atomic_size_t front_ { };
			alignas ( cache_line_size ) MetaData meta_data_ [ capacity ];
			alignas ( cache_line_size > alignof( Type ) ? cache_line_size : alignof ( Type ) ) Type data_ [ capacity ];
		};

	}
}
