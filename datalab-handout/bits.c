/*
 * CS:APP Data Lab
 *
 * <Please put your name and userid here>
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */

#endif
// 1
/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y)
{
  // return ~((~(~x&y))&(~(x&~y))); // ops 8
  return (~(~x & ~y)) & (~(x & y));
}
/*
 * tmin - return minimum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void)
{
  return 1 << 31;
}
// 2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x)
{
  return !~((x + 1) ^ x) & (!!~x);
}
/*
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x)
{
  int odds = 0xAA | 0xAA << 8 | 0xAA << 16 | 0xAA << 24;
  return !((x & odds) ^ odds);
}
/*
 * negate - return -x
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x)
{
  return (~x) + 1;
}
// 3
/* 0000 0000 0011 (0000-1001)
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x)
{
  return (!((x >> 3) ^ 0x6)) | !(x ^ 0x38) | !(x ^ 0x39);
}
/*
 * conditional - same as x ? y : z
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z)
{
  int a = (~0) + (!x);
  int b = ~a;
  return (a & y) | (b & z);
}
/*
 * isLessOrEqual - if x <= y  then return 1, else return 0
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y)
{
  int xsign = x >> 31;
  int ysign = y >> 31;
  int com = y + (~x + 1);
  int xn_yp = xsign & (!ysign); // x - y +
  int xp_yn = (!xsign) & ysign; // x + y -
  return xn_yp | (!xp_yn & !(com >> 31));
}
// 4
/*
 * logicalNeg - implement the ! operator, using all of
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
int logicalNeg(int x)
{
  // assert that x is all zero
  x |= x >> 16;
  x |= x >> 8;
  x |= x >> 4;
  x |= x >> 2;
  x |= x >> 1;
  return (x & 0x1) ^ 0x1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x)
{
  // x>=0 32 - leading 0s count + 1 eg: 00001000 -> 01000 8-4+1=5
  // x<0  32 - leading 1s count + 1 eg: 11110100 -> 10100 8-4+1=5
  // if x<0, transform the question to the minimum number of bits required
  // to represent ~x in two's complement
  // because if positive a (~x) need at least k bit, then -a-1 can be represent
  // in k bit (aka. ~(~x)+1-1 = x)

  int leadingCount;
  int xAndM0;
  int xAndM1;
  int xAndM2;
  int xAndM3;
  int xAndM4;
  int xAndM5;
  int xAndM6;
  int xAndM7;
  int xAndM8;
  int xAndM9;
  int xAndM10;
  int xAndM11;
  int xAndM12;
  int xAndM13;
  int xAndM14;
  int xAndM15;
  int xAndM16;
  int xAndM17;
  int xAndM18;
  int xAndM19;
  int xAndM20;
  int xAndM21;
  int xAndM22;
  int xAndM23;
  int xAndM24;
  int xAndM25;
  int xAndM26;
  int xAndM27;
  int xAndM28;
  int xAndM29;
  int xAndM30;
  int xAndM31;

  int m0;
  int m1;
  int m2;
  int m3;
  int m4;
  int m5;
  int m6;
  int m7;
  int m8;
  int m9;
  int m10;
  int m11;
  int m12;
  int m13;
  int m14;
  int m15;
  int m16;
  int m17;
  int m18;
  int m19;
  int m20;
  int m21;
  int m22;
  int m23;
  int m24;
  int m25;
  int m26;
  int m27;
  int m28;
  int m29;
  int m30;
  int m31;

  x = ((~(x >> 31)) & x) | ((x >> 31) & (~x)); // cast negative to positive, ~x+1

  m0 = 1 << 31;
  m1 = m0 >> 1;
  m2 = m0 >> 2;
  m3 = m0 >> 3;
  m4 = m0 >> 4;
  m5 = m0 >> 5;
  m6 = m0 >> 6;
  m7 = m0 >> 7;
  m8 = m0 >> 8;
  m9 = m0 >> 9;
  m10 = m0 >> 10;
  m11 = m0 >> 11;
  m12 = m0 >> 12;
  m13 = m0 >> 13;
  m14 = m0 >> 14;
  m15 = m0 >> 15;
  m16 = m0 >> 16;
  m17 = m0 >> 17;
  m18 = m0 >> 18;
  m19 = m0 >> 19;
  m20 = m0 >> 20;
  m21 = m0 >> 21;
  m22 = m0 >> 22;
  m23 = m0 >> 23;
  m24 = m0 >> 24;
  m25 = m0 >> 25;
  m26 = m0 >> 26;
  m27 = m0 >> 27;
  m28 = m0 >> 28;
  m29 = m0 >> 29;
  m30 = m0 >> 30;
  m31 = m0 >> 31;

  xAndM0 = x & m0;
  xAndM1 = x & m1;
  xAndM2 = x & m2;
  xAndM3 = x & m3;
  xAndM4 = x & m4;
  xAndM5 = x & m5;
  xAndM6 = x & m6;
  xAndM7 = x & m7;
  xAndM8 = x & m8;
  xAndM9 = x & m9;
  xAndM10 = x & m10;
  xAndM11 = x & m11;
  xAndM12 = x & m12;
  xAndM13 = x & m13;
  xAndM14 = x & m14;
  xAndM15 = x & m15;
  xAndM16 = x & m16;
  xAndM17 = x & m17;
  xAndM18 = x & m18;
  xAndM19 = x & m19;
  xAndM20 = x & m20;
  xAndM21 = x & m21;
  xAndM22 = x & m22;
  xAndM23 = x & m23;
  xAndM24 = x & m24;
  xAndM25 = x & m25;
  xAndM26 = x & m26;
  xAndM27 = x & m27;
  xAndM28 = x & m28;
  xAndM29 = x & m29;
  xAndM30 = x & m30;
  xAndM31 = x & m31;

  leadingCount =
      !xAndM0 +
      !xAndM1 +
      !xAndM2 +
      !xAndM3 +
      !xAndM4 +
      !xAndM5 +
      !xAndM6 +
      !xAndM7 +
      !xAndM8 +
      !xAndM9 +
      !xAndM10 +
      !xAndM11 +
      !xAndM12 +
      !xAndM13 +
      !xAndM14 +
      !xAndM15 +
      !xAndM16 +
      !xAndM17 +
      !xAndM18 +
      !xAndM19 +
      !xAndM20 +
      !xAndM21 +
      !xAndM22 +
      !xAndM23 +
      !xAndM24 +
      !xAndM25 +
      !xAndM26 +
      !xAndM27 +
      !xAndM28 +
      !xAndM29 +
      !xAndM30 +
      !xAndM31;

  return 34 + (~leadingCount); // 32 - leadingCount + 1
}
// float
/*
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf)
{
  unsigned sign = uf >> 31;
  unsigned exp = uf << 1 >> 24;
  unsigned frac = uf << 9 >> 9;

  unsigned mask = 0xff;
  // NaN or infinite
  if (!(exp ^ mask))
  {
    return uf;
  }

  // denormalize, frac<<1
  if (!exp)
  {
    return sign << 31 | frac << 1;
  }

  // normalize, overflow, frac set to zeros
  if (!(exp ^ 0xfe))
  {
    return sign << 31 | 0xff << 23;
  }

  // other normalize exp+1
  return sign << 31 | (exp + 1) << 23 | frac;
}
/*
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf)
{
  return 2;
}
/*
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 *
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatPower2(int x)
{
  return 2;
}
