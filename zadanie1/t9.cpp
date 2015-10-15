/*
 * t9.cpp
 */

#include <fstream>
#include <iostream>
#include <initializer_list>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

using namespace std;

class T9_dictionary {
private:

	char digit[128]; //a lookup table for converting letters into digits

	static const unordered_set<string> not_found; //returned when no match is found

	unordered_map<string, unordered_set<string>> data;

	//helper function for filling the lookup table
	void map(initializer_list<char> what, char to) {
		for (char c : what) {
			digit[static_cast<size_t>(c)] = to;
		}
	}

public:
	T9_dictionary() {
		map( { 'a', 'b', 'c' }, '2');
		map( { 'd', 'e', 'f' }, '3');
		map( { 'g', 'h', 'i' }, '4');
		map( { 'j', 'k', 'l' }, '5');
		map( { 'm', 'n', 'o' }, '6');
		map( { 'p', 'q', 'r', 's' }, '7');
		map( { 't', 'u', 'v' }, '8');
		map( { 'w', 'x', 'y', 'z' }, '9');
	}

	void add_word(const string& word) {
		string converted;

		for (char c : word) {
			converted.push_back(digit[static_cast<size_t>(c)]);
		}

		//unordered_set will silently ignore duplicates
		data[converted].insert(word);
	}

	const unordered_set<string>& get(const string& in) const {
		auto it = data.find(in);
		if (it == data.end()) {
			return not_found;
		} else {
			return it->second;
		}
	}
};

const unordered_set<string> T9_dictionary::not_found = { "BRAK" };

int main() {
	T9_dictionary dict;
	ifstream fin("slownik.txt");

	if (!fin) {
		cerr << "nie udalo sie wczytac pliku slownik.txt\n";
		return 1;
	}

	string line;
	while (getline(fin, line)) {
		if (line.empty()
				|| find_if_not(line.begin(), line.end(), [](char c) {return islower(c);}) != line.end()) {
			cerr << "niewlasciwy format pliku slownik.txt\n";
			return 2;
		}

		dict.add_word(line);
	}

	while (getline(cin, line)) {
		if (line.empty()
				|| find_if_not(line.begin(), line.end(), [](char c) {return isdigit(c);}) != line.end()) {
			cerr << "niewlasciwy format wejscia\n";
			return 3;
		}

		cout << line << ":";
		for (const string& s : dict.get(line)) {
			cout << " " << s;
		}
		cout << "\n";
	}
}
