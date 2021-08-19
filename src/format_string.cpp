#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> // va_list and its operations
#include <stdint.h> // various type definitions
#include <sys/types.h>
#include <ctype.h> // https://www.tutorialspoint.com/c_standard_library/ctype_h.htm
#include <string.h>
#include <math.h>
#include <limits.h>
//#include <endian.h> // __BYTE_ORDER, __LITTLE_ENDIAN　

#include "g3log/format_string.hpp"

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#define DOUBLE_ZERO double(1E-307) //10^(-307)
#define IS_DOUBLE_ZERO(D) (D <= DOUBLE_ZERO && D >= -DOUBLE_ZERO)
#define MAXFRACT    39

//typedef char* va_list;

#ifndef __va_rounded_size // sizeof(int) == 4
#define __va_rounded_size(TYPE) \
    ( (sizeof(TYPE) + sizeof(intptr_t) - 1) & ~(sizeof(intptr_t) - 1) )
#endif

//__va_rounded_size("sdsds");

#ifndef va_start
#define va_start(AP, LASTARG) \
    (AP = ((char *)& (LASTARG) + __va_rounded_size(LASTARG)))
#endif

#ifndef va_arg
#define va_arg(AP, TYPE) \
    (AP += __va_rounded_size(TYPE), *((TYPE *)(AP - __va_rounded_size(TYPE))))
#endif

#ifndef va_end
#define va_end(AP)              (AP = (va_list)0 )
#endif

#ifndef pid_t
typedef int pid_t;
#endif

// cpymem returns the next position of dst after calling memcpy
#define cpymem(dst, src, n)	   (((u_char *) memcpy(dst, src, n)) + (n))
// 32bit max unsigned ‭int 4294967295‬
#define NGX_MAX_UINT32_VALUE   (uint32_t) 0xffffffff
// NGX_INT64_LEN == 20
#define NGX_INT64_LEN          (sizeof("-9223372036854775808") - 1)

#define abs(a)  ((a) < 0 ?  -(a) :(a))
// #define is_digit(c) ((c) >= '0' && (c) <= '9')

#define FLT_MAX_10_EXP     38
#define DBL_MAX_10_EXP     308
#define LDBL_MAX_10_EXP    308

#define	PADSIZE		16		// pad chunk size

#define MAXEXP_DIGIT  4
#define	BUF           400     // buffer for %dfg etc

#define	to_digit(c)	((c) - '0')
#define is_digit(c)	((unsigned)to_digit(c) <= 9)
#define	to_char(n)	((n) + '0')

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define P_INDEX	3
#else  // __BYTE_ORDER == __BIG_ENDIAN
#define P_INDEX	0
#endif

#define MSIGN 0x7fff

typedef union
{
    unsigned short s[4];
    unsigned int   p_mant[2];
    double         p_double;
} __cv_type;

typedef unsigned char   uchar_t;
typedef unsigned short  ushort_t;
typedef unsigned int    uint_t;
typedef unsigned long   ulong_t;

// used by snprintf(), vsnprintf(), fioFormat() 
typedef struct outbuf_arg {
    char* pBuf;            // running pointer to the next char
    char* pBufEnd;         // pointer to buffer end 
}   OUTBUF_ARG;

//#if	!defined(EOF) || (EOF!=(-1))
//#define EOF		(-1)
//#endif
//
//#if	!defined(FALSE) || (FALSE!=(0))
//#define FALSE		(0)
//#endif
//
//#if	!defined(TRUE) || (TRUE!=(1))
//#define TRUE		(1)
//#endif
//
//#define NONE	(-1)	// for times when NULL won't do
//#define EOS		'\0'	// C string terminator
//
//// return status values
//#define OK		0
//#define ERROR		(-1)

// to extend shorts, signed and unsigned, arg extraction methods are needed
#define	SARG() \
    ((doLongLongInt) ? (long long) va_arg(vaList, long long) : \
    (doLongInt) ? (long long)(long)va_arg(vaList, long) : \
    (doShortInt) ? (long long)(short)va_arg(vaList, long/*int*/) : \
    (long long)(int) va_arg(vaList, long/*int*/))

#define	UARG() \
    ((doLongLongInt) ? (unsigned long long) va_arg(vaList, unsigned long long) : \
    (doLongInt) ? (unsigned long long)(ulong_t)va_arg(vaList, ulong_t): \
    (doShortInt) ? (unsigned long long)(ushort_t)va_arg(vaList, ulong_t/*int*/): \
    (unsigned long long)(uint_t) va_arg(vaList, ulong_t/*uint_t*/))

