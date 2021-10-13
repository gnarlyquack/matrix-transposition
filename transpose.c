// This software is licensed under the following licenses. Choose whichever you
// prefer.
// -----------------------------------------------------------------------------
// ISC License
// Copyright (c) 2021 Karl Nack
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
// OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
// CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// -----------------------------------------------------------------------------
// Public Domain (www.unlicense.org)
//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or distribute
// this software, either in source code form or as a compiled binary, for any
// purpose, commercial or non-commercial, and by any means.
//
// In jurisdictions that recognize copyright laws, the author or authors of
// this software dedicate any and all copyright interest in the software to the
// public domain. We make this dedication for the benefit of the public at
// large and to the detriment of our heirs and successors. We intend this
// dedication to be an overt act of relinquishment in perpetuity of all present
// and future rights to this software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <assert.h> // assert
#include <errno.h> // errno
#include <limits.h> // ULONG_MAX
#include <stdarg.h> // va_list, va_start, va_end
#include <stdio.h> // stdout, stderr, fputs, printf, puts, vfprintf
#include <stdlib.h> // calloc


static unsigned long
gcd(unsigned long a, unsigned long b)
{
    // Compute greatest common divisor using Euclid's algorithm
    while (b)
    {
        unsigned long temp = a % b;
        a = b;
        b = temp;
    }
    return a;
}


static unsigned long
transpose_index(unsigned long rows, unsigned long cols, unsigned long index)
{
    unsigned long from_row = index / cols;
    unsigned long from_col = index - (from_row * cols);
    unsigned long to_col = from_row;
    unsigned long to_row = from_col;
    unsigned long to = to_row * rows + to_col;
    return to;
}


static int
should_transpose_cycle(unsigned long rows, unsigned long cols, unsigned long start)
{
    unsigned long count = 0;
    unsigned long from = start;
    for (;;)
    {
        from = transpose_index(rows, cols, from);
        if (from > start)
        {
            ++count;
        }
        else
        {
            break;
        }
    }

    int result = (from == start) && count;
    return result;
}


static unsigned long
transpose_cycle(
    unsigned long rows, unsigned long cols, unsigned long *matrix,
    unsigned long start)
{
    unsigned long count = 0;
    unsigned long from = start;
    unsigned long prev = matrix[from];
    for (;;)
    {
        ++count;
        unsigned long to = transpose_index(rows, cols, from);
        unsigned long temp = matrix[to];
        matrix[to] = prev;
        if (to == start)
        {
            break;
        }
        else
        {
            from = to;
            prev = temp;
        }
    }

    assert(count > 1);
    return count;
}


static void
transpose_matrix(unsigned long rows, unsigned long cols, unsigned long *matrix)
{
    unsigned long nswaps = rows * cols;
    // Reduce the number of swaps by the number of self-cycles
    nswaps -= gcd(rows - 1, cols - 1) + 1;

    if (nswaps)
    {
        unsigned long index = 1;
        // Not sure if element 1 could ever be a self-cycle, but just to be safe.
        while (transpose_index(rows, cols, index) == index)
        {
            ++index;
        }

        nswaps -= transpose_cycle(rows, cols, matrix, index);
        while (nswaps)
        {
            if (should_transpose_cycle(rows, cols, ++index))
            {
                nswaps -= transpose_cycle(rows, cols, matrix, index);
            }
        }
    }
}


static unsigned
count_digits(unsigned long value)
{
    unsigned result = 1;
    while (value /= 10)
    {
        ++result;
    }

    return result;
}


static void
print_matrix(unsigned long rows, unsigned long cols, const unsigned long *matrix)
{
    unsigned long size = rows * cols;
    unsigned ndigits = count_digits(size);

    // Generate a format string to justify matrix values based on the number of
    // digits of the largest value. So for a 100-element matrix, the format
    // string would be "%3lu". Assuming ULONG_MAX maxes out at 20 digits
    // (assuming 64-bit), the longest format string is "%20lu", which is 5
    // characters + 1 for the null terminator.
    assert(ndigits <= 20);
    char format[6];
    sprintf(format, "%%%ulu", ndigits);

    for (unsigned i = 0; i < size; ++i)
    {
        printf(format, matrix[i]);
        if (0 == ((i + 1) % cols))
        {
            fputs("\n", stdout);
        }
        else
        {
            fputs("  ", stdout);
        }
    }
    fputs("\n", stdout);
}


static _Noreturn void
error_and_exit(int show_usage, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    if (show_usage)
    {
        fputs("Usage: transpose ROWS COLS\n", stderr);
    }
    exit(EXIT_FAILURE);
}


static unsigned long
read_ul(const char *arg, const char *param)
{
    char *end;
    errno = 0;
    unsigned long result = strtoul(arg, &end, 0);

    if (!result || *end)
    {
        error_and_exit(1, "Invalid value for %s: '%s'\n", param, arg);
    }
    else if (result == ULONG_MAX && errno == ERANGE)
    {
        error_and_exit(0, "The number of %s may not exceed %lu\n", param, ULONG_MAX);
    }

    return result;
}


int
main(int argc, char **argv)
{
    if (argc != 3)
    {
        error_and_exit(1, "Received %d arguments but 2 are required\n", argc - 1);
    }

    unsigned long rows = read_ul(argv[1], "rows");
    unsigned long cols = read_ul(argv[2], "columns");
    if (rows > (ULONG_MAX / cols))
    {
        error_and_exit(0, "rows × columns may not exceed %lu\n", ULONG_MAX);
    }

    unsigned long *matrix = calloc(sizeof(*matrix), rows * cols);
    if (!matrix)
    {
        error_and_exit(0, "Unable to allocate memory for %d×%d matrix\n", rows, cols);
    }
    for (unsigned long n = 0; n < rows * cols; ++n)
    {
        matrix[n] = n + 1;
    }

    puts("Before:");
    print_matrix(rows, cols, matrix);

    transpose_matrix(rows, cols, matrix);

    puts("After:");
    print_matrix(cols, rows, matrix);

    puts("Done!");
    return 0;
}
