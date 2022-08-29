#include "s21_decimal.h"

/* получаем бит на указанной позиции*/
unsigned get_bit(s21_decimal src, unsigned ind) {
  unsigned tmp = src.bits[ind / 32] >> ind % 32;
  return tmp & 1 ? 1 : 0;
}

/* обнуляем бит на указанной позиции*/
void null_bit(s21_decimal *src, unsigned ind) {
  unsigned mask = ~(1 << ind % 32);
  src->bits[ind / 32] &= mask;
}

/* устанавливаем бит на указанной позиции*/
void set_bit(s21_decimal *src, unsigned ind) {
  unsigned mask = (1 << ind % 32);
  src->bits[ind / 32] |= mask;
}

/* инвертируем бит на указанной позиции*/
void invert_bit(s21_decimal *src, unsigned ind) {
  unsigned mask = (1 << ind % 32);
  src->bits[ind / 32] ^= mask;
}

/* Печатаем побитово указанный децимал*/
// void print_bit(s21_decimal n) {
//   char str[129];
//   str[128] = '\0';
//   unsigned tmp = 0;
//   for (int i = 127; i >= 0; i--, tmp++) {
//     str[i] = get_bit(n, tmp) + 48;
//   }
//   for (int i = 0; i < 128; i++) {
//     if (!(i % 32) && i) {
//       printf("| ");
//     }
//     printf("%c", str[i]);
//   }
//   printf("\n");
// }

/* Инициализация децимала нулями */
void s21_init_decimal_by_zero(s21_decimal *m) {
  m->bits[LOW] = m->bits[MID] = m->bits[HIGH] = m->bits[SCALE] = 0;
}

/* Побитовый сдвиг децимала влево */
// void shift_left(s21_decimal *src, unsigned count) {
//   while (count--) {
//     src->bits[HIGH] <<= 1;
//     if (get_bit(*src, 63)) {
//       set_bit(src, 64);
//     }
//     src->bits[MID] <<= 1;
//     if (get_bit(*src, 31)) {
//       set_bit(src, 32);
//     }
//     src->bits[LOW] <<= 1;
//   }
// }

/* Побитовый сдвиг децимала вправо */
// void shift_right(s21_decimal *src, unsigned count) {
//   while (count--) {
//     src->bits[LOW] >>= 1;
//     if (get_bit(*src, 32)) {
//       set_bit(src, 31);
//     }
//     src->bits[MID] >>= 1;
//     if (get_bit(*src, 64)) {
//       set_bit(src, 63);
//     }
//     src->bits[HIGH] >>= 1;
//   }
// }

/* Получение скейла */
unsigned get_scale(s21_decimal value) { return value.bits[SCALE] << 1 >> 17; }

/* Обнулить скейл */
void null_scale(s21_decimal *value) { value->bits[SCALE] &= 0x80000000; }

/* Установить скейл */
void set_scale(s21_decimal *value, unsigned scale) {
  null_scale(value);
  value->bits[SCALE] ^= scale << 16;
}

/* Преобразование  в int из децимала
    обработан общий случай, когда децимал помещается в знаковый инт
    0 - OK
    1 - ошибка конвертации
*/
int s21_from_decimal_to_int(s21_decimal src, int *dst) {
  int result = 0;
  int sign = (get_bit(src, 127) ? -1 : 1);
  if (!dst || src.bits[HIGH] || src.bits[MID] ||
      (sign == -1 && src.bits[LOW] > MIN_INT) ||
      (sign == 1 && src.bits[LOW] > MAX_INT)) {
    result = 1;
  } else {
    s21_truncate(src, &src);
    *dst = src.bits[LOW] * sign;
  }
  return result;
}

/* Преобразование из инта в децимал */
int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  int result = 1;
  s21_init_decimal_by_zero(dst);
  if (src < 0) {
    set_bit(dst, 127);
    src = -src;
  }
  dst->bits[LOW] = src;
  result = 0;
  return result;
}

/* Преобразование  в децимал из float
    обработан общий случай, c точностью 6 знаков после точки
    0 - OK
    1 - ошибка конвертации
*/
// int s21_from_float_to_decimal(float src, s21_decimal *dst) {
//   int flag = 0;
//   char str[255];
//   int sign = 0;
//   if (src < 0) sign = 1;
//   float_to_str(src, str);
//   flag = convert_float_to_dec(str, dst);
//   set_scale(dst, 6);
//   if (sign) set_bit(dst, 127);
//   return flag;
// }

/* Преобразование из decimal во float */
// int s21_from_decimal_to_float(s21_decimal src, float *dst) {
//   int flag = 0;
//   double tmp = 0;
//   if (dst) {
//     for (int i = 0; i < 96; i++) {
//       if (get_bit(src, i)) {
//         tmp += pow(2, i);
//       }
//     }
//     if (!get_scale(src)) {
//       for (int i = get_scale(src); i > 0; i--) {
//         tmp /= 10;
//       }
//     }
//     tmp /= 1000000;
//     *dst = (float)tmp;
//     if (get_bit(src, 127)) {
//       *dst *= -1;
//     }
//   } else {
//     flag = 1;
//   }
//   return flag;
// }

