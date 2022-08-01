#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <mutex>
#include <chrono>

using ull = unsigned long long int;

struct random {
    std::random_device rd;
    std::unique_ptr<std::mt19937> gen;
    std::unique_ptr<std::uniform_int_distribution<int>> distribution;

    random(unsigned int min, unsigned int max) {
        gen = std::make_unique<std::mt19937>(rd());
        distribution = std::make_unique<std::uniform_int_distribution<int>>(min, max);
    }

    auto operator()() const {
        return (*distribution)((*gen));
    }
};

struct game {
private:
    std::unique_ptr<std::vector<char>> str;
    std::unique_ptr<random> rand;

    unsigned long long int STR_SIZE;
    unsigned int NTHREADS;
    std::mutex mutex;

    std::vector<char> characters;

    auto random_char() {
        return characters[(*rand)()];
    };

    auto generate_string(ull start, ull end) {
        for(ull i=start; i<end; i++) {
            (*str)[i] = random_char();
        }
    }

    auto search(const std::string& s, ull start, ull end, bool* found) {
        if(*found) return;

        for(ull i=start; i<end; i++) {
            if(*found) return;

            if((*str)[i] == s[0]) {
                int k = 0;
                for(ull j=i; k<s.size(); j++, k++)
                    if((*str)[j] != s[k])
                        break;
                if(k == s.size()) {
                    std::lock_guard guard(mutex);
                    *found = true;
                    return;
                }
            }
        }
    }
/*
 * // to compare performance against multithread
    auto search_s(const std::string& input) {
        std::string s = input;
        for(ull i=0; i<(*str).size(); i++) {
            if((*str)[i] == s[0]) {
                int k=0;
                for(ull j=i; k<s.size(); j++, k++) {
                    if((*str)[j] != s[k]) break;
                }
                if(k == s.size()) {
                    return;
                }
            }
        }
    }
*/

public:
    explicit game(unsigned long long int str_size) {
        STR_SIZE = str_size;

        characters.reserve(26);
        for(auto i='a'; i<='z'; i++)
            characters.emplace_back(i);

        str = std::make_unique<std::vector<char>>(STR_SIZE);
        rand = std::make_unique<random>(0, characters.capacity()-1);

        std::vector<std::thread> threads;
        NTHREADS = std::thread::hardware_concurrency();
        threads.reserve(NTHREADS);

        auto step = STR_SIZE / NTHREADS;
        auto remain = STR_SIZE % NTHREADS;
        auto start = 0ull;
        auto end = start + step;

        for(auto i=0; i<NTHREADS; i++) {
            threads.emplace_back(std::thread(&game::generate_string, this, start, end));

            start = end;
            end = end + (i == NTHREADS-2 ? step + remain : step);
        }

        for(auto& t : threads) t.join();
    }

    auto run(const std::string& input) {
        std::vector<std::thread> threads;

        auto step = STR_SIZE / NTHREADS;
        auto remain = STR_SIZE % NTHREADS;
        auto start = 0ull;
        auto end = start + step;
        bool found = false;

        for(auto i=0; i<NTHREADS; i++) {
            threads.emplace_back(std::thread(&game::search, this, input, start, end, &found));

            start = end;
            end = end + (i == NTHREADS-2 ? step + remain : step);
        }

        for(auto& t : threads) t.join();

        return found;
    }

    auto print_string() {
        for(const auto& s : *str)
            std::cout << s;
        std::cout << "\n";
    }
};

int main() {
    unsigned long long int str_size;

    std::cout << "Insert the string size:\n";
    std::cin >> str_size;
    std::cout << "Generating string with size " << str_size << "...\n";
    game g(str_size);
    std::cout << "String generated!\n";

    std::string guess;
    while(true) {
        std::cout << "Insert a word:\n";
        std::cin >> guess;

        if(guess == "0") break;

        if(g.run(guess))
            std::cout << guess << " is in the string!\n";
        else
            std::cout << "It looks like " << guess << " is not in the in the string.\n";
    }

    return 0;
}
