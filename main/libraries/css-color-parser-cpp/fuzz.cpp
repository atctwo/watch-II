#include "csscolorparser.hpp"

#include <iostream>
#include <istream>

int main(int argc, char* argv[]) {
    while (__AFL_LOOP(1000)) {
        // Pass stdin to the function.
        std::cin >> std::noskipws;
        CSSColorParser::parse({ std::istream_iterator<char>(std::cin), std::istream_iterator<char>() });
    }
    return 0;
}