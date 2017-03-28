/*
 * Copyright (C) 2001 MontaVista Software Inc.
 * Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include	"printf.h"

extern void board_putc(int ch);

/* this is the maximum width for a variable */
#define		LP_MAX_BUF	256

/* macros */
#define		IsDigit(x)	( ((x) >= '0') && ((x) <= '9') )
#define		Ctod(x)		( (x) - '0')

/* forward declaration */
static int PrintChar(char *, char);
static int PrintString(char *, char *, int);
static int PrintHex(char *, unsigned long, int);

/* private variable */
static const char theFatalMsg[] = "lp_Print: ENOBUFS";

static void printf_output(const void *arg, const char *s, int l)
{
    int i;

    if (((l) < 0) || ((l) > LP_MAX_BUF)) {
	s = (char*)theFatalMsg;
	for (; *s; s++)
	    board_putc(*s);
	for(;;);
    }

    for (i=0; i< l; i++) {
	board_putc(s[i]);
	if (s[i] == '\n') board_putc('\r');
    }
}

/* -*-
 * A low level printf() function.
 */
static void
lp_Print(const void * arg,
	 const char *fmt,
	 va_list ap)
{
    char buf[LP_MAX_BUF];

    char c;
    char *s;
    int num;

    int width;

    int length;

    for(;;) {
	{
	    /* scan for the next '%' */
	    const char *fmtStart = fmt;
	    while ( (*fmt != '\0') && (*fmt != '%')) {
		fmt ++;
	    }

	    /* flush the string found so far */
	    printf_output(arg, fmtStart, fmt-fmtStart);

	    /* are we hitting the end? */
	    if (*fmt == '\0') break;
	}

	/* we found a '%' */
	fmt ++;

	width = 0;

	/* ignore long flag */
	if (*fmt == 'l') {
	    fmt ++;
	}

	/* check for other prefixes */
	if (*fmt == '0') {
	    fmt++;
	}

	if (IsDigit(*fmt)) {
	    while (IsDigit(*fmt)) {
		width = 10 * width + Ctod(*fmt++);
	    }
	}

	/* check format flag */
	switch (*fmt) {
	 case 'x':
	    num = va_arg(ap, int);
	    length = PrintHex(buf, num, width);
	    printf_output(arg, buf, length);
	    break;

	 case 'c':
	    c = (char)va_arg(ap, int);
	    length = PrintChar(buf, c);
	    printf_output(arg, buf, length);
	    break;

	 case 's':
	    s = (char*)va_arg(ap, char *);
	    length = PrintString(buf, s, width);
	    printf_output(arg, buf, length);
	    break;

	 case '\0':
	    fmt --;
	    break;

	 default:
	    /* output this char as it is */
	    printf_output(arg, fmt, 1);
	}	/* switch (*fmt) */

	fmt ++;
    }		/* for(;;) */
}


/* --------------- local help functions --------------------- */
static int
PrintChar(char * buf, char c)
{
    buf[0] = c;
    return 1;
}

static int
PrintString(char * buf, char* s, int length)
{
    int i;
    int len=0;
    char* s1 = s;
    while (*s1++) len++;
    if (length < len) length = len;

    for (i=0; i< length-len; i++) buf[i] = ' ';
    for (i=length-len; i < length; i++) buf[i] = s[i-length+len];
    return length;
}

static int
PrintHex(char * buf, unsigned long u, int length)
{
    /* algorithm :
     *  1. prints the number from left to right in reverse form.
     *  2. fill the remaining spaces with padc if length is longer than
     *     the actual length
     *     TRICKY : if left adjusted, no "0" padding.
     *		    if negtive, insert  "0" padding between "0" and number.
     *  3. we reverse the whole string including paddings
     *  4. otherwise we only reverse the actual string representing the num.
     */

    const char padc = '0';
    int actualLength =0;
    char *p = buf;
    int i;

    do {
	int tmp = u & 15;
	if (tmp <= 9) {
	    *p++ = '0' + tmp;
	} else {
	    *p++ = 'a' + tmp - 10;
	}
	u >>= 4;
    } while (u != 0);

    /* figure out actual length and adjust the maximum length */
    actualLength = p - buf;
    if (length < actualLength) length = actualLength;

    /* add padding */
    for (i = actualLength; i< length; i++) buf[i] = padc;


    /* prepare to reverse the string */
    {
	int begin = 0;
	int end;
	end = length -1;

	while (end > begin) {
	    char tmp = buf[begin];
	    buf[begin] = buf[end];
	    buf[end] = tmp;
	    begin ++;
	    end --;
	}
    }

    /* adjust the string pointer */
    return length;
}

void printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    lp_Print(0, fmt, ap);
    va_end(ap);
}

void puts(const char *s)
{
	printf("%s", s);
}