#define	PAD(howmany, with) \
{ \
    int n; \
    if ((n = (howmany)) > 0) \
	{ \
	    while (n > PADSIZE) \
	    { \
            fioBufPut(with, PADSIZE, outarg); \
	        n -= PADSIZE; \
	    } \
        fioBufPut(with, (size_t)n, outarg); \
	} \
}

// The fieldSzIncludeSign indicates whether the sign should be included
// in the precision of a number.
bool fieldSzIncludeSign = /*TRUE*/true;

static const char* blanks = "                ";
static const char* zeroes = "0000000000000000";


/******************************************************************************
*
* roundCvt - helper function for fioFormat
*
* RETURNS:
*/

// The assumption is the compiler can assign variables to registers better than 
// the programmer, so register has been deprecated and there is no other equivalent
// keyword in the new c++ standard.
// The removal of register for C++17 was approved in the Lenexa meeting but it is 
// still reserved for future use.
static char*
roundCvt(
    double	       fract,
    int*           exp,
    /*register*/ char* start,
    /*register*/ char* end,
    char 	       ch,
    bool*          pDoSign
)
{
    double tmp;

    if (fract)
        (void)modf(fract * 10, &tmp);
    else
        tmp = to_digit(ch);

    if (tmp > 4) // tmp >= 5
    {
        for (;; --end)
        {
            if (*end == '.')
                --end;
            if (++(*end) <= '9')
                break;
            *end = '0';
            if (end == start)
            {
                if (exp)
                {
                    // e/E; increment exponent
                    *end = '1';
                    ++(*exp);
                }
                else
                {
                    // f; add extra digit
                    *(--end) = '1';
                    --start;
                }
                break;
            }
        }
    }

    // ``"%.3f", (double)-0.0004'' gives you a negative 0.
    else if (*pDoSign) // 0 =< tmp =< 4 
    {
        for (;; --end)
        {
            if (*end == '.')
                --end;

            if (*end != '0')
                break;

            if (end == start)
            {
                *pDoSign = /*FALSE*/false;
                break;
            }
        }
    }

    return (start);
}


/******************************************************************************
*
* exponentCvt - helper function for fioFormat
*
* RETURNS:
*/

static char* 
exponentCvt(
    /*register*/ char* p,
    /*register*/ int exp,
    int fmtch
)
{
    /*register*/ char* t;
    char expbuf[MAXEXP_DIGIT];

    *(p++) = fmtch;
    if (exp < 0)
    {
        exp = -exp;
        *(p++) = '-';
    }
    else
        *(p++) = '+';

    t = expbuf + MAXEXP_DIGIT;

    if (exp > 9) // exp >= 10
    {
        do
        {
            *(--t) = to_char(exp % 10);
        } while ((exp /= 10) > 9);

        *(--t) = to_char(exp);

        for (; t < expbuf + MAXEXP_DIGIT; *(p++) = *(t++)) ;
    }
    else // exp =< 9 
    {
        *(p++) = '0';
        *(p++) = to_char(exp);
    }

    return (p);
}


/******************************************************************************
*
* cvt - helper function for fioFormat
*
*/

