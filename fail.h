#pragma once

#include <stdarg.h>

extern int unalert;
///
/// \brief fail
/// \param minor Is it only a minor error, wich we can ignore, if unalert is passed?
///
int fail(int minor_failure, char const* format, ...);

int vfail(int minor_failure, char const* format, va_list args);
