#pragma once

// TODO:При компиляции под форточками невозможно статическое связывание.
// Варианты лечения на выбор:
// 1. CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS
// 2. Как намекается в туторе, использовать модуль GenerateExportHeader, что предпочтительнее
#if defined(_WIN32)
#   if defined(MATHLIBRARY_EXPORT)
#       define MATHFUNCTIONS_API __declspec(dllexport)
#   else
#       define MATHFUNCTIONS_API __declspec(dllimport)
#   endif
#else
#   define MATHFUNCTIONS_API
#endif

namespace mathfunctions
{
    MATHFUNCTIONS_API double sqrt(double);
}