static int
cvt(
    double number,
    /*register*/ int prec,
    bool doAlt,
    int fmtch,
    bool* pDoSign,
    char* startp,
    char* endp
)
{
    /*register*/ char* p;
    /*register*/ char* t;
    /*register*/ double  fract;
    int  dotrim;
    int  expcnt;

    int  gformat = 0;
    bool nonZeroInt = /*FALSE*/false;

    double 	integer;
    double	tmp;
    __cv_type convert;

    dotrim = expcnt = 0;

    convert.p_double = number;
    // (int)~MSIGN = 1111 1111 1111 1111 1000 0000 0000 0000
    // convert.s[3] = (high addr byte + low addr byte) & (1000 0000 0000 0000)
    if (convert.s[P_INDEX] & ~MSIGN)
    {
        number = -number;
        if (number != 0.0)
            *pDoSign = /*TRUE*/true;
    }
    else
        *pDoSign = /*FALSE*/false;

    fract = modf(number, &integer); // number >= 0
    // double modf(double x, double* integer);
    // returns fractional part of x (with the same sign), and sets integer to
    // the integer component of x.

    if (integer != (double)0.0)
        nonZeroInt = /*TRUE*/true;

    t = ++startp;  // get an extra slot for rounding.
    // startp = startp + 1 = cp + 1 = buf + 1
    // t = startp

    // get integer portion of number; put into the end of the buffer; the
    // .01 is added for modf(356.0 / 10, &integer) returning .59999999...
    for (p = endp - 1; integer; ++expcnt)
    {
        tmp = modf(integer / 10, &integer);
        *(p--) = to_char((int)((tmp + .01) * 10));
    }

    switch (fmtch)
    {
    case 'f':
        // reverse integer into beginning of buffer
        if (expcnt)
            for (; ++p < endp; *(t++) = *p);
        else
            *(t++) = '0';

        // if precision required or alternate flag set, add in a
        // decimal point.
        if (prec || doAlt)
            *(t++) = '.';

        // if requires more precision and some fraction left
        if (fract)
        {
            if (prec)
            {
                do
                {
                    fract = modf(fract * 10, &tmp);
                    *(t++) = to_char((int)tmp);
                } while(--prec && fract);
            }

            if (fract)
            {
                startp = roundCvt(fract, (int*)NULL, startp, t - 1, (char)0,
                    pDoSign);
            }
        }

        for (; prec--; *(t++) = '0');
        break;

    case 'e':
    case 'E':
    eformat:
        if (expcnt)
        {
            *(t++) = *(++p);

            if (prec || doAlt)
                *(t++) = '.';

            // if requires more precision and some integer left
            for (; prec && ++p < endp; --prec)
                *(t++) = *p;

            // if done precision and more of the integer component,
            // round using it; adjust fract so we don't re-round later.
            if (!prec && ++p < endp)
            {
                fract = 0;
                startp = roundCvt((double)0, &expcnt, startp, t - 1, *p,
                    pDoSign);
            }

            --expcnt;  // adjust expcnt for dig. in front of decimal
        }
        else if (fract)
        {
            // adjust expcnt for digit in front of decimal
            for (expcnt = -1;; --expcnt)
            {
                fract = modf(fract * 10, &tmp);
                if (tmp)
                    break;
            }

            *(t++) = to_char((int)tmp);

            if (prec || doAlt)
                *(t++) = '.';
        }
        else
        {
            *(t++) = '0';
            if (prec || doAlt)
                *(t++) = '.';
        }

        // if requires more precision and some fraction left
        if (fract)
        {
            if (prec)
            {
                do
                {
                    fract = modf(fract * 10, &tmp);
                    *(t++) = to_char((int)tmp);
                } while (--prec && fract);
            }

            if (fract)
                startp = roundCvt(fract, &expcnt, startp, t - 1, (char)0,
                    pDoSign);
        }

        for (; prec--; *(t++) = '0'); // if requires more precision

        // unless alternate flag, trim any g/G format trailing 0's
        if (gformat && !doAlt)
        {
            while (t > startp && *(--t) == '0');

            if (*t == '.')
                --t;
            ++t;
        }

/*
 * Copyright (c) 1990 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 * This license only applies to the source code demarcated by the page-wide
 * line of asterisks.  The rest of the source code in this file may be
 * subject to a different open-source or proprietary software license.
 */
        t = exponentCvt(t, expcnt, fmtch);
        break;

    case 'g':
    case 'G':

        // a precision of 0 is treated as a precision of 1.
        if (!prec)
            ++prec;

        // ``The style used depends on the value converted; style e
        // will be used only if the exponent resulting from the
        // conversion is less than -4 or greater than the precision.''
        //	-- ANSI X3J11
        if (expcnt > prec || (!expcnt && fract && fract < .0001))
        {
            // g/G format counts "significant digits, not digits of
            // precision; for the e/E format, this just causes an
            // off-by-one problem, i.e. g/G considers the digit
            // before the decimal point significant and e/E doesn't
            // count it as precision.
            --prec;
            fmtch -= 2;   // G->E, g->e
            gformat = 1;
            goto eformat;
        }

        // reverse integer into beginning of buffer,
        // note, decrement precision
        if (expcnt)
            for (; ++p < endp; *(t++) = *p, --prec);
        else // no integer portion
            *(t++) = '0';

        /**********************************************************************
         * End of BSD-licensed source code
         *********************************************************************/
         
        // if precision required or alternate flag set, add in a
        // decimal point.  If no digits yet, add in leading 0.
        if (prec || doAlt)
        {
            dotrim = 1;
            *(t++) = '.';
        }
        else // prec == 0 && doAlt == 0
            dotrim = 0;

        // if requires more precision and some fraction left
        if (fract)
        {
            if (prec)
            {               
                // If there is a zero integer portion, so we can't count any
                // of the leading zeros as significant.  Roll 'em on out until
                // we get to the first non-zero one.              
                if (!nonZeroInt)
                {
                    do
                    {
                        fract = modf(fract * 10, &tmp);
                        *(t++) = to_char((int)tmp);
                    } while (!tmp);
                    prec--;
                }

                // Now add on a number of digits equal to our precision.
                while (prec && fract)
                {
                    fract = modf(fract * 10, &tmp);
                    *(t++) = to_char((int)tmp);
                    prec--;
                }
            }

            if (fract)
            {
                startp = 
                    roundCvt(fract, (int*)NULL, startp, t - 1, (char)0, pDoSign);
            }
        }

        // alternate format, adds 0's for precision, else trim 0's
        if (doAlt)
        {
            for (; prec--; *(t++) = '0');
        }
        else if (dotrim)
        {
            while (t > startp && *(--t) == '0');

            if (*t != '.') ++t;
        }

    default:
        break;
    }

    return (t - startp);
}


