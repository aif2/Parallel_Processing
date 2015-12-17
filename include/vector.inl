

namespace cs477
{

	template <typename T>
	vector<T>::vector()
	{
	}

	template <typename T>
	void vector<T>::empty() const
	{
		lock_guard<> lock(mtx);
		return list.empty();
	}

	template <typename T>
	void vector<T>::push_back(T &&t)
	{
		lock_guard<> lock(mtx);
		list.emplace_back(std::move(t));
	}

	template <typename T>
	void vector<T>::push_back(const T &t)
	{
		lock_guard<> lock(mtx);
		list.push_back(std::move(t));
	}

	template <typename T>
	T vector<T>::pop_back()
	{
		lock_guard<> lock(mtx);
		auto value = list.back();
		list.pop_back();
		return value;
	}

	template <typename T>
	typename vector<T>::iterator vector<T>::begin()
	{
		return list.begin();
	}

	template <typename T>
	typename vector<T>::iterator vector<T>::end()
	{
		return list.end();
	}

	template <typename T>
	typename vector<T>::const_iterator vector<T>::begin() const
	{
		return list.cbegin();
	}

	template <typename T>
	typename vector<T>::const_iterator vector<T>::end() const
	{
		return list.cend();
	}

}
