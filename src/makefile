.PHONY: all clean test s21_decimal.a check gcov_report valgrind

CC = gcc
CFLAGS = -Wall -Wextra -Werror
PROFILE_FLAGS = -fprofile-arcs -ftest-coverage
OS := $(shell uname -s)

ifeq ($(OS), Darwin)
	CC += -D OS_MAC
else
	CC += -D OS_LINUX
endif
default: all

all: clean s21_decimal.a test gcov_report

test: s21_decimal.a
ifeq ($(OS), Darwin)
	$(CC) $(CFLAGS) -o test unit_tests.c s21_decimal.a -lcheck -coverage
	./test
else
	$(CC) $(CFLAGS) -o test unit_tests.c s21_decimal.a -lcheck -lsubunit -lrt -lpthread -lm -coverage
	./test
endif

s21_decimal.a: s21_decimal.o
	ar rcs s21_decimal.a s21_decimal.o
	ranlib s21_decimal.a

s21_decimal.o:  s21_decimal.c s21_decimal.h
	$(CC) -c s21_decimal.c

clean:
	-rm -rf test s21_decimal *.out *.o *.a *.cfg fizz *.gc* *.info report unit_tests RESULT_VALGRIND.txt

rebuild: clean all

check:
	cppcheck *.h *.c
	cp ../materials/linters/CPPLINT.cfg CPPLINT.cfg
	python3 ../materials/linters/cpplint.py --extension=c *.c *.h
	rm CPPLINT.cfg

valgrind:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=RESULT_VALGRIND.txt ./test

gcov_report:
ifeq ($(OS), Darwin)
	$(CC) --coverage $(PROFILE_FLAGS) unit_tests.c s21_decimal.c -o test -lcheck
else 
	$(CC) --coverage $(PROFILE_FLAGS) unit_tests.c s21_decimal.c -o test -lcheck -lsubunit -lrt -lpthread -lm
endif
	./test
	lcov -t "test" -o test.info -c -d ./
	genhtml -o report test.info
ifeq ($(OS), Darwin)
	open ./report/index-sort-f.html
else
	xdg-open ./report/index-sort-f.html
endif
	rm -rf ./*.gcno ./*.gcda ./GcovReport.* 