/******************************************************************************
 *
 * fioBufPut - put characters in a buffer
 *
 * This is a support routine for fioFormat(). This routine copies [length]
 * bytes from source to destination, leaving the destination buffer pointer
 * (pArg->pBuf) pointing at byte following block copied.
 *
 * If [length] exceeds the number of bytes available in the buffer, only the
 * number of bytes that fit within the buffer are copied.  In this case
 * OK is still returned (although no further copying will be performed) so
 * that fioFormat() can return the number of characters that would have been
 * copied if the supplied buffer was of sufficient size.
 *
 * RETURNS: OK always
 *
 */

static void 
fioBufPut(
    const char*   pInBuf,    // pointer to input buffer
    size_t        length,    // length of input buffer
    OUTBUF_ARG*   pArg       // fioBufPut argument structure
)
{
    size_t remaining;

    // check if sufficient free space remains in the buffer
    remaining = (size_t)(pArg->pBufEnd - pArg->pBuf - 1);
    // the last byte position is reserved for the terminating '\0' 

    // bail if at the end of buffer, recall need a single byte for null
    if ((ssize_t)remaining <= 0)
        return;
    //int vsnprintf(
    //    char*       buffer,   // buffer to write to
    //    size_t      count,    // max number of characters to store in buffer
    //    const char* fmt,      // format string
    //    va_list     vaList    // optional arguments to format
    //)
    // Upon successful return, vsnprintf return the number of 
    // characters printed( <= count - 1 and excluding the '\0' 
    // used to end output to strings).
    // vsnprintf do not write more than count bytes(including the
    // terminating null byte('\0')). If the output was truncated due to this 
    // limit, then the return value is the number of characters(excluding
    // the terminating null byte) which would have been written to the final
    // string if enough space had been available. Thus, a return value of 
    // size or more means that the output was truncated.

    else if (length > remaining)
        length = remaining;

    memcpy(pArg->pBuf, pInBuf, length);

    pArg->pBuf += length;

    return;
}


