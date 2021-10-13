Matrix Transposition
--------------------
This is a C program that implements in-place matrix transpositions. If you're
interested in the underlying concepts, take a look at the discussion below.


Building and Running
--------------------
If you have clang (or gcc) on your system, you can just run build.bash.
Alternatively, just compile transpose.c using a (C17) C compiler.

Run the program as follows (assuming an executable named 'transpose'):

    transpose ROWS COLS

The program generates and prints out a ROWS×COLS matrix and then tranposes it
and prints out the result.


Discussion
----------
Matrix transposition is an operation that transforms an M×N matrix into an N×M
matrix by swapping the matrix's rows and columns. That is, for each row, m, and
each column, n, in some matrix, A, the element A[m,n] maps to the element
B[n,m] in a new matrix, B. This "flips" a matrix over its diagonal.

My interest in this was inspired by day 138 of Handmade Hero when Casey gets
caught up on trying to de-interleave multichannel audio into separate channels.
Comments by some viewers noted that the problem boils down to a matrix
transposition, with the audio samples constituting the matrix's rows and the
audio channels constituting the matrix's columns. It struck me as a neat
example of a seemingly-academic idea being applied to an actual problem.

The most obvious solution is to allocate memory for a new matrix. We can then
just iterate through the old matrix and copy each element to its proper
location in the new one. However, this isn't particularly ideal if we don't
need to retain the old matrix. Since the original matrix and its transpose both
have the same memory footprint, it would be better if we could perform the
transposition in place. This would avoid memory allocation and deallocation as
well as a potential memory copy to replace the old matrix with the new one.

For a square matrix, the solution is straightfoward: just swap elements across
the diagonal.

    a b c    a d g
    d e f -> b e h
    g h i    c f i

For a non-square matrix, things are less clear:

    a b
    c d -> a c e g
    e f    b d f h
    g h

One place to start is to look at the memory location of each element in the
original and transposed matrixes (assuming a memory layout similar to C):

    a: 0 -> 0
    b: 1 -> 4
    c: 2 -> 1
    d: 3 -> 5
    e: 4 -> 2
    f: 5 -> 6
    g: 6 -> 3
    h: 7 -> 7

Some things to note:
- 0 and 7 -- the first and last elements -- stay in the same memory location.
- All other elements move according to the following cycles:
    1 -> 4 -> 2 -> 1
    3 -> 5 -> 6 -> 3
  We can also notate these cycles more compactly as:
    (1, 4, 2)
    (3, 5, 6)

Generalizing these observations:
- The first and last matrix elements never move, so we can always skip
  transposing them.
- Other elements move according to one or more permutation cycles. In other
  words, some initial element moves to a new location, which in turn moves the
  element at that location to another new location, and so on, until some final
  element moves into the location of the initial element.

If we could predict how many permutation cycles were involved for a given
matrix transposition and where they started, our task would again be fairly
simple: we would simply iterate through the cycles and successively transpose
the values in each cycle. Unfortunately, there doesn't seem to be an obvious
way to precompute these cycles.

One way to address this is to use a bit array (or similar structure) to keep
track of visited matrix elements. We then step through each element of the
matrix and check the corresponding index in the bit array to see if we've
already visited the element. If so, then proceed to the next element. If not,
then transpose the permutation cycle and mark visited indexes in the bit array.
Although this technically transposes the matrix in place, it does require extra
memory for the bit array, which may still be undesirable.

Another solution materializes if we take a closer, more-algorithmic look at the
process of transposing the 4×2 matrix we considered earlier:
- Element 0 doesn't move, so skip it.
- Move to element 1 and transpose its cycle (1, 4, 2).
- Move to element 2 and start walking its cycle. We see it immediately
  transposes to element 1. Since we started walking the cycle from element 2,
  we know we've already transposed element 1 as well as all other elements in
  the cycle. Therefore, we've already transposed element 2 and can move on.
- Move to element 3 and walk the cycle (3, 5, 6). Since we never visit an
  element earlier than 3, this is a new cycle. Walk the cycle again and
  transpose it.
- Move to element 4 and walk its cycle. We visit element 2, so we've already
  transposed this cycle and can move on.
- Move to element 5 and walk its cycle. We visit element 6 and then element 3,
  so we've already transposed this cycle and can move on.
- Move to element 6 and walk its cycle. We visit element 3, so we've already
  transposed this cycle and can move on.
- Element 7 doesn't move, so we're done.

The key realization here is: as we step through each element in order and walk
its permutation cycle, if we ever visit an element earlier than where we
started walking the cycle, we know we've already transposed that cycle and can
skip it. Otherwise, if we complete the cycle, we know it's a new cycle and can
transpose it. With this insight, we can perform an in-place matrix
transposition without additional memory allocation.

The trade-off is we're now doing a lot of redundant computation. Although our
sample 4×2 matrix is completely transposed after only two cycles, we visit
every element and have to walk at least part of each element's permutation
cycle to determine if it's a new cycle. If it is a new cycle, then we have to
walk it again and perform the actual transposition.

One obvious optimization is to keep track of how many transpositions we've
done. Given an M×N matrix, we have to do M×N transpositions, so once we've
transposed that many elements, we're done. Since the first and last elements
never move, we can trivially take them into account in our algorithm and
automatically eliminate 2 transpositions. This lets us skip element 0 and stop
iterating through our sample matrix after visiting element 3.

The first and last matrix elements are "self-cycles", that is, they transpose
to themselves. As we can see, identifying self-cycles is very useful because it
reduces the number of transpositions and lets us know we've finished tranposing
a matrix sooner than we would otherwise. While every matrix has at least two
self-cycles, it's possible for a matrix to have more (consider the 3×3 matrix
from above, which has 3 self-cycles). It turns out, we can calculate exactly
how many.

For some M×N matrix, the number of self-cycles = 1 + gcd(M - 1, N - 1), where
gcd calculates the greatest common divisor. Note that this always results in at
least 2 self-cycles, reflecting the first and last matrix elements. So at the
offset, we can calculate the number of self-cycles and subtract it from the
number of matrix elements. This tells us how many transpositions we have to do,
and once we've done that many, the matrix is fully transposed.


References
----------
- Casey Muratori, "Handmade Hero Day 138 - Loading WAV Files", YouTube video,
  June 17, 2015, https://youtu.be/RSxUBaoomy0.
- Gustavson F.G., Swirszcz T. (2007) In-Place Transposition of Rectangular
  Matrices. In: Kågström B., Elmroth E., Dongarra J., Waśniewski J. (eds)
  Applied Parallel Computing. State of the Art in Scientific Computing. PARA
  2006.  Lecture Notes in Computer Science, vol 4699. Springer, Berlin,
  Heidelberg.  https://doi.org/10.1007/978-3-540-75755-9_68