/* умножение биг децимала на 10
  (эквивалентно сдвигу влево на 1 и на 3)

 */
void s21_multiply_by_10(s21_big_decimal *result) {
  s21_big_decimal tmp1 = *result;
  s21_big_decimal tmp2 = *result;

  s21_init_big_decimal_by_zero(result);
  shift_left_big_decimal(&tmp1, 1);
  shift_left_big_decimal(&tmp2, 3);
  s21_simple_add(tmp1, tmp2, result);
}

/*  простое выравнивание скейл без дальнейшего анализа
    увеличиваем скейл меньшего децимала
    на каждом шаге меньший децимал домножаем на 10
*/
void s21_scale_simple_alignment(s21_big_decimal *value_1,
                                s21_big_decimal *value_2) {
  unsigned scale1 = get_big_scale(*value_1);
  unsigned scale2 = get_big_scale(*value_2);
  if (scale1 != scale2) {
    unsigned small_scale = 0, big_scale = 0;
    s21_big_decimal *small;

    small_scale = (scale1 < scale2 ? scale1 : scale2);
    big_scale = (scale1 < scale2 ? scale2 : scale1);
    small = (scale1 < scale2 ? value_1 : value_2);

    while (small_scale < big_scale) {
      small_scale++;
      s21_multiply_by_10(small);
    }
    set_scale_big_decimal(small, big_scale);
  }
}

/* Простое побитовое сложение, без учета остальных аргументов*/
int s21_simple_add(s21_big_decimal value_1, s21_big_decimal value_2,
                   s21_big_decimal *result) {
  s21_big_decimal mask, tmp;
  s21_init_big_decimal_by_zero(&mask);
  s21_init_big_decimal_by_zero(&tmp);
  s21_init_big_decimal_by_zero(result);
  result->bits[7] = value_1.bits[7];  // заносим в результат знак и скейл

  for (int i = 0; i < 7; i++) {
    mask.bits[i] = value_1.bits[i] & value_2.bits[i];
    result->bits[i] = value_1.bits[i] ^ value_2.bits[i];
  }
  while (check_big_decimal_equal_zero(mask)) {
    if (get_bit_big_decimal(mask, 223)) {
      return 1;
    }
    shift_left_big_decimal(&mask, 1);
    for (int i = 0; i < 7; i++) {
      tmp.bits[i] = mask.bits[i] & result->bits[i];
      result->bits[i] = mask.bits[i] ^ result->bits[i];
    }
    mask = tmp;
  }

  return 0;
}

/* Деление на 10 с округлением */
void s21_div_by_10(s21_big_decimal *result) {
  unsigned scale = get_big_scale(*result);
  s21_big_decimal five = {{5, 0, 0, 0, 0, 0, 0, 0}};
  s21_big_decimal ten = {{10, 0, 0, 0, 0, 0, 0, 0}};
  // перед понижением скейла округляем
  s21_simple_add(*result, five, result);
  s21_simple_div(*result, ten, result);
  null_scale_big_decimal(result);
  set_scale_big_decimal(result, scale);
}

/* Понижение скейла с округлением */
int decrease_scale(s21_big_decimal *result) {
  unsigned scale = get_big_scale(*result);

  int res = 0;
  s21_big_decimal tmp = *result;
  s21_big_decimal ten = {{10, 0, 0, 0, 0, 0, 0, 0}};
  // пока переполнение понижаем скейл
  while (check_infinity(*result)) {
    if (!scale) {
      res = 1;  // бесконечность (скейл 0, а попрежнему переполнение)
      break;
    }
    // если при делении на 10 переполнение продолжаем делить
    // если нет, то на последней итерации делим на 10 с округлением
    s21_simple_div(tmp, ten, &tmp);
    if (check_infinity(tmp)) {
      s21_simple_div(*result, ten, result);
    } else {
      s21_div_by_10(result);
    }
    scale--;
  }
  // если скейл больше допустимого понижаем
  while (scale > 28) {
    if (scale > 29)
      s21_simple_div(*result, ten, result);
    else
      s21_div_by_10(result);
    scale--;
  }
  // если скейл остался а децимал равен нулю - минус бесконечность
  if (scale && !check_big_decimal_equal_zero(*result)) res = 2;
  set_scale_big_decimal(result, scale);

  return res;
}

/* Сложение двух децималов
    0 - OK
    1 - число слишком велико или равно бесконечности
    2 - число слишком мало или равно отрицательной бесконечности
    3 - деление на 0

  1 Конвертируем в биг децимал
  2 выравниваем скейл
  3 если знаки разные, анализируем итоговый знак и вычитаем
  4 если одинаковые - суммируем
  5 Обратная конвертация
  */
