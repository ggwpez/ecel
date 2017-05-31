#pragma once

#include <stdarg.h>
#include <stdarg.h>
#include <stddef.h>

extern int unalert;

#define fail(mf, format, ...) _fail(mf, "\n[%s:%i] " format, __FILE__, __LINE__, ##__VA_ARGS__)

///
/// \brief fail
/// \param minor Is it only a minor error, wich we can ignore, if unalert is passed?
///
int _fail(int minor_failure, char const* format, ...);

int _vfail(int minor_failure, char const* format, va_list args);
