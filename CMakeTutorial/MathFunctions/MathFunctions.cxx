#include <cmath>

#include "MathFunctions.h"

#ifdef USE_MYMATH
#   include "mysqrt.h"
#endif // USE_MYMATH

namespace mathfunctions
{
    double sqrt(double x)
    {
#ifdef USE_MYMATH
        return detail::sqrt(x);
#else
        return std::sqrt(x);
#endif // USE_MYMATH
    }
} // namespace mathfunctions
