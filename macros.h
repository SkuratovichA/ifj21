/**
 * @file macros.h
 *
 * @brief Macros nobody used.
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 */
#pragma once

#define _cat_2_(a, b) a##_##b

#define case_1(first)  case first
#define case_2(first, ...)  case first: case_1(__VA_ARGS__)
#define case_3(first, ...)  case first: case_2(__VA_ARGS__)
#define case_4(first, ...)  case first: case_3(__VA_ARGS__)
#define case_5(first, ...)  case first: case_4(__VA_ARGS__)
#define case_6(first, ...)  case first: case_5(__VA_ARGS__)
#define case_7(first, ...)  case first: case_6(__VA_ARGS__)
#define case_8(first, ...)  case first: case_7(__VA_ARGS__)
#define case_9(first, ...)  case first: case_8(__VA_ARGS__)
#define case_10(first, ...) case first: case_9(__VA_ARGS__)
#define case_11(first, ...) case first: case_10(__VA_ARGS__)
#define case_12(first, ...) case first: case_11(__VA_ARGS__)
#define case_13(first, ...) case first: case_12(__VA_ARGS__)
#define case_14(first, ...) case first: case_13(__VA_ARGS__)
#define case_15(first, ...) case first: case_14(__VA_ARGS__)
#define case_16(first, ...) case first: case_15(__VA_ARGS__)
#define case_17(first, ...) case first: case_16(__VA_ARGS__)
#define case_18(first, ...) case first: case_17(__VA_ARGS__)
#define case_19(first, ...) case first: case_18(__VA_ARGS__)
#define case_20(first, ...) case first: case_19(__VA_ARGS__)
#define case_21(first, ...) case first: case_20(__VA_ARGS__)
#define case_22(first, ...) case first: case_21(__VA_ARGS__)
#define case_23(first, ...) case first: case_22(__VA_ARGS__)
#define case_24(first, ...) case first: case_23(__VA_ARGS__)
#define case_25(first, ...) case first: case_24(__VA_ARGS__)
#define case_26(first, ...) case first: case_25(__VA_ARGS__)
#define case_27(first, ...) case first: case_26(__VA_ARGS__)
#define case_28(first, ...) case first: case_27(__VA_ARGS__)

// for regular expressions
#define case_alpha \
    case_26('a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',  \
            'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'): \
    case_26('A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',  \
            'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z')

#define case_nonzero_digit case_9('1', '2', '3', '4', '5', '6', '7', '8', '9')

#define case_digit case_nonzero_digit: case '0'

#define case_alnum \
    case_alpha: case_digit \