int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  int flag = 0;
  unsigned sign = get_bit(value_1, 127);
  s21_big_decimal v1, v2, res;
  s21_init_big_decimal_by_zero(&res);
  convert_from_decimal_to_big_decimal(value_1, &v1);
  convert_from_decimal_to_big_decimal(value_2, &v2);
  if (get_bit(value_1, 127) != get_bit(value_2, 127)) {
    s21_scale_simple_alignment(&v1, &v2);
    if (s21_is_not_equal_simple(v1, v2)) {
      unsigned final_sign = 0;
      if (sign) {
        if (s21_is_greater_simple(v1, v2)) {
          final_sign = 1;
        } else {
          swap_big_decimal(&v1, &v2);
        }
      } else {
        if (s21_is_less_simple(v1, v2)) {
          final_sign = 1;
          swap_big_decimal(&v1, &v2);
        }
      }
      s21_simple_sub(v1, v2, &res);
      convert_from_big_decimal_to_decimal(res, result);

      if (!check_decimal_equal_zero(*result)) {
        result->bits[3] = 0;
      } else {
        if (final_sign) set_bit(result, 127);
      }
    }
  } else {
    s21_scale_simple_alignment(&v1, &v2);
    s21_simple_add(v1, v2, &res);
    // если переполнение понижаем скейл пока не вместится в 96 битный десимал
    flag = decrease_scale(&res);

    convert_from_big_decimal_to_decimal(res, result);
    if (!check_decimal_equal_zero(*result)) {
      result->bits[3] = 0;
    } else {
      // выставляем итоговый знак в результат
      if (sign)
        set_bit(result, 127);
      else
        null_bit(result, 127);
    }
  }
  return flag;
}

/* простое побитовое вычитание без учета остальных аргументов
   Первый аргумент обязательно больше второго!!!
*/
void s21_simple_sub(s21_big_decimal a, s21_big_decimal b,
                    s21_big_decimal *res) {
  int buffer = 0;
  s21_init_big_decimal_by_zero(res);
  for (int i = 0; i < 224; i++) {
    int bitA = get_bit_big_decimal(a, i);
    int bitB = get_bit_big_decimal(b, i);
    if (!buffer) {
      if (bitA && !bitB) set_bit_big_decimal(res, i);
      if (!bitA && bitB) {
        set_bit_big_decimal(res, i);
        buffer = 1;
      }
    } else {
      if (bitA && !bitB) {
        buffer = 0;
      } else if (!bitA && bitB) {
      } else {
        set_bit_big_decimal(res, i);
      }
    }
  }
  unsigned scale = get_big_scale(a);
  set_scale_big_decimal(res, scale);
}

/* Вычитание двух децималов
    0 - OK
    1 - число слишком велико или равно бесконечности
    2 - число слишком мало или равно отрицательной бесконечности
    3 - деление на 0

  1 Конвертируем в биг децимал
  2 выравниваем скейл
  3 если знаки разные, анализируем итоговый знак и суммируем
  4 если одинаковые - вычитаем
  5 Обратная конвертация
  */
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  int flag = 0;
  unsigned sign = get_bit(value_1, 127);
  s21_init_decimal_by_zero(result);
  if (get_bit(value_1, 127) != get_bit(value_2, 127)) {
    // если знаки разные то итоговый знак будет равен знаку первого аргумента
    // запоминаем его, обнуляем знаки обоих аргументов, складываем
    null_bit(&value_1, 127);
    null_bit(&value_2, 127);
    flag = s21_add(value_1, value_2, result);
    // выставляем итоговый знак в результат
    if (sign) set_bit(result, 127);
  } else {
    s21_big_decimal v1, v2, res;
    s21_init_big_decimal_by_zero(&res);
    convert_from_decimal_to_big_decimal(value_1, &v1);
    convert_from_decimal_to_big_decimal(value_2, &v2);
    s21_scale_simple_alignment(&v1, &v2);
    if (s21_is_not_equal_simple(v1, v2)) {
      if (sign) {
        if (s21_is_less_simple(v1, v2)) {
          sign = 0;
          swap_big_decimal(&v1, &v2);
        }
      } else {
        if (s21_is_less_simple(v1, v2)) {
          sign = 1;
          swap_big_decimal(&v1, &v2);
        }
      }
      s21_simple_sub(v1, v2, &res);
      flag = decrease_scale(&res);
      convert_from_big_decimal_to_decimal(res, result);
      if (!check_decimal_equal_zero(*result)) {
        result->bits[3] = 0;
      } else {
        // выставляем итоговый знак в результат
        if (sign)
          set_bit(result, 127);
        else
          null_bit(result, 127);
      }
    }
  }
  return flag;
}

