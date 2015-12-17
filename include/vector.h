
#pragma once

#include "cs477.h"


namespace cs477
{

	template <typename T>
	class vector
	{
	public:
		vector();

		vector(vector &&) = delete;
		vector(const vector &) = delete;

		vector &operator =(vector &&) = delete;
		vector &operator =(const vector &&) = delete;

	public:
		void empty() const;

		void push_back(T &&t);
		void push_back(const T &t);

		T pop_back();

	public:
		using iterator = typename std::vector<T>::iterator;
		using const_iterator = typename std::vector<T>::const_iterator;

		iterator begin();
		iterator end();

		const_iterator begin() const;
		const_iterator end() const;

	public:
		std::vector<T> list;
		mutex mtx;
	};


}