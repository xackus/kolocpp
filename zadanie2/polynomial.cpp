/*
 * polynomial.cpp
 * Maciej Walczak
 */

#include "polynomial.hpp"

polynomial& polynomial::operator+=(const polynomial& rhs) {
	for (auto p : rhs.data) {
		auto it = data.emplace(p.first, 0).first;
		it->second += p.second;
		it->second %= MOD;
		if (it->second == 0) {
			data.erase(it);
		}
	}
	validate_zero();
	return *this;
}

polynomial& polynomial::operator-=(const polynomial& rhs) {
	for (auto p : rhs.data) {
		auto it = data.emplace(p.first, 0).first;
		it->second -= p.second;
		it->second %= MOD;
		if (it->second == 0) {
			data.erase(it);
		}
	}
	validate_zero();
	return *this;
}

std::pair<polynomial, polynomial> polynomial::divide(const polynomial& divident,
		const polynomial& divisor) {
	polynomial quotient = 0;
	polynomial remainder = divident;

	polynomial* rem_ptr = &remainder;//capture by reference does not work here

	//generator of the leading term of remainder
	std::function<polynomial()> leading = [rem_ptr] {
		int deg = rem_ptr->degree() + 1;
		return [=]() mutable {
			--deg;
			return polynomial { {deg, (*rem_ptr)[deg]}};
		};
	}();

	polynomial divisor_leading { { divisor.degree(), divisor[divisor.degree()] } };

	polynomial term;
	while (remainder.degree() >= divisor.degree()) {
		polynomial ld = leading();
		term = div_monomial(ld, divisor_leading);
		quotient += term;
		remainder -= term * divisor;
	}

	quotient.clean_zeros();
	remainder.clean_zeros();

	return {quotient, remainder};
}

polynomial::int_type polynomial::operator ()(int_type val) const {
	//Horner's method
	int_type result = 0;
	auto it = data.begin();
	std::pair<int_type, int_type> prev = *it;
	for (std::pair<int_type, int_type> curr; it != data.end(); ++it) {
		curr = *it;
		result *= std::pow(val, prev.first - curr.first);
		result += curr.second;

		prev = curr;
	}
	result *= std::pow(val, prev.first - 0);
	return result;
}

polynomial operator*(const polynomial& lhs, const polynomial& rhs) {
	polynomial p;
	for (auto a : lhs.data) {
		for (auto b : rhs.data) {
			auto it = p.data.emplace(a.first + b.first, 0).first;
			it->second += a.second * b.second;
			it->second %= polynomial::MOD;
			if (it->second == 0) {
				p.data.erase(it);
			}
		}
	}
	p.validate_zero();
	return p;
}

std::ostream& operator<<(std::ostream& os, const polynomial& p) {
	auto it = p.data.begin();

	//print the first one
	if (it->second == -1 && it->first != 0) {
		os << "-";
	} else if (it->second != 1 || it->first == 0) {
		os << it->second;
	}

	if (it->first > 1) {
		os << "x^" << it->first;
	} else if (it->first == 1) {
		os << "x";
	}
	++it;
	//print the rest
	for (; it != p.data.end(); ++it) {
		if (it->second > 0) {
			os << " + ";
			if (it->second != 1 || it->first == 0) {
				os << it->second;
			}
		} else if (it->second < 0) {
			os << " - ";
			if (it->second != -1 || it->first == 0) {
				os << -(it->second);
			}
		} else {
			continue;
		}
		if (it->first > 1) {
			os << "x^" << it->first;
		} else if (it->first == 1) {
			os << "x";
		}
	}

	return os;
}
