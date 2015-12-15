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
	validate_empty();
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
	validate_empty();
	return *this;
}

std::pair<polynomial, polynomial> polynomial::divide(const polynomial& divident,
		const polynomial& divisor) {

	if (divisor.degree() == -1) {
		throw std::domain_error("divisor is zero");
	}

	polynomial quotient = 0;
	polynomial remainder = divident;

	polynomial* rem_ptr = &remainder; //capture by reference does not work here

	//generator of the leading term of remainder
	//doesn't skip zeros
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

polynomial::eval_type polynomial::operator()(eval_type val) const {
	//Horner's method
	eval_type result = 0;
	auto it = data.begin();
	std::pair<uint_type, int_type> prev = *it;
	for (std::pair<uint_type, int_type> curr; it != data.end(); ++it) {
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
	polynomial::eval_type tmp;
	for (auto a : lhs.data) {
		for (auto b : rhs.data) {
			auto it = p.data.emplace(a.first + b.first, 0).first;

			//use a 64 bit integer to prevent overflow
			tmp = it->second;
			tmp += static_cast<polynomial::eval_type>(a.second)
					* static_cast<polynomial::eval_type>(b.second);
			tmp %= polynomial::MOD;
			it->second = tmp;

			if (it->second == 0) {
				p.data.erase(it);
			}
		}
	}
	p.validate_empty();
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
