#ifndef _PARSER_H_
#define _PARSER_H_

#include "parser/lexer.h"
#include "baas/hashtbl.h"

/* precedence relation between two operators */
typedef enum {
  LT = -1,
  EQ = 0,
  GT = 1,
  E0, /* internal error */
  E1, /* non-variable assignment */
  E2, /* wrong argument type for operator */
  E3, /* expected binary operator or eol */
  E4, /* unbalanced open parenthesis */
  E5, /* comma only allowed between function arguments */
  E6, /* unbalanced closing parenthesis */
  E7  /* empty buffer and stack, end of parsing */
} op_prec_t;

/* return the precedence relation of two operators */
typedef op_prec_t (*precedencefn)(lexcomp_t op1, lexcomp_t op2);
/* an implementation of the precedence evaluation */
op_prec_t math_precedence(lexcomp_t op1, lexcomp_t op2);

typedef enum {
  stNumber,
  stVariable,
} symtype_t;

typedef union _symbol_t {
  symtype_t t;
  union {
    double dVal;
    char *sVal
  };
} symbol_t;

symbol_t * symbol_number(double d);
symbol_t * symbol_variable(char *varname);
void symbol_destroy(symbol_t *s);

typedef struct _parser_t {
  hashtbl_t *symbol_table;
  precedencefn p;
  list_t *stack;
  list_t *partial;
} parser_t;


parser_t *parser_create(precedencefn p);
void parser_destroy(parser_t *p);

expresion_t * parser_parse(parser_t *p, lexer_t *l);
double * parser_eval(parser_t *p, expresion_t *e);

#endif /*  _PARSER_H_ */

/* vim: set sw=2 sts=2 : */
