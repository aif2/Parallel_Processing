
#pragma once

#include <stdexcept>

// A simple matrix
struct matrix
{
public:
	matrix()
		: cols(0), rows(0), data(nullptr)
	{
	}

	matrix(matrix &&x)
		: matrix()
	{
		std::swap(cols, x.cols);
		std::swap(rows, x.rows);
		std::swap(data, x.data);
	}

	matrix(const matrix &x)
		: matrix()
	{
		create(x.rows, x.cols);
		memcpy(data, x.data, rows * cols * sizeof(int));
	}

	~matrix()
	{
		if (data)
		{
			delete[]data;
		}
	}

	matrix &operator =(matrix &&x)
	{
		this->~matrix();
		new(this) matrix(std::move(x));
		return *this;
	}

	matrix &operator =(const matrix &x)
	{
		this->~matrix();
		new(this) matrix(x);
		return *this;
	}

	void create(unsigned r, unsigned c)
	{
		rows = r;
		cols = c;
		data = new int[rows * cols];

		auto size = rows * cols;
		for (unsigned i = 0; i < size; i++)
		{
			data[i] = 0;
		}
	}

	int operator()(int r, int c) const
	{
		return data[r * cols + c];
	}

	int &operator()(int r, int c)
	{
		return data[r * cols + c];
	}

	matrix &operator +=(int s)
	{
		auto ptr = data;
		auto end = ptr + rows * cols;
		while (ptr != end)
		{
			*ptr += s;
			ptr++;
		}
		return *this;
	}

	matrix &operator -=(int s)
	{
		auto ptr = data;
		auto end = ptr + rows * cols;
		while (ptr != end)
		{
			*ptr -= s;
			ptr++;
		}
		return *this;
	}

	matrix &operator *=(int s)
	{
		auto ptr = data;
		auto end = ptr + rows * cols;
		while (ptr != end)
		{
			*ptr *= s;
			ptr++;
		}
		return *this;
	}

	matrix &operator /=(int s)
	{
		auto ptr = data;
		auto end = ptr + rows * cols;
		while (ptr != end)
		{
			*ptr /= s;
			ptr++;
		}
		return *this;
	}

	int *data;
	unsigned cols;
	unsigned rows;
};

matrix operator +(const matrix &x, const matrix &y)
{
	if (x.rows != y.rows || x.cols != y.cols)
	{
		throw std::invalid_argument("Invalid arguments");
	}

	matrix z;
	z.create(x.rows, y.cols);

	for (unsigned i = 0; i < x.rows; i++)
	{
		for (unsigned j = 0; j < y.cols; j++)
		{
			z(i, j) = x(i, j) + y(i, j);
		}
	}

	return z;
}

matrix operator -(const matrix &x, const matrix &y)
{
	if (x.rows != y.rows || x.cols != y.cols)
	{
		throw std::invalid_argument("Invalid arguments");
	}

	matrix z;
	z.create(x.rows, y.cols);

	for (unsigned i = 0; i < x.rows; i++)
	{
		for (unsigned j = 0; j < y.cols; j++)
		{
			z(i, j) = x(i, j) - y(i, j);
		}
	}

	return z;
}

matrix operator *(const matrix &x, const matrix &y)
{
	if (x.cols != y.rows)
	{
		throw std::invalid_argument("Invalid arguments");
	}

	matrix z;
	z.create(x.rows, y.cols);

	for (unsigned i = 0; i < x.rows; i++)
	{
		for (unsigned j = 0; j < y.cols; j++)
		{
			int zz = 0;
			for (unsigned k = 0; k < x.cols; k++)
			{
				zz += x(i, k) * y(k, j);
			}

			z(i, j) = zz;
		}
	}

	return z;
}
