#include <iostream>
#include "stream/RandomStreamGen.hpp"
#include "hash/HashFuncGen.hpp"

int main() {
    RandomStreamGen gen(123, 1000, RandomStreamGen::Mode::Mixed);
    HashFuncGen h(777);

    auto idx = gen.split_indices_by_fraction(0.1);
    std::cout << "splits: " << idx.size() << "\n";
    std::cout << "first: " << gen.stream()[0] << "\n";
    std::cout << "hash: " << h(gen.stream()[0]) << "\n";
    return 0;
}
