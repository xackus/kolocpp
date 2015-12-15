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
 * A single indeterminate (x) polynomial. Coefficients are integers modulo 10^9 + 9.
 */
class polynomial {
public:
	using eval_type = int_least64_t; //for evaluation operator
	using int_type = int_least32_t;
	using uint_type = uint_least32_t;

	struct pair_ctor {
	};
	struct all_one_ctor {
	};

	static const int_type MOD = 1e9 + 9;

	/**
	 * Zero polynomial.
	 */
	polynomial() {
		data.insert( { 0, 0 });
	}

	/**
	 * Constant polynomial.
	 * Allows implicit conversion from integral types.
	 */
	polynomial(int_type val) {
		data.insert( { 0, val % MOD });
	}

	/**
	 * Polynomial with all terms that have degree in range `[0, n]`. All coefficients set to 1.
	 */
	polynomial(uint_type n, all_one_ctor) {
		for (int i = n; i >= 0; --i) {
			data.emplace_hint(data.end(), i, 1);
		}
	}

	/**
	 * Constructs a polynomial with its coefficients copied from range `[first, last)`.
	 */
	template<typename input_iterator>
	polynomial(input_iterator first, input_iterator last) {
		int_type tmp;
		for (int i = std::distance(first, last) - 1; first != last;
				--i, ++first) {
			tmp = *first % MOD;
			if (tmp == 0) {
				continue;
			}
			data.emplace_hint(data.end(), i, tmp);
		}
		validate_empty();
	}

	/**
	 * Constructs a polynomial with given coefficients.
	 */
	polynomial(std::initializer_list<int_type> il) :
			polynomial(il.begin(), il.end()) {
	}

	/**
	 * Constructs a polynomial with given coefficients.
	 */
	template<typename ... Args>
	polynomial(Args ... args) :
			polynomial( { args... }) {
	}

	/**
	 * Constructs a polynomial with its terms copied from range `[first, last)`.
	 * A term is a `std::pair<uint_type, int_type>` of a degree and a coefficient.
	 * No two terms shall have the same degree.
	 */
	template<typename input_iterator>
	polynomial(input_iterator first, input_iterator last, pair_ctor) {
		int_type tmp;
		for (; first != last; ++first) {
			tmp = first->second % MOD;
			if (tmp == 0) {
				continue;
			}
			data.emplace_hint(data.end(), first->first, tmp);
		}
		validate_empty();
	}

	/**
	 * Constructs a polynomial with its terms copied from an initializer list.
	 * A term is a `std::pair<uint_type, int_type>` of a degree and a coefficient.
	 * No two terms shall have the same degree.
	 */
	polynomial(std::initializer_list<std::pair<uint_type, int_type>> il) :
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
	int_type operator[](uint_type degree) const {
		auto it = data.find(degree);
		return it != data.end() ? it->second : 0;
	}

	/**
	 * Addition assignment.
	 */
	polynomial& operator+=(const polynomial& rhs);

	/**
	 * Addition.
	 */
	friend polynomial operator+(polynomial lhs, const polynomial& rhs) {
		lhs += rhs;
		return lhs;
	}

	/**
	 * Subtraction assignment.
	 */
	polynomial& operator-=(const polynomial& rhs);

	friend polynomial operator-(polynomial lhs, const polynomial& rhs) {
		lhs -= rhs;
		return lhs;
	}

	/**
	 * Multiplication assignment.
	 */
	polynomial& operator*=(const polynomial& rhs) {
		data = (*this * rhs).data;
		return *this;
	}

	/**
	 * Multiplication.
	 */
	friend polynomial operator*(const polynomial& lhs, const polynomial& rhs);

	/**
	 * Evaluates the polynomial at the given value of x.
	 */
	eval_type operator()(eval_type val) const;

	/**
	 * Performs a polynomial long division.
	 * @return a pair containing quotient and remainder
	 * @throws domain_error if divisor is zero
	 */
	static std::pair<polynomial, polynomial> divide(const polynomial& divident,
			const polynomial& divisor);

	/**
	 * Performs a polynomial long division and returns the quotient.
	 * @throws domain_error if divisor is zero
	 */
	static polynomial quotient(const polynomial& divident,
			const polynomial& divisor) {
		return divide(divident, divisor).first;
	}

	/**
	 * Performs a polynomial long division and returns the remainder.
	 * @throws domain_error if divisor is zero
	 */
	static polynomial remainder(const polynomial& divident,
			const polynomial& divisor) {
		return divide(divident, divisor).second;
	}

	/**
	 * Returns a monomial that is given degree with its coefficient set to 1.
	 */
	static polynomial monomial(uint_type n) {
		return polynomial { { n, 1 } };
	}

	/**
	 * Stream output.
	 */
	friend std::ostream& operator<<(std::ostream& os, const polynomial& p);

private:
	std::map<uint_type, int_type, std::greater<uint_type>> data;

	//data shouldn't be empty
	void validate_empty() {
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
		validate_empty();
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
