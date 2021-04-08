#pragma once

/*
The purpose of this class is to achieve similar functionality to a vector
of indexes, but without the overhead of dereferences and memory management.
This can be achieved by just specifying an increment amount and an offset.
*/

#include <ranges>

template<std::integral T = unsigned>
class index_generator
{
	T offset, inc_amount, length;
	
	public:
	
	[[nodiscard]] constexpr index_generator(T off, T inc, T len) noexcept
		: offset(off), inc_amount(inc), length(len) {}
	
	class iterator
	{
		T inc_amount, value;
		
		[[nodiscard]] constexpr iterator(T inc, T init) noexcept
			: inc_amount(inc), value(init) {}
		
		public:
		
		constexpr iterator() = default;
		
		constexpr iterator& operator++() noexcept { value += inc_amount; return *this; }
		
		[[nodiscard]] constexpr iterator operator++(int) noexcept
		{
			iterator copy(*this);
			++*this;
			return copy;
		}
		
		constexpr iterator& operator--() noexcept { value -= inc_amount; return *this; }
		
		[[nodiscard]] constexpr iterator operator--(int) noexcept
		{
			iterator copy(*this);
			--*this;
			return copy;
		}
		
		[[nodiscard]] constexpr iterator operator+(std::integral auto shift_amount) noexcept
			{ return iterator(*this) += shift_amount; }
		
		constexpr iterator& operator+=(std::integral auto shift_amount) noexcept
			{ value += (shift_amount * inc_amount); return *this; }
		
		[[nodiscard]] constexpr iterator operator-(std::integral auto shift_amount) noexcept
			{ return iterator(*this) -= shift_amount; }
		
		constexpr iterator& operator-=(std::integral auto shift_amount) noexcept
			{ value -= (shift_amount * inc_amount); return *this; }
		
		[[nodiscard]] constexpr bool operator==(const iterator& other) noexcept
		{
			return value == other.value;
		}
		
		[[nodiscard]] constexpr std::strong_ordering operator<=>(const iterator& other) noexcept
		{
			return value <=> other.value;
		}
		
		[[nodiscard]] constexpr T operator*() const noexcept { return value; }
		
		friend class index_generator;
	};
	
	[[nodiscard]] constexpr iterator begin() const noexcept
		{ return iterator(inc_amount, offset); }
	
	[[nodiscard]] constexpr iterator end() const noexcept
		{ return iterator(inc_amount, offset + inc_amount * length); }
		
	[[nodiscard]] constexpr T operator[](std::integral auto i) const noexcept
		{ return offset + inc_amount * i; }
	
	[[nodiscard]] constexpr T size() const noexcept { return length; }
};