/* простое побитовое умножение разрядов без учета знаков и скейла */
void s21_simple_mul(s21_big_decimal value_1, s21_big_decimal value_2,
                    s21_big_decimal *result) {
  s21_init_big_decimal_by_zero(result);
  while (check_big_decimal_equal_zero(value_2)) {
    if (get_bit_big_decimal(value_2, 0))
      s21_simple_add(value_1, *result, result);
    shift_right_big_decimal(&value_2, 1);
    shift_left_big_decimal(&value_1, 1);
  }
}

/* Умножение двух децималов
    0 - OK
    1 - число слишком велико или равно бесконечности
    2 - число слишком мало или равно отрицательной бесконечности
    3 - деление на 0

  1 если знаки разные итоговый знак минус, иначе плюс
  2 выравниваем скейл
  3 умножаем / скейл удваивается
  4 если переполнение понижаем скейл. если скейл 0 и переполнение - то
  бесконечность
  5. если скейл >= 28  и результат 0  - минус бесконечность
  5 Обратная конвертация
  */
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  int flag = 0;
  s21_init_decimal_by_zero(result);
  // если оба множителя не равны 0 то умножаем
  if (check_decimal_equal_zero(value_1) && check_decimal_equal_zero(value_2)) {
    unsigned sign = 0;
    if (get_bit(value_1, 127) != get_bit(value_2, 127)) {
      // если знаки разные то итоговый знак минус, иначе плюс
      sign = 1;
    }
    s21_big_decimal v1, v2, res;
    s21_init_big_decimal_by_zero(&res);
    convert_from_decimal_to_big_decimal(value_1, &v1);
    convert_from_decimal_to_big_decimal(value_2, &v2);
    s21_scale_simple_alignment(&v1, &v2);
    unsigned scale = get_big_scale(v1) * 2;  // скейл при умножении удваивается
    s21_simple_mul(v1, v2, &res);
    set_scale_big_decimal(&res,
                          scale);  // записываем в результат удвоенный скейл
    if (check_big_decimal_equal_zero(res)) flag = decrease_scale(&res);
    convert_from_big_decimal_to_decimal(res, result);
    if (!check_decimal_equal_zero(*result)) {
      result->bits[3] = 0;
    } else {
      // выставляем итоговый знак в результат
      if (sign)
        set_bit(result, 127);
      else
        null_bit(result, 127);
    }
  }
  return flag;
}

/* простое побитовое деление абсолютных значений двух децималов */
void s21_simple_div(s21_big_decimal dividend, s21_big_decimal divisor,
                    s21_big_decimal *result) {
  s21_big_decimal dividend_temp;
  s21_init_big_decimal_by_zero(result);
  s21_init_big_decimal_by_zero(&dividend_temp);
  for (int i = 6; i >= 0; i--) {
    for (int j = 0; j < 32; j++) {
      unsigned int bit = dividend.bits[i];
      bit = bit << j >> 31;
      shift_left_big_decimal(&dividend_temp, 1);
      dividend_temp.bits[0] = dividend_temp.bits[0] | bit;
      shift_left_big_decimal(result, 1);
      if (s21_is_greater_or_equal_simple(dividend_temp, divisor)) {
        s21_simple_sub(dividend_temp, divisor, &dividend_temp);

        result->bits[0] |= 0x1;
      }
    }
  }
}

/* получение остатка от простого целочисленного деления */
void get_remainder_big(s21_big_decimal dividend, s21_big_decimal divisor,
                       s21_big_decimal *result, s21_big_decimal *remainder) {
  s21_simple_div(dividend, divisor, result);
  s21_big_decimal dividend_temp;

  s21_init_big_decimal_by_zero(&dividend_temp);

  s21_simple_mul(divisor, *result, &dividend_temp);
  s21_simple_sub(dividend, dividend_temp, remainder);
}

/* точное деление двух децималов */
void s21_eq_div(s21_big_decimal dividend, s21_big_decimal divisor,
                s21_big_decimal *result) {
  int scale_tmp = get_big_scale(dividend) - get_big_scale(divisor);
  if (scale_tmp < 0) {
    while (scale_tmp) {
      s21_multiply_by_10(&dividend);
      scale_tmp++;
    }
  }
  s21_big_decimal remainder;
  s21_init_big_decimal_by_zero(&remainder);
  get_remainder_big(dividend, divisor, result, &remainder);
  // полученый остаток умножаем на 10 пока скейл не станет 29 или
  // остаток не разделится нацело
  while (check_big_decimal_equal_zero(remainder) && scale_tmp < 29) {
    s21_multiply_by_10(&remainder);
    scale_tmp++;
    s21_multiply_by_10(result);
    if (s21_is_greater_or_equal_simple(remainder, divisor)) {
      s21_big_decimal sss;
      s21_init_big_decimal_by_zero(&sss);
      get_remainder_big(remainder, divisor, &sss, &remainder);
      s21_simple_add(*result, sss, result);
    }
  }

  // понижаем искусственно задранный скейл с округлением
  if (scale_tmp == 29) {
    s21_div_by_10(result);
    scale_tmp--;
  }
  set_scale_big_decimal(result, scale_tmp);
}

/* деление двух децималов
    0 - OK
    1 - число слишком велико или равно бесконечности
    2 - число слишком мало или равно отрицательной бесконечности
    3 - деление на 0

  1 если знаки разные итоговый знак минус, иначе плюс
  2 выравниваем скейл
  3 умножаем / скейл удваивается
  4 если переполнение понижаем скейл. если скейл 0 и переполнение - то
  бесконечность
  5. если скейл >= 28  и результат 0  - минус бесконечность
  5 Обратная конвертация
  */
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  int flag = 0;
  unsigned sign = 0;
  if (get_bit(value_1, 127) != get_bit(value_2, 127)) {
    // если знаки разные то итоговый знак минус, иначе плюс
    sign = 1;
  }
  null_bit(&value_1, 127);
  null_bit(&value_1, 127);
  s21_init_decimal_by_zero(result);
  if (!check_decimal_equal_zero(value_2)) {
    flag = 3;
  } else if (!check_decimal_equal_zero(value_1)) {
  } else if (s21_is_equal(value_1, value_2)) {
    s21_decimal one = {{1, 0, 0, 0}};
    // результат 1 если делимое равно делителю.
    *result = one;
    set_scale(result, 0);
  } else {
    s21_big_decimal v1, v2, res;
    s21_init_big_decimal_by_zero(&res);
    convert_from_decimal_to_big_decimal(value_1, &v1);
    convert_from_decimal_to_big_decimal(value_2, &v2);

    s21_eq_div(v1, v2, &res);
    if (check_infinity(res)) {
      flag = decrease_scale(&res);
    }
    convert_from_big_decimal_to_decimal(res, result);

    // выставляем итоговый знак в результат
    if (sign)
      set_bit(result, 127);
    else
      null_bit(result, 127);
  }
  return flag;
}

/* остатокот деления двух децималов
    0 - OK
    1 - число слишком велико или равно бесконечности
    2 - число слишком мало или равно отрицательной бесконечности
    3 - деление на 0

  1 если знаки разные итоговый знак минус, иначе плюс
  2 выравниваем скейл
  3 умножаем / скейл удваивается
  4 если переполнение понижаем скейл. если скейл 0 и переполнение - то
  бесконечность
  5. если скейл >= 28  и результат 0  - минус бесконечность
  5 Обратная конвертация
  */
int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  int flag = 0;
  // остаток от деления будет иметь знак первого аргумента
  unsigned sign = get_bit(value_1, 127);

  s21_big_decimal v1, v2, res;
  s21_init_big_decimal_by_zero(&res);
  convert_from_decimal_to_big_decimal(value_1, &v1);
  convert_from_decimal_to_big_decimal(value_2, &v2);
  s21_scale_simple_alignment(&v1, &v2);
  if (!check_big_decimal_equal_zero(v2)) {
    flag = 3;
  } else if (s21_is_less_simple(v1, v2)) {
    *result = value_1;
    return flag;

  } else if (s21_is_not_equal_simple(v1, v2)) {
    // остаток от деления res = v1 - v1/v2*v2;
    s21_simple_div(v1, v2, &res);
    s21_simple_mul(res, v2, &res);
    s21_simple_sub(v1, res, &res);
    flag = decrease_scale(&res);
  }
  convert_from_big_decimal_to_decimal(res, result);
  if (!check_decimal_equal_zero(*result)) {
    result->bits[3] = 0;
  } else {
    // выставляем итоговый знак в результат
    if (sign)
      set_bit(result, 127);
    else
      null_bit(result, 127);
  }
  return flag;
}

/* получаем бит на указанной позиции*/
unsigned get_bit_big_decimal(s21_big_decimal src, unsigned ind) {
  unsigned tmp = src.bits[ind / 32] >> ind % 32;
  return tmp & 1 ? 1 : 0;
}

/* Конвертация из децимала в биг децимал */
void convert_from_decimal_to_big_decimal(s21_decimal src,
                                         s21_big_decimal *dst) {
  s21_init_big_decimal_by_zero(dst);
  dst->bits[7] = src.bits[3];
  for (int i = 0; i < 3; i++) dst->bits[i] = src.bits[i];
}

/* Конвертация из биг децимала в децимал */
void convert_from_big_decimal_to_decimal(s21_big_decimal src,
                                         s21_decimal *dst) {
  dst->bits[3] = src.bits[7];
  for (int i = 0; i < 3; i++) dst->bits[i] = src.bits[i];
}

/* меняем местами два биг десимала */
void swap_big_decimal(s21_big_decimal *a, s21_big_decimal *b) {
  s21_big_decimal tmp = *a;
  *a = *b;
  *b = tmp;
}

/* проверка равен ли big десимал нулю
  0 - равен нулю
  1 - не равен нулю
*/
unsigned check_big_decimal_equal_zero(s21_big_decimal src) {
  unsigned res = 0;
  for (int i = 0; i < 7; i++) {
    if (src.bits[i]) res = 1;
  }
  return res;
}

/* проверка равен ли десимал нулю
  0 - равен нулю
  1 - не равен нулю
*/
unsigned check_decimal_equal_zero(s21_decimal src) {
  unsigned res = 0;
  for (int i = 0; i < 3; i++) {
    if (src.bits[i]) res = 1;
  }
  return res;
}

/* проверка биг десимала на infinity
  0 - нормальное число
  1 - бесконечность
*/
unsigned check_infinity(s21_big_decimal src) {
  unsigned res = 0;
  for (unsigned i = 3; i < 7; i++) {
    if (src.bits[i]) res = 1;
  }
  return res;
}

/* устанавливаем бит на указанной позиции*/
void set_bit_big_decimal(s21_big_decimal *src, unsigned ind) {
  unsigned mask = (1 << ind % 32);
  src->bits[ind / 32] |= mask;
}

/* Инициализация биг децимала нулями */
void s21_init_big_decimal_by_zero(s21_big_decimal *m) {
  for (int i = 0; i < 8; i++) {
    m->bits[i] = 0;
  }
}

/* Побитовый сдвиг биг децимала влево */
void shift_left_big_decimal(s21_big_decimal *src, unsigned count) {
  while (count--) {
    for (unsigned i = 6; i >= 1; i--) {
      src->bits[i] <<= 1;
      if (get_bit_big_decimal(*src, i * 32 - 1)) {
        set_bit_big_decimal(src, i * 32);
      }
    }
    src->bits[0] <<= 1;
  }
}

/* Побитовый сдвиг биг децимала вправо */
void shift_right_big_decimal(s21_big_decimal *src, unsigned count) {
  while (count--) {
    for (unsigned i = 1; i < 7; i++) {
      src->bits[i - 1] >>= 1;
      if (get_bit_big_decimal(*src, i * 32)) {
        set_bit_big_decimal(src, i * 32 - 1);
      }
    }
    src->bits[6] >>= 1;
  }
}

/* Печатаем побитово указанный биг децимал*/
void print_bit_big_decimal(s21_big_decimal n) {
  char str[257];
  str[256] = '\0';
  unsigned tmp = 0;
  for (int i = 255; i >= 0; i--, tmp++) {
    str[i] = get_bit_big_decimal(n, tmp) + 48;
  }
  for (int i = 0; i < 256; i++) {
    if (!(i % 32) && i) {
      // printf("| ");
    }
    printf("%c", str[i]);
  }
  printf("\n");
}

/* Получение скейла big decimal*/
unsigned get_big_scale(s21_big_decimal value) {
  return value.bits[7] << 1 >> 17;
}

/* Обнулить скейл big decimal */
void null_scale_big_decimal(s21_big_decimal *value) {
  value->bits[7] &= 0x80000000;
}

/* Установить скейл big decimal */
void set_scale_big_decimal(s21_big_decimal *value, unsigned scale) {
  null_scale_big_decimal(value);
  value->bits[7] ^= scale << 16;
}

/*
    ----------------------------------------------------------------------------------

    сравнение с учетом знаков и скейла
*/

/* сравнение двух децималов. Первый меньше второго
  0 - Error
  1 - OK
*/
int s21_is_less(s21_decimal a, s21_decimal b) {
  int res = 0;
  if (get_bit(a, 127) > get_bit(b, 127)) {
    res = 1;
  } else if (get_bit(a, 127) < get_bit(b, 127)) {
  } else {
    unsigned sign = get_bit(a, 127);  // узнаем знак наших децималов
    s21_big_decimal a_big;
    s21_big_decimal b_big;
    convert_from_decimal_to_big_decimal(a, &a_big);
    convert_from_decimal_to_big_decimal(b, &b_big);
    s21_scale_simple_alignment(&a_big, &b_big);
    if (s21_is_less_simple(a_big, b_big)) {
      res = (sign ? 0 : 1);
    } else {
      if (s21_is_not_equal_simple(a_big, b_big)) res = (sign ? 1 : 0);
    }
  }
  return res;
}

/* сравнение двух децималов. Первый равен второму
  0 - Error
  1 - OK
*/
int s21_is_equal(s21_decimal a, s21_decimal b) {
  int res = 1;
  if (get_bit(a, 127) != get_bit(b, 127)) {
    res = 0;
  } else {
    s21_big_decimal a_big;
    s21_big_decimal b_big;
    convert_from_decimal_to_big_decimal(a, &a_big);
    convert_from_decimal_to_big_decimal(b, &b_big);
    s21_scale_simple_alignment(&a_big, &b_big);
    res = (s21_is_equal_simple(a_big, b_big) ? 1 : 0);
  }

  return res;
}

/* сравнение двух децималов. Первый НЕ равен второму
  0 - Error
  1 - OK
*/
int s21_is_not_equal(s21_decimal a, s21_decimal b) {
  return (!s21_is_equal(a, b) ? 1 : 0);
}

/* сравнение двух децималов. Первый меньше либо равен второму
  0 - Error
  1 - OK
*/
int s21_is_less_or_equal(s21_decimal a, s21_decimal b) {
  return (s21_is_less(a, b) || s21_is_equal(a, b));
}

/* сравнение двух децималов. Первый больше второго
  0 - Error
  1 - OK
*/
int s21_is_greater(s21_decimal a, s21_decimal b) {
  return (!s21_is_less_or_equal(a, b) ? 1 : 0);
}

/* сравнение двух децималов. Первый больше либо равен второму
  0 - Error
  1 - OK
*/
int s21_is_greater_or_equal(s21_decimal a, s21_decimal b) {
  return (!s21_is_less(a, b) ? 1 : 0);
}

/*
    ----------------------------------------------------------------------------------------------
    Функции сравнения simple сравнивают абсолютные
    значения без учета скейла и знака.
*/

/* простое сравнение абсолютных значений двух биг децималов. Первый меньше
  второго
  0 - Error
  1 - OK
*/
int s21_is_less_simple(s21_big_decimal a, s21_big_decimal b) {
  int res = 0;

  for (int i = 6; i >= 0; i--) {
    if (a.bits[i] > b.bits[i]) {
      break;
    } else if (a.bits[i] < b.bits[i]) {
      res = 1;
      break;
    }
  }

  return res;
}

/* сравнение двух биг децималов. Первый меньше либо равен второму
  0 - Error
  1 - OK
*/
int s21_is_less_or_equal_simple(s21_big_decimal a, s21_big_decimal b) {
  return (s21_is_less_simple(a, b) || s21_is_equal_simple(a, b) ? 1 : 0);
}

/* сравнение абсолютных значений двух биг децималов. Первый равен второму
  0 - Error
  1 - OK
*/
int s21_is_equal_simple(s21_big_decimal a, s21_big_decimal b) {
  int res = 1;

  for (int i = 0; i < 7; i++) {
    if (a.bits[i] != b.bits[i]) {
      res = 0;
      break;
    }
  }
  return res;
}

/* сравнение двух биг децималов. Первый НЕ равен второму
  0 - Error
  1 - OK
*/
int s21_is_not_equal_simple(s21_big_decimal a, s21_big_decimal b) {
  return (!s21_is_equal_simple(a, b) ? 1 : 0);
}

/* сравнение двух биг децималов. Первый больше второго
  0 - Error
  1 - OK
*/
int s21_is_greater_simple(s21_big_decimal a, s21_big_decimal b) {
  return (!s21_is_less_or_equal_simple(a, b) ? 1 : 0);
}

/* сравнение двух биг децималов. Первый больше либо равен второму
  0 - Error
  1 - OK
*/
int s21_is_greater_or_equal_simple(s21_big_decimal a, s21_big_decimal b) {
  return (!s21_is_less_simple(a, b) ? 1 : 0);
}
/*
  Возвращает целые цифры указанного Decimal числа; любые дробные цифры
  отбрасываются, включая конечные нули.
  0 - OK
  1 - ошибка вычисления
*/
int s21_truncate(s21_decimal value, s21_decimal *result) {
  s21_init_decimal_by_zero(result);
  if (check_decimal_equal_zero(value)) {
    // обнуляем скейл, деля в цикле на 10( без округления)
    unsigned sign = get_bit(value, 127);
    unsigned scale = get_scale(value);
    if (scale) {
      s21_big_decimal res;
      s21_big_decimal ten = {{10, 0, 0, 0, 0, 0, 0, 0}};
      convert_from_decimal_to_big_decimal(value, &res);
      while (scale--) {
        s21_simple_div(res, ten, &res);
      }
      convert_from_big_decimal_to_decimal(res, result);

      result->bits[3] = 0;

      if (sign)
        set_bit(result, 127);
      else
        null_bit(result, 127);
    } else {
      *result = value;
    }
  }
  return 0;
}

/*
  Округляет Decimal до ближайшего целого числа.
  0 - OK
  1 - ошибка вычисления
*/
int s21_round(s21_decimal value, s21_decimal *result) {
  s21_init_decimal_by_zero(result);
  if (check_decimal_equal_zero(value)) {
    unsigned sign = get_bit(value, 127);
    // понижаем скейл до 1, деля в цикле на 10( без округления)
    // на последнем шаге делим на 10 с округлением
    unsigned scale = get_scale(value);
    if (scale) {
      scale -= 1;
      s21_big_decimal res;
      convert_from_decimal_to_big_decimal(value, &res);
      s21_big_decimal ten = {{10, 0, 0, 0, 0, 0, 0, 0}};
      while (scale--) {
        s21_simple_div(res, ten, &res);
      }

      s21_div_by_10(&res);
      convert_from_big_decimal_to_decimal(res, result);
      // обнуляем итоговый скейл и выставляем знак
      result->bits[3] = 0;

      if (sign)
        set_bit(result, 127);
      else
        null_bit(result, 127);
    } else {
      *result = value;
    }
  }
  return 0;
}

