#ifndef SMARTCALC
#define SMARTCALC

#include <ctype.h>
#include <locale.h>
#include <math.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEN 256

struct stack {
  double value;
  struct stack *next;
};

struct stack_str {
  char *lexeme;
  struct stack_str *next;
};

int is_operator(char lexeme);
int is_function(char lexeme);
int string_handling(const char *str, int *ii, char **str_rpn,
                    struct stack_str **top);
int operation_handling(char *token, struct stack **top);

int get_rpn(const char *str, char *str_rpn);
int calculate(const char *str_rpn, double x, double *result);

void push(struct stack **top, double value);
double pop(struct stack **top);

void push_str(struct stack_str **top, char *lexeme);
void pop_to_str(struct stack_str **top, char **str);
void pop_str(struct stack_str **top);
void critical_error();

#endif  // SMARTCALC