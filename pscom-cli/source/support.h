#pragma once

#include <QTextStream>


#define elif else if

// BEGIN Insane pragma stuff
// CREDITS (edited):
// https://groups.google.com/g/comp.std.c/c/d-6Mj5Lko_s/m/ZM7VM42uLFEJ
#define __VA_NARG__(...) \
(__VA_NARG_(_0, ## __VA_ARGS__, __RSEQ_N()) - 1)
#define __VA_NARG_(...) \
__VA_ARG_N(__VA_ARGS__)
#define __VA_ARG_N( \
_1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
_21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
_41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
_51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
_61,_62,_63,N,...) N
#define __RSEQ_N() \
63, 62, 61, 60, \
59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
9, 8, 7, 6, 5, 4, 3, 2, 1, 0

// CREDITS (extended):
// https://stackoverflow.com/a/11994395/13994294
#define FE_0(WHAT, C, NC)
#define FE_1(WHAT, C, NC, X) WHAT(0, X, NC)
#define FE_2(WHAT, C, NC, X, ...) WHAT(1, X, C) FE_1(WHAT, C, NC, __VA_ARGS__)
#define FE_3(WHAT, C, NC, X, ...) WHAT(2, X, C) FE_2(WHAT, C, NC, __VA_ARGS__)
#define FE_4(WHAT, C, NC, X, ...) WHAT(3, X, C) FE_3(WHAT, C, NC, __VA_ARGS__)
#define FE_5(WHAT, C, NC, X, ...) WHAT(4, X, C) FE_4(WHAT, C, NC, __VA_ARGS__)

#define __GET_MACRO(_0, _1, _2, _3, _4, _5, NAME, ...) NAME
#define _FOR_EACH(action, C, NC, ...) __GET_MACRO( \
		_0, ##__VA_ARGS__, FE_5, FE_4, FE_3, FE_2, FE_1, FE_0, \
	)(action, C, NC, ##__VA_ARGS__)

#define STRINGIFY_(X) #X
#define STRINGIFY(X) STRINGIFY_(X)
// END Insane pragma stuff


QStringList intersection(const QList<QStringList> &vecs);

QString variantToString(const QVariant variant);
