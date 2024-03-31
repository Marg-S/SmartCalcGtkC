#include "smartcalc.h"

double neg21(double value) { return -value; }
double sum21(double value_1, double value_2) { return value_1 + value_2; }
double sub21(double value_1, double value_2) { return value_1 - value_2; }
double mul21(double value_1, double value_2) { return value_1 * value_2; }
double div21(double value_1, double value_2) { return value_1 / value_2; }

int calculate(const char *str, double x, double *result) {
  setlocale(LC_NUMERIC, "C");
  char str_rpn[LEN];
  int error = get_rpn(str, str_rpn);
  if (error) error = 2;
  struct stack *top = NULL;

  char *token = strtok(str_rpn, " ");
  while (token != NULL && error == 0) {
    if (strcmp(token, "x") == 0) {
      push(&top, x);
    } else if (token[strlen(token) - 1] == 'x') {
      push(&top, atof(token) * x);
    } else if (isdigit(token[0]) || token[0] == '.') {
      push(&top, atof(token));
    } else {
      error = operation_handling(token, &top);
    }
    token = strtok(NULL, " ");
  }

  if (top != NULL) {
    *result = pop(&top);
    if (top != NULL) error = 2;
  } else
    error = 2;
  while (top != NULL) pop(&top);

  return error;
}

int operation_handling(char *token, struct stack **top) {
  int error = 0;
  int find = 0;

  struct {
    const char *string;
    double (*operation)(double);
  } op1[] = {{"sin", sin},   {"cos", cos},   {"tan", tan},   {"asin", asin},
             {"acos", acos}, {"atan", atan}, {"sqrt", sqrt}, {"log", log10},
             {"ln", log},    {"!", neg21}};
  int count_op1 = sizeof(op1) / sizeof(op1[0]);

  struct {
    const char *string;
    double (*operation)(double, double);
  } op2[] = {{"+", sum21}, {"-", sub21}, {"*", mul21},
             {"/", div21}, {"^", pow},   {"mod", fmod}};
  int count_op2 = sizeof(op2) / sizeof(op2[0]);

  double operand;
  if (*top != NULL)
    operand = pop(top);
  else
    error = 2;
  if (operand == 0 && strcmp(token, "/") == 0) error = 1;

  for (int i = 0; i < count_op1 && !find && !error; i++) {
    if (strcmp(token, op1[i].string) == 0) {
      push(top, op1[i].operation(operand));
      find++;
    }
  }
  for (int i = 0; i < count_op2 && !find && !error; i++) {
    if (strcmp(token, op2[i].string) == 0) {
      if (*top != NULL)
        push(top, op2[i].operation(pop(top), operand));
      else
        error = 2;
      find++;
    }
  }

  return error;
}

int valid_operation(char *operation) {
  int valid = 0;
  char operations[][5] = {"mod",  "cos",  "sin",  "tan", "acos",
                          "asin", "atan", "sqrt", "ln",  "log",
                          "+",    "-",    "*",    "/",   "^"};

  int count_operations = sizeof(operations) / sizeof(operations[0]);

  for (int i = 0; i < count_operations && valid == 0; i++)
    if (strcmp(operation, operations[i]) == 0) valid++;

  return valid;
}

int is_operator(char lexeme) { return (strchr("+-*/^m", lexeme) != NULL); }

int is_function(char lexeme) { return !is_operator(lexeme); }

int is_left(char lexeme) { return (lexeme != '^'); }

int get_priority(char lexeme) {
  int priority;

  if (lexeme == '(') {
    priority = 0;
  } else if (lexeme == '+' || lexeme == '-') {
    priority = 1;
  } else if (strchr("*/m", lexeme)) {
    priority = 2;
  } else if (lexeme == '^') {
    priority = 3;
  } else if (lexeme == '!') {
    priority = 5;
  } else {
    priority = 4;
  }

  return priority;
}