/*
    Округляет указанное Decimal число до ближайшего целого числа
    в сторону отрицательной бесконечности
    0 - OK
    1 - ошибка вычисления
*/
int s21_floor(s21_decimal value, s21_decimal *result) {
  s21_init_decimal_by_zero(result);
  if (check_decimal_equal_zero(value)) {
    unsigned sign = get_bit(value, 127);
    unsigned flag = 0;
    // проверяем, были ли в дробной части значимые цифры
    // если да, и знак минус увеличиваем результат на 1
    s21_decimal temp = value;
    s21_truncate(temp, &temp);
    if (s21_is_not_equal(value, temp)) flag = 1;
    s21_truncate(value, result);
    if (sign && flag) {
      s21_decimal one = {{1, 0, 0, 0}};
      s21_sub(*result, one, result);
    }
    if (sign)
      set_bit(result, 127);
    else
      null_bit(result, 127);
  }
  return 0;
}

/* Умножение децимала на -1 / смена знака*/
int s21_negate(s21_decimal value, s21_decimal *result) {
  int res = 1;
  if (result) {
    *result = value;
    invert_bit(result, 127);
    res = 0;
  }
  return res;
}

// узнаем экспоненту float
int s21_get_exp_float(float src) {
  int t = *(int *)&src;
  return ((t >> 23) & 0b11111111) - 127;
}

// операции по конвертации f to dec
void s21_float_decimal(float src, s21_decimal *dst) {
  int flag = 0;
  if (src < 0) {  // проверяем знак
    flag = 1;
    src = -src;
  }
  // int exp = s21_get_exp_float(src); // берем только экспоненту числа
  double tmp = (double)src;
  int degree = 0;
  while (degree < 28 &&
         (int)tmp / (int)powl(2, 21) ==
             0) {  // вычисляем скейл и переносим все число в экспоненту
    tmp *= 10;
    degree++;
  }
  tmp = round(tmp);
  while (fmod(tmp, 10) == 0 && degree > 0) {  // оставляем только значащие цифры
    degree--;
    tmp /= 10;
  }
  float f1 = tmp;  // все число в экспоненте без лишних нулей
  int exp = s21_get_exp_float(f1);  // записывает в инт как хранится наш флот
  if (exp < 96) {
    dst->bits[exp / 32] = 1 << exp % 32;
    for (int i = exp - 1, j = 22; j >= 0; i--, j--) {
      if ((*(int *)&f1 & (0x1 << j)) != 0) dst->bits[i / 32] |= 0x1 << i % 32;
    }
    set_scale(dst, degree);  // записываем в децимал наш скейл
  } else {
    dst->bits[HIGH] = dst->bits[MID] = dst->bits[LOW] = 0xFFFFFFFF;
  }
  if (degree > 28) dst->bits[HIGH] = dst->bits[MID] = dst->bits[LOW] = 0;
  if (flag == 1) set_bit(dst, 127);  // если число с минусом ставим 1 в децимал
}

// проводим проверку и выполняем конвертацию
int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  int res = 1;
  if (dst != NULL) {
    dst->bits[HIGH] = dst->bits[MID] = dst->bits[LOW] = dst->bits[SCALE] = 0;
    if (!(src < 1e-28 && src > -1e-28 && src != 0.0)) {
      if (src != 0.0) {
        s21_float_decimal(src, dst);
      }
      res = 0;
    }
  }
  return res;
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  double result = 0.0;
  int count = 0;
  unsigned sign, scale;
  sign = get_bit(src, 127);
  scale = get_scale(src);
  if (check_decimal_equal_zero(src)) {
    s21_big_decimal ten = {{10, 0, 0, 0, 0, 0, 0, 0}};
    s21_big_decimal tmp;
    s21_big_decimal value;
    s21_decimal sss;

    convert_from_decimal_to_big_decimal(src, &value);
    int number = 0;

    if (s21_is_less_simple(value, ten)) {
      result = src.bits[0];

    } else {
      if (value.bits[1] == 0 && value.bits[2] == 0) {
        result = value.bits[0];
      } else {
        while (check_big_decimal_equal_zero(value)) {
          s21_init_big_decimal_by_zero(&tmp);
          s21_simple_div(value, ten, &tmp);
          s21_simple_mul(tmp, ten, &tmp);
          s21_simple_sub(value, tmp, &tmp);
          convert_from_big_decimal_to_decimal(tmp, &sss);
          s21_from_decimal_to_int(sss, &number);
          result += number * pow(10, count);
          count++;
          s21_simple_div(value, ten, &value);
        }
      }
    }
    result /= pow(10, scale);
  }
  *dst = result;
  if (sign) *dst = -(*dst);
  return 0;
}
