// A simple program that computes the square root of a number
#include <cmath>
#include <iostream>
#include <string>
#include <regex>

#include "TutorialConfig.h"

#ifdef USE_MYMATH
#   include "MathFunctions.h"
#endif // USE_MYMATH

int main(int argc, char* argv[])
{
    // V0VAska: for test purposes
    int not_used_var;

    if (argc < 2) {
        std::cmatch match;
        std::regex_search(argv[0], match, std::regex("[^\\\\|/]+$"));

        std::cout << match[0] << " Version " << Tutorial_VERSION_MAJOR << "."
                  << Tutorial_VERSION_MINOR << std::endl;
        std::cout << "Usage: " << match[0] << " number" << std::endl;
        return 1;
    }

    const double inputValue = std::stod(argv[1]);
    
    // calculate square root
#ifdef USE_MYMATH
    const double outputValue = mysqrt(inputValue);
#else
    const double outputValue = sqrt(inputValue);
#endif // USE_MYMATH
    std::cout << "The square root of " << inputValue << " is " << outputValue
              << std::endl;
    return 0;
}