static int
fioFormat(
    const char* fmt,      // format string
    va_list vaList,       // pointer to varargs list
    OUTBUF_ARG* outarg    // fioFormat argument structure
)
{
    int	    ch;         // character from fmt
    int	    n;          // handy integer (short term usage)
    char*   cp;         // handy char pointer (short term usage)
    int	    width;      // width from format (e.g. %8d), or 0
    char    sign;       // sign prefix (' ', '+', '-', or '\0')

    unsigned long long
          ulongLongVal; // unsigned 64 bit arguments %[diouxX]

    int     prec;       // precision from format (e.g. %.3d), or -1
    int	    oldprec;	// old precision from format (e.g. %.3d), or -1

    int	    dprec;	    // a copy of prec if [diouxX], 0 otherwise
    int	    fpprec;	    // `extra' floating precision in [eEfgG]
    size_t	size;	    // size of converted field or string
    int	    fieldsz;	// field size expanded by sign, etc
    int	    realsz;	    // field size expanded by dprec

    char    FMT[64];	// To collect fmt info
    char*   Collect;	// char pointer to FMT

    bool    doLongInt;  // long integer
    bool    doLongLongInt;  // long long integer - 64 bit
    bool    doShortInt;	// short integer
    bool    doAlt;      // alternate form
    bool    doLAdjust;	// left adjustment
    bool    doZeroPad;	// zero (as opposed to blank) pad
    bool    doHexPrefix; // add 0x or 0X prefix

    double  dbl;

    bool    doSign;     // change sign to '-'

    char    buf[BUF];	// space for %c, %[diouxX], %[eEfgG]
    char    ox[4];		// space for 0x hex-prefix
    const char* xdigs = NULL;   // digits for [xX] conversion
    int     ret = 0;    // return value accumulator

    enum { OCT__, DEC__, HEX__ } base;   // base for [diouxX] conversion

    // Scan the format for conversions (`%' character)
    for (;;) 
    {
        // consume the current non-format string until reaching the next '%' 
        for (cp = (char*)(fmt); ((ch = *fmt) != '\0') && (ch != '%'); fmt++)
            ;

        if ((n = (int)(fmt - cp)) != 0)
        {
            fioBufPut(cp, (size_t)n, outarg);
            ret += n;
        }

        if (ch == '\0')
            return ret;	// return total length

        fmt++;   // *fmt == '\0' fmt++ skips over '%'

        *FMT = /*EOS*/'\0';  // char FMT[64];
        Collect = FMT;

        doLongInt = false;	// long integer
        doLongLongInt = false;	// 64 bit integer
        doShortInt = false;	 // short integer
        doAlt = false;	// alternate form
        doLAdjust = false;	// left adjustment
        doZeroPad = false;	// zero (as opposed to blank) pad
        doHexPrefix = false;  // add 0x or 0X prefix

        doSign = false;	// change sign to '-'

        dprec = 0;
        fpprec = 0;
        width = 0;
        prec = -1;
        oldprec = -1;

        dbl = 0.0;

        sign = /*EOS*/'\0';

#define get_CHAR  (ch = *(Collect++) = *(fmt++))
        // char  FMT[64];   // To collect fmt info
        // char* Collect;	// char pointer to FMT
        // Collect = FMT;

    rflag:
        get_CHAR;
    reswitch:
        switch (ch)
        {
        case ' ':
            // If the space and + flags both appear, the space
            // flag will be ignored. -- ANSI X3J11
            if (!sign)
                sign = ' ';
            goto rflag;

        case '#':
            doAlt = true;
            goto rflag;

        case '*':
            // A negative field width argument is taken as a
            // flag followed by a positive field width.
            // -- ANSI X3J11
            // They don't exclude field widths read from args.        
            if ((width = va_arg(vaList, int)) >= 0)
                goto rflag;

            width = -width;			// FALLTHROUGH

        case '-':
            doLAdjust = true;
            goto rflag;

        case '+':
            sign = '+';
            goto rflag;

        case '.':
            get_CHAR;
            if (ch == '*')
            {
                n = va_arg(vaList, int);
                prec = (n < 0) ? -1 : n;
                goto rflag;
            }

            n = 0;
            while (is_digit(ch))
            {
                n = 10 * n + to_digit(ch);
                get_CHAR;
            }
            prec = n < 0 ? -1 : n;
            goto reswitch;

        case '0':
            // Note that 0 is taken as a flag, not as the
            // beginning of a field width. -- ANSI X3J11

            // If the '0' and '-' both appear, the '0' flag will be ignored.
            // For 'd', 'i', 'o', 'u', 'x', and 'X' conversions, if a precision 
            // is specified, the '0' flag will be ignored. For other conversions,
            // the behavior is undefined.
            doZeroPad = true;
            goto rflag;

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            n = 0;
            do
            {
                n = 10 * n + to_digit(ch);
                get_CHAR;
            } while (is_digit(ch));

            width = n;
            goto reswitch;

        case 'h':
            doShortInt = true;
            goto rflag;

        case 'l':
            get_CHAR;
            if (ch == 'l')
            {
                doLongLongInt = true;
                goto rflag;
            }
            else
            {
                doLongInt = true;
                goto reswitch;
            }

        case 'c':
            *(cp = buf) = (char)va_arg(vaList, int);
            size = 1;
            sign = /*EOS*/'\0';
            doZeroPad = false;
            break;

        case 'D':
            doLongInt = true;			// FALLTHROUGH

        case 'd':
        case 'i':
            ulongLongVal = (unsigned long long)SARG();

            if ((long long)ulongLongVal < 0)
            {
                ulongLongVal = -ulongLongVal;
                sign = '-';
            }
            base = DEC__;
            goto number;

        case 'n':
            // The argument should be a pointer to an integer into which
            // the number of characters written to the output stream
            // so far by this call is written. No argument is converted.
            // ret is int, so effectively %lln = %ln
            if (doLongLongInt)
                *va_arg(vaList, long long*) = (long long)ret;
            else if (doLongInt)
                *va_arg(vaList, long*) = (long)ret;
            else if (doShortInt)
                *va_arg(vaList, short*) = (short)ret;
            else
                *va_arg(vaList, int*) = (int)ret;
            continue;  // no output

        case 'O':
            doLongInt = true;    // FALLTHROUGH

        case 'o':
            ulongLongVal = UARG();
            base = OCT__;
            goto nosign;

        case 'P':  // pid_t type (typedef int pid_t)
            ulongLongVal = (unsigned long long)(long long)(int)
                va_arg(vaList, pid_t);

            if ((long long)ulongLongVal < 0)
            {
                ulongLongVal = -ulongLongVal;
                sign = '-';
            }
            base = DEC__;
            goto number;

        case 'p':
            // The argument shall be a pointer to void. The value of the 
            // pointer is converted to a sequence of printable characters,
            // in an implementation defined manner. -- ANSI X3J11
            ulongLongVal = (unsigned long long) (unsigned long)
                va_arg(vaList, void*);  // NOSTRICT
            base = HEX__;
            xdigs = "0123456789abcdef";
            doHexPrefix = true;
            ch = 'x';
            goto nosign;

        case 's':
            if ((cp = va_arg(vaList, char*)) == NULL)
                cp = (char*)"(null)";
            // get string size(<=prec) excluding the tail '\0'
            if (prec >= 0)
            {
                // can't use strlen; can only look for the
                // NULL in the first `prec' characters, and
                // strlen() will go further.
                char* p = (char*)memchr(cp, 0, (size_t)prec);
                // The C library function
                // void* memchr(const void* str, int c, size_t n) 
                // searches for the first occurrence of the character 
                // c(an unsigned char) in the first n bytes of the string 
                // pointed to, by the argument str.
                // This function returns a pointer to the matching byte or NULL 
                // if the character does not occur in the given memory area.

                if (p != NULL)
                {
                    size = (size_t)(p - cp);
                    if (size > (size_t)prec)
                        size = (size_t)prec;
                }
                else
                    size = (size_t)prec;
            }
            else
                size = strlen(cp);

            sign = /*EOS*/'\0';
            doZeroPad = false;
            break;

        case 'U':
            doLongInt = true;   // FALLTHROUGH

        case 'u':
            ulongLongVal = UARG();

            base = DEC__;
            goto nosign;

        case 'X':
            xdigs = "0123456789ABCDEF";
            goto hex;

        case 'x':
            xdigs = "0123456789abcdef";

        hex:
            ulongLongVal = UARG();
            base = HEX__;

            // leading 0x/X only if non-zero
            if (doAlt && (ulongLongVal != 0))
                doHexPrefix = true;

        // unsigned conversions	
        nosign:           
            sign = /*EOS*/'\0';

        // ... [diouXx] conversions ... if a precision is specified, 
        // (prec >= 0) the 0 flag will be ignored. -- ANSI X3J11
        number:
            if ((dprec = prec) >= 0)
                doZeroPad = false;

            // The result of converting a zero value with an
            // explicit precision of zero is no characters.
            // -- ANSI X3J11
            cp = buf + BUF;
            if ((ulongLongVal != 0) || (prec != 0))
            {
                // unsigned mod is hard, and unsigned mod
                // by a constant is easier than that by
                // a variable; hence this switch.
                // ulongLongVal >= 0
                switch (base)
                {
                case OCT__:
                    do
                    {
                        *(--cp) = to_char(ulongLongVal & 7);
                        ulongLongVal >>= 3;
                    } while (ulongLongVal);

                    // handle octal leading 0
                    if (doAlt && (*cp != '0'))
                        *(--cp) = '0';
                    break;

                case DEC__:
                    // many numbers are 1 digit
                    while (ulongLongVal >= 10)
                    {
                        *(--cp) = to_char(ulongLongVal % 10);
                        ulongLongVal /= 10;
                    }

                    // ulongLongVal < 10
                    *(--cp) = (char)to_char(ulongLongVal);
                    break;

                case HEX__:
                    do
                    {
                        *(--cp) = xdigs[ulongLongVal & 15];
                        ulongLongVal >>= 4;
                    } while (ulongLongVal);
                    break;

                default:
                    cp = (char*)"bug in fioFormat: bad base";
                    size = strlen(cp);
                    goto skipsize;
                }
            }

            // size == 0 if ulongLongVal == 0 and prec == 0 
            size = (size_t)(buf + BUF - cp);

        skipsize:
            break;

        case 'L':
            // NOT IMPLEMENTED
            goto rflag;

        ///////////////////////////// FLOAT FORMAT ////////////////////////////
        case 'e':
        case 'E':
        case 'f':
        case 'g':
        case 'G':
            oldprec = prec;		// in case of strange float

            if (prec > MAXFRACT) 	// do realistic precision
            {
                if (((ch != 'g') && (ch != 'G')) || doAlt)
                    fpprec = prec - MAXFRACT;
                prec = MAXFRACT;	// they asked for it!
            }
            else if (prec == -1)
                prec = 6;		// ANSI default precision

            cp = buf;		    // where to fill in result
            *cp = /*EOS*/'\0';	// EOS terminate just in case//////////////////

            dbl = (double)va_arg(vaList, double);
            doSign = false;  // assume no sign needed

            if (/*isInf*/isinf(dbl))    // infinite?+
            {
                // fill in the string
                if (ch == 'E' || ch == 'G')
                    strcpy(cp, "INF");    
                else
                    strcpy(cp, "inf");

                if (dbl < 0.0)				 // less than 0.0
                    doSign = /*TRUE*/true; // we need a sign
                size =  -3;  // length will be three, excluding tail '\0'
                // macro isinf(x): returns whether x is an infinity value
                // (either positive infinity or negative infinity).
            }
            else if (/*isNan*/isnan(dbl))    // not a number?
            {
                // fill in the string
                if (ch == 'E' || ch == 'G')
                    strcpy(cp, "NAN");
                else
                    strcpy(cp, "nan");

                size = -3;  // length will be three
                // macro isnan(x): returns whether x is a NaN (Not-A-Number) value.
                // isnan(sqrt(-1.0)) returns 1
            }
            else
                size = cvt(dbl, prec, doAlt, ch, &doSign, cp, buf + sizeof(buf));

            if ((int)size < 0)	// strange value (Nan,Inf,..)
            {
                size = -size;  // get string length
                prec = oldprec;	 // old precision (not default)

                doZeroPad = false;  // don't pad with zeroes
                if (doSign)  // is strange value signed?
                    sign = '-';
            }
            else
            {
                if (doSign)
                    sign = '-';

                if (*cp == /*EOS*/'\0')  //////////////////////////////////////
                    cp++;
            }
            break;
        ///////////////////////////// FLOAT FORMAT ////////////////////////////

        default:			// "%?" prints ?, unless ? is NULL
            if (ch == /*EOS*/'\0')
                return ((size_t)ret);

            // pretend it was %c with argument ch
            cp = buf;
            *cp = (char)ch;
            size = 1;
            sign = /*EOS*/'\0';
            break;
        }

        // All reasonable formats wind up here. At this point, `cp' points to a
        // string which (if not doLAdjust) should be padded out to `width' 
        // places. If doZeroPad, it should first be prefixed by any sign or 
        // other prefix; otherwise, it should be blank padded before the prefix 
        // is emitted. After any left-hand padding and prefixing, emit zeroes
        // required by a decimal [diouxX] precision, then print the string 
        // properly, then emit zeroes required by any leftover floating 
        // precision; finally, if doLAdjust, pad with blanks.
 
        // compute actual size, so we know how much to pad.
        // fieldsz excludes decimal prec; realsz includes it
        
        fieldsz = (int)((int)size + fpprec); // normally fpprec is 0

        // sign != '\0' and doHexPrefix == true can not be met at the same time
        if (sign)
        {
            fieldsz++;
            if (fieldSzIncludeSign)
                dprec++;
        }
        else if (doHexPrefix)
        {
            fieldsz += 2;
            if (fieldSzIncludeSign)
                dprec += 2;
        }
        // The fieldSzIncludeSign indicates whether the sign should be included
        // in the precision of a number. This will result in a corresponding 
        // increase(1 or 2) in dprec and fieldsz.

        realsz = (dprec > fieldsz) ? dprec : fieldsz;

        ///////////////////////////// Right Adjust ////////////////////////////
        // right-adjusting blank padding
        if (!doLAdjust && !doZeroPad)
            PAD(width - realsz, blanks);
        ///////////////////////////// Right Adjust ////////////////////////////

        // prefix
        if (sign)
        {
            fioBufPut(&sign, 1, outarg);
        }
        else if (doHexPrefix)
        {
            ox[0] = '0';
            ox[1] = (char)ch;

            fioBufPut(ox, 2, outarg);
        }

        ///////////////////////////// Right Adjust ////////////////////////////
        // right-adjusting zero padding
        if (!doLAdjust && doZeroPad)
            PAD(width - realsz, zeroes);
        ///////////////////////////// Right Adjust ////////////////////////////

        // [diouXx] leading zeroes from decimal precision
        // when dprec > fieldsz, realsz == dprec and zero padding size is dprec - fieldsz;
        // when dprec =< fieldsz, realsz == fieldsz and zero padding is skipped.
        PAD(dprec - fieldsz, zeroes);

        // the string or number proper
        fioBufPut(cp, size, outarg);

        // trailing floating point zeroes
        PAD(fpprec, zeroes);

        ////////////////////////////// Left Adjust ////////////////////////////
        // left-adjusting padding (always blank)
        if (doLAdjust)
            PAD(width - realsz, blanks);
        ////////////////////////////// Left Adjust ////////////////////////////

        // finally, adjust ret
        ret += (width > realsz) ? width : realsz;
    }
}