int string_handling(const char *str, int *i, char **str_rpn,
                    struct stack_str **top) {
  int error = 0;
  char temp_str[5] = {0};

  int k = 0;
  while (isalpha(str[*i])) temp_str[k++] = str[(*i)++];
  if (k == 0) temp_str[k++] = str[(*i)++];
  (*i)--;

  if (!valid_operation(temp_str)) error++;
  if (!error && is_operator(temp_str[0])) {
    int priority = get_priority(temp_str[0]);
    int stack_priority, left, go = 1;
    while (go && (*top) != NULL) {
      stack_priority = get_priority((*top)->lexeme[0]);
      left = is_left((*top)->lexeme[0]);
      if (stack_priority > priority || (stack_priority == priority && left))
        pop_to_str(top, str_rpn);
      else
        go = 0;
    }
  }
  push_str(top, temp_str);

  return error;
}

int get_rpn(const char *str, char *str_rpn) {
  // char *ptr_str_rpn = str_rpn;
  int error = 0;
  struct stack_str *top = NULL;
  memset(str_rpn, 0, LEN);

  for (int i = 0, j = 0; str[i] != '\0' && !error; i++, j = i) {
    int point = 0, symbol_x = 0, symbol_e = 0;
    while (isdigit(str[j]) || str[j] == '.' || str[j] == 'x' || str[j] == 'e') {
      char c = str[j++];
      if (symbol_x || (symbol_e && !isdigit(c)) ||
          (point && !(isdigit(c) || c == 'e')))
        error++;
      if (!error) {
        if (c == '.') point++;
        if (c == 'x') symbol_x++;
        if (c == 'e') symbol_e++;
        *(str_rpn++) = c;
      }
    }
    if (i != j) {
      *(str_rpn++) = ' ';
      i = j;
    }
    if ((i == 0 || str[i - 1] == '(') && strchr("-+", str[i])) {
      if (str[i] == '-') push_str(&top, "!");
    } else if (str[i] == '(') {
      push_str(&top, "(");
    } else if (str[i] == ')') {
      while (top != NULL && top->lexeme[0] != '(') pop_to_str(&top, &str_rpn);
      if (top == NULL) error++;
      if (!error) pop_str(&top);
      if (top != NULL && is_function(top->lexeme[0]))
        pop_to_str(&top, &str_rpn);
    } else if (str[i] != '\0') {
      if (!isspace(str[i])) error = string_handling(str, &i, &str_rpn, &top);
    } else
      i--;
  }
  while (top != NULL) pop_to_str(&top, &str_rpn);

  return error;
}

void push(struct stack **top, double value) {
  struct stack *tmp = (struct stack *)malloc(sizeof(struct stack));
  tmp->next = *top;
  tmp->value = value;
  *top = tmp;
}

double pop(struct stack **top) {
  double value = 0;
  if (*top != NULL) {
    struct stack *tmp = *top;
    value = (*top)->value;
    *top = (*top)->next;
    free(tmp);
  }
  return value;
}

void push_str(struct stack_str **top, char *lexeme) {
  struct stack_str *tmp = (struct stack_str *)malloc(sizeof(struct stack_str));
  tmp->next = *top;
  tmp->lexeme = (char *)malloc(5);
  strcpy(tmp->lexeme, lexeme);
  *top = tmp;
}

void pop_to_str(struct stack_str **top, char **str) {
  if (*top != NULL) {
    int length = strlen((*top)->lexeme);

    struct stack_str *tmp = *top;
    strcpy((*str), (*top)->lexeme);
    *top = (*top)->next;
    free(tmp->lexeme);
    free(tmp);

    *(*str + length) = ' ';
    (*str) += length + 1;
  }
}

void pop_str(struct stack_str **top) {
  if (*top != NULL) {
    struct stack_str *tmp = *top;
    *top = (*top)->next;
    free(tmp->lexeme);
    free(tmp);
  }
}