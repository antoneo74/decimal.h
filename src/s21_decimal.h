#ifndef SRC_S21_DECIMAL_H_
#define SRC_S21_DECIMAL_H_

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INT 0x7FFFFFFF  // 01111111111111111111111111111111
#define MIN_INT 0x80000000  // 10000000000000000000000000000000

typedef struct {
  unsigned bits[4];
} s21_decimal;

typedef struct {
  unsigned bits[8];
} s21_big_decimal;

enum rank { LOW, MID, HIGH, SCALE };

/// Операторы сравнения

int s21_is_less(s21_decimal a, s21_decimal b);
int s21_is_less_simple(s21_big_decimal a, s21_big_decimal b);
int s21_is_less_or_equal(s21_decimal a, s21_decimal b);
int s21_is_less_or_equal_simple(s21_big_decimal a, s21_big_decimal b);
int s21_is_greater(s21_decimal a, s21_decimal b);
int s21_is_greater_simple(s21_big_decimal a, s21_big_decimal b);
int s21_is_greater_or_equal(s21_decimal a, s21_decimal b);
int s21_is_greater_or_equal_simple(s21_big_decimal a, s21_big_decimal b);
int s21_is_equal(s21_decimal a, s21_decimal b);
int s21_is_equal_simple(s21_big_decimal a, s21_big_decimal b);
int s21_is_not_equal(s21_decimal a, s21_decimal b);
int s21_is_not_equal_simple(s21_big_decimal a, s21_big_decimal b);

/// Преобразователи

int s21_from_int_to_decimal(int src, s21_decimal *dst);
int s21_from_float_to_decimal(float src, s21_decimal *dst);  //--------------
int s21_from_decimal_to_int(s21_decimal src, int *dst);
int s21_from_decimal_to_float(s21_decimal src, float *dst);  //--------------
void convert_from_decimal_to_big_decimal(s21_decimal src, s21_big_decimal *dst);
void convert_from_big_decimal_to_decimal(s21_big_decimal src, s21_decimal *dst);

/// Арифметические операторы

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);

/// Другие функции

// Округляет децимал по ближайшего целого числа в сторону отрицательной
// бесконечности
int s21_floor(s21_decimal value, s21_decimal *result);

// Округляет децимал по ближайшего целого числа
int s21_round(s21_decimal value, s21_decimal *result);

// выводит децимал без дробной части и конечных нулей
int s21_truncate(s21_decimal value, s21_decimal *result);

// умножает децимал на -1
int s21_negate(s21_decimal value, s21_decimal *result);

/// Битовые операции и преобразования

unsigned get_bit(s21_decimal src, unsigned ind);
void null_bit(s21_decimal *src, unsigned ind);
void set_bit(s21_decimal *src, unsigned ind);
// void print_bit(s21_decimal n);
void invert_bit(s21_decimal *src, unsigned ind);

/// доп. функции для работы с децимал
void s21_init_decimal_by_zero(s21_decimal *m);
// void shift_left(s21_decimal *src, unsigned count);
// void shift_right(s21_decimal *src, unsigned count);
unsigned get_scale(s21_decimal value);
void null_scale(s21_decimal *value);
void set_scale(s21_decimal *value, unsigned scale);
unsigned check_decimal_equal_zero(s21_decimal src);

/// Битовые операции и преобразования with big decimal

unsigned get_bit_big_decimal(s21_big_decimal src, unsigned ind);
void set_bit_big_decimal(s21_big_decimal *src, unsigned ind);
void print_bit_big_decimal(s21_big_decimal n);
void null_scale_big_decimal(s21_big_decimal *value);
unsigned get_big_scale(s21_big_decimal value);
void set_scale_big_decimal(s21_big_decimal *value, unsigned scale);

// простые побитовые арифметичсекие операции без учета знака и скейла

int s21_simple_add(s21_big_decimal value_1, s21_big_decimal value_2,
                   s21_big_decimal *result);
void s21_simple_sub(s21_big_decimal a, s21_big_decimal b, s21_big_decimal *res);
void s21_simple_mul(s21_big_decimal value_1, s21_big_decimal value_2,
                    s21_big_decimal *result);
void s21_simple_div(s21_big_decimal dividend, s21_big_decimal divisor,
                    s21_big_decimal *result);

/// доп. функции для работы с big децимал

void s21_init_big_decimal_by_zero(s21_big_decimal *m);
void shift_left_big_decimal(s21_big_decimal *src, unsigned count);
void shift_right_big_decimal(s21_big_decimal *src, unsigned count);
void s21_scale_simple_alignment(s21_big_decimal *value_1,
                                s21_big_decimal *value_2);
void swap_big_decimal(s21_big_decimal *a, s21_big_decimal *b);
void s21_multiply_by_10(s21_big_decimal *result);
void s21_div_by_10(s21_big_decimal *result);
int decrease_scale(s21_big_decimal *result);
unsigned check_big_decimal_equal_zero(s21_big_decimal src);
unsigned check_infinity(s21_big_decimal src);
void get_remainder_big(s21_big_decimal dividend, s21_big_decimal divisor,
                       s21_big_decimal *result, s21_big_decimal *remainder);
void s21_eq_div(s21_big_decimal dividend, s21_big_decimal divisor,
                s21_big_decimal *result);
int s21_get_exp_float(float src);
void s21_float_decimal(float src, s21_decimal *dst);

#endif  // SRC_S21_DECIMAL_H_