/******************************************************************************
 *
 * tz_vsnprintf - write a string formatted with a variable argument list to a 
 * buffer, not exceeding buffer size (ANSI)
 *
 * This routine copies a string formatted with a variable argument list to a
 * specified buffer, up to a given  number of characters. The formatted string
 * will be null terminated. This routine guarantees never to write beyond the
 * provided buffer regardless of the format specifier or the arguments to be
 * formatted. The [count] argument specifies the maximum number of characters
 * to store in the buffer, including the null terminator.
 *
 * This routine is identical to snprintf(), except that it takes the variable
 * arguments to be formatted as a list [vaList] of type `va_list' rather than
 * as in-line arguments.
 *
 * RETURNS
 * The number of characters copied to [buffer], not including the
 * NULL terminator.
 *
 * Even when the supplied [buffer] is too small to hold the complete formatted 
 * string, the return value represents the number of characters that would have 
 * been written to [buffer] if [count] was sufficiently large.
 */

int 
tz_vsnprintf(
    char*       buffer,   // buffer to write to
    size_t 	    count,	  // max number of characters to store in buffer
    const char* fmt,      // format string
    va_list	    vaList    // optional arguments to format
)
{
    size_t	 nChars;
    OUTBUF_ARG snputbufArg;

    snputbufArg.pBuf = buffer;
    snputbufArg.pBufEnd = (char*)(buffer + count);

    nChars = fioFormat(fmt, vaList, &snputbufArg);

    if (count != 0)
        *snputbufArg.pBuf = /*EOS*/'\0'; // null-terminate the string

    return ((int)nChars);
}


