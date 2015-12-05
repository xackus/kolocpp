/*
 * polynomial.hpp
 * Maciej Walczak
 */

#ifndef POLYNOMIAL_HPP_
#define POLYNOMIAL_HPP_

#include <ostream>
#include <initializer_list>
#include <map>
#include <utility>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <functional>

/**
 * A single indeterminate polynomial with integer coefficients modulo 10^9 + 9.
 */
class polynomial {
public:
	using int_type = int_least64_t;

	struct pair_ctor {
	};
	struct all_one_ctor {
	};

	static const int_type MOD = 1e9 + 9;

	/**
	 * Zero polynomial
	 */
	polynomial() {
		data.insert( { 0, 0 });
	}

	/**
	 * Constant polynomial
	 */
	polynomial(int_type val) {
		data.insert( { 0, val % MOD });
	}

	/**
	 * Polynomial of degree `n` with all coefficients set to 1.
	 * Negative degree results in undefined behavior.
	 */
	polynomial(int_type n, all_one_ctor) {
		for (int i = n; i >= 0; --i) {
			data.emplace_hint(data.end(), i, 1);
		}
	}

	/**
	 * Constructs a polynomial with its coefficients copied from range `[first, last)`.
	 * The coefficients will be assigned to terms in order of decreasing degree.
	 */
	template<typename input_iterator>
	polynomial(input_iterator first, input_iterator last) {
		for (int i = std::distance(first, last) - 1; first != last;
				--i, ++first) {
			if (*first == 0) {
				continue;
			}
			data.emplace_hint(data.end(), i, *first % MOD);
		}
		validate_zero();
	}

	/**
	 * Constructs a polynomial with its coefficients copied from an initializer list.
	 * The coefficients will be assigned to terms in order of decreasing degree.
	 */
	polynomial(std::initializer_list<int_type> il) :
			polynomial(il.begin(), il.end()) {
	}

	/**
	 * Constructs a polynomial with its terms copied from range `[first, last)`.
	 * A term is a `std::pair` of a degree and a coefficient.
	 * A negative degree results in undefined behavior.
	 * Multiple terms that are the same degree result in undefined behavior.
	 */
	template<typename input_iterator>
	polynomial(input_iterator first, input_iterator last, pair_ctor) {
		for (; first != last; ++first) {
			if (first->second == 0) {
				continue;
			}
			data.emplace_hint(data.end(), first->first, first->second % MOD);
		}
		validate_zero();
	}

	/**
	 * Constructs a polynomial with its terms copied from an initializer list.
	 * A term is a `std::pair` of a degree and a coefficient.
	 * A negative degree results in undefined behavior.
	 * Multiple terms that are the same degree result in undefined behavior.
	 */
	polynomial(std::initializer_list<std::pair<int_type, int_type>> il) :
			polynomial(il.begin(), il.end(), pair_ctor()) {
	}

	/**
	 * Returns the degree of the polynomial or -1 if it's the zero polynomial.
	 */
	int_type degree() const {
		auto deg = *data.begin();
		if (deg.first == 0 && deg.second == 0) {
			return -1;
		} else {
			return deg.first;
		}
	}

	/**
	 * Returns the coefficient of the term that is the given degree.
	 * (Zero if such a term does not exist.)
	 */
	int_type operator[](int_type degree) const {
		auto it = data.find(degree);
		return it != data.end() ? it->second : 0;
	}

	polynomial& operator+=(const polynomial& rhs);

	friend polynomial operator+(polynomial lhs, const polynomial& rhs) {
		lhs += rhs;
		return lhs;
	}

	polynomial& operator-=(const polynomial& rhs);

	friend polynomial operator-(polynomial lhs, const polynomial& rhs) {
		lhs -= rhs;
		return lhs;
	}

	polynomial& operator*=(const polynomial& rhs) {
		data = (*this * rhs).data;
		return *this;
	}

	friend polynomial operator*(const polynomial& lhs, const polynomial& rhs);

	/**
	 * Evaluates the polynomial at the given value of x.
	 */
	int_type operator()(int_type val) const;

	/**
	 * Performs a polynomial long division.
	 * Division by zero results in undefined behavior.
	 * @return a pair containing quotient and remainder
	 */
	static std::pair<polynomial, polynomial> divide(const polynomial& divident,
			const polynomial& divisor);

	/**
	 * Performs a polynomial long division and returns the quotient.
	 * Division by zero results in undefined behavior.
	 */
	static polynomial quotient(const polynomial& divident,
			const polynomial& divisor) {
		return divide(divident, divisor).first;
	}

	/**
	 * Performs a polynomial long division and returns the remainder.
	 * Division by zero results in undefined behavior.
	 */
	static polynomial remainder(const polynomial& divident,
			const polynomial& divisor) {
		return divide(divident, divisor).second;
	}

	/**
	 * Returns a monomial of given degree with its coefficient set to 1.
	 */
	static polynomial monomial(unsigned n) {
		return polynomial { { n, 1 } };
	}

	friend std::ostream& operator<<(std::ostream& os, const polynomial& p);

private:
	std::map<int_type, int_type, std::greater<int_type>> data;

	void validate_zero() {
		if (data.size() == 0) {
			data.insert( { 0, 0 });
		}
	}

	void clean_zeros() {
		for (auto it = data.begin(); it != data.end(); ++it) {
			if (it->second == 0) {
				data.erase(it);
			}
		}
		validate_zero();
	}

	static polynomial div_monomial(const polynomial& divident,
			const polynomial& divisor) {
		int_type degree = divident.degree() - divisor.degree();
		int_type coefficient = divident.data.begin()->second
				/ divisor.data.begin()->second;
		return { {degree, coefficient}};
	}
};

#endif /* POLYNOMIAL_HPP_ */
