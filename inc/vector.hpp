#pragma once
#include "c++lib.hpp"

namespace phobos {

template <typename T>
struct vector
{
	using value_type = T;
	using pointer = T*;
	using const_pointer = T const *;
	using reference = T&;
	using const_reference = T const &;

	using iterator = pointer;
	using const_iterator = const_pointer;

	void init();
	void fini();

	void push_back(const_reference value);
	void pop_back();

	size_t size() const;
	void reserve(size_t count);
	void clear();

	reference operator[](size_t index);
	const_reference operator[](size_t index) const;
	reference back();
	const_reference back() const;

	iterator begin();
	iterator end();
	const_iterator cbegin() const;
	const_iterator cend() const;

private:
	pointer memory;
	size_t len;
	size_t cap;

	size_t size_bytes() const;
};

template <typename T>
void vector<T>::init()
{
	memory = nullptr;
	len = 0;
	cap = 0;
}

template <typename T>
void vector<T>::fini()
{
	std::free(memory);
}

template <typename T>
void vector<T>::push_back(const_reference value)
{
	if (size() + 1 > cap) {
		cap = cap? cap*2: 4;
		memory = static_cast<pointer>(std::realloc(memory, size_bytes()));
		assert(memory);
	}
	memory[len++] = value;
}

template <typename T>
void vector<T>::pop_back()
{
	assert(size());
	--len;
}

template <typename T>
size_t vector<T>::size() const
{
	return len;
}

template <typename T>
void vector<T>::reserve(size_t count)
{
	cap += count;
	memory = static_cast<pointer>(std::realloc(memory, size_bytes()));
	assert(memory);
}

template <typename T>
void vector<T>::clear()
{
	cap = 0;
	memory = static_cast<pointer>(std::realloc(memory, size_bytes()));
	assert(memory);
}

template <typename T>
vector<T>::reference vector<T>::operator[](size_t index)
{
	assert(index < len);
	return memory[index];
}

template <typename T>
vector<T>::const_reference vector<T>::operator[](size_t index) const
{
	assert(index < len);
	return memory[index];
}

template <typename T>
vector<T>::reference vector<T>::back()
{
	return this->operator[](size()-1);
}

template <typename T>
vector<T>::const_reference vector<T>::back() const
{
	return this->operator[](size()-1);
}

template <typename T>
vector<T>::iterator vector<T>::begin()
{
	return memory;
}

template <typename T>
vector<T>::iterator vector<T>::end()
{
	return &memory[len];
}

template <typename T>
vector<T>::const_iterator vector<T>::cbegin() const
{
	return memory;
}

template <typename T>
vector<T>::const_iterator vector<T>::cend() const
{
	return &memory[len];
}

template <typename T>
size_t vector<T>::size_bytes() const
{
	return cap * sizeof(T);
}

} // phobos