/******************************************************************************
 *
 * tz_snprintf - write a formatted string to a buffer, not exceeding buffer size
 *
 * This routine copies a formatted string to a specified buffer, up to a given
 * number of characters. The formatted string will be null terminated. This
 * routine guarantees never to write beyond the provided buffer regardless of
 * the format specifier or the arguments to be formatted. The [count] argument 
 * specifies the maximum number of characters to store in the buffer, including 
 * the null terminator.
 *
 * RETURNS: The number of characters copied to [buffer], not including the NULL 
 * terminator. Even when the supplied [buffer] is too small to hold the complete
 * formatted string, the return value represents the number of characters that 
 * would have been written to [buffer] if [count] was sufficiently large.
 */

int 
tz_snprintf(
    char*       buffer,    // buffer to write to
    size_t      count,     // max number of characters to store in buffer
    const char* fmt,       // format string
    ...                    // optional arguments to format
)
{
    va_list	  vaList;	 // traverses the argument list
    size_t	  nChars;
    OUTBUF_ARG  snputbufArg;

    snputbufArg.pBuf = buffer;
    snputbufArg.pBufEnd = (char*)(buffer + count);

    va_start(vaList, fmt);
    nChars = fioFormat(fmt, vaList, &snputbufArg);
    va_end(vaList);

    if (count != 0)
        *snputbufArg.pBuf = /*EOS*/'\0'; // null-terminate the string

    return ((int)nChars);
}
