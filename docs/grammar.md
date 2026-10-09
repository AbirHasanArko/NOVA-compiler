# NOVA Formal Grammar Specification

This document defines the formal Context-Free Grammar (CFG) for the NOVA programming language, formatted for translation into **Flex** (lexer) and **Bison** (LALR(1) parser).

---

## 1. Terminal Symbols (Tokens)

### 1.1 Keywords
```text
KW_IGNITE     ::= "ignite"
KW_PROC       ::= "proc"
KW_RET        ::= "ret"
KW_CONST      ::= "const"
KW_FLUX       ::= "flux"
KW_DO         ::= "do"
KW_END        ::= "end"

KW_I32        ::= "i32"
KW_F32        ::= "f32"
KW_BOOL       ::= "bool"
KW_CHAR       ::= "char"
KW_STR        ::= "str"
KW_VOID       ::= "void"

KW_IF         ::= "if"
KW_ELIF       ::= "elif"
KW_ELSE       ::= "else"
KW_ROUTE      ::= "route"
KW_CASE       ::= "case"
KW_DEFAULT    ::= "default"

KW_ORBIT      ::= "orbit"
KW_IN         ::= "in"
KW_STEP       ::= "step"
KW_CRUISE     ::= "cruise"
KW_HALT       ::= "halt"
KW_SKIP       ::= "skip"

KW_TRANSMIT   ::= "transmit"
KW_RECEIVE    ::= "receive"
KW_VERIFY     ::= "verify"
KW_REQUIRE    ::= "require"

KW_AND        ::= "and"
KW_OR         ::= "or"
KW_NOT        ::= "not"
KW_TRUE       ::= "true"
KW_FALSE      ::= "false"
```

### 1.2 Operators and Punctuators
```text
OP_ADD        ::= "+"
OP_SUB        ::= "-"
OP_MUL        ::= "*"
OP_DIV        ::= "/"
OP_MOD        ::= "%"
OP_POW        ::= "**"

OP_LT         ::= "<"
OP_LE         ::= "<="
OP_GT         ::= ">"
OP_GE         ::= ">="
OP_EQ         ::= "=="
OP_NEQ        ::= "!="
OP_ASSIGN     ::= "="

DOTDOT        ::= ".."
ARROW         ::= "->"
COLON         ::= ":"
SEMICOLON     ::= ";"
COMMA         ::= ","
LPAREN        ::= "("
RPAREN        ::= ")"
```

### 1.3 Literals & Identifiers
```text
IDENT         ::= [a-zA-Z_][a-zA-Z0-9_]*
INT_LIT       ::= [0-9]+
FLOAT_LIT     ::= [0-9]+\.[0-9]+
CHAR_LIT      ::= '[^'\\]' | '\\[ntr\\'0]'
STR_LIT       ::= \"([^"\\]|\\.)*\"
```

---

## 2. Operator Precedence and Associativity

Precedence rules declared from lowest to highest:

```bison
%right OP_ASSIGN
%left  KW_OR
%left  KW_AND
%left  OP_EQ OP_NEQ
%left  OP_LT OP_LE OP_GT OP_GE
%left  OP_ADD OP_SUB
%left  OP_MUL OP_DIV OP_MOD
%right UMINUS KW_NOT
%right OP_POW
%left  LPAREN
```

---

## 3. Production Rules (BNF)

### 3.1 Top-Level Program Structure
```bnf
program
    ::= proc_decl_list ignite_block
    | ignite_block
    ;

proc_decl_list
    ::= proc_decl_list proc_decl
    | proc_decl
    ;

ignite_block
    ::= KW_IGNITE KW_DO stmt_list KW_END
    ;
```

### 3.2 Procedures (`proc`)
```bnf
proc_decl
    ::= KW_PROC IDENT LPAREN opt_param_list RPAREN opt_return_type KW_DO opt_require_list stmt_list KW_END
    ;

opt_param_list
    ::= param_list
    | /* empty */
    ;

param_list
    ::= param_list COMMA param
    | param
    ;

param
    ::= IDENT COLON type
    ;

opt_return_type
    ::= ARROW type
    | /* empty */
    ;

opt_require_list
    ::= opt_require_list require_stmt
    | /* empty */
    ;

require_stmt
    ::= KW_REQUIRE expr SEMICOLON
    ;
```

### 3.3 Types
```bnf
type
    ::= KW_I32
    | KW_F32
    | KW_BOOL
    | KW_CHAR
    | KW_STR
    | KW_VOID
    ;
```

### 3.4 Statements
```bnf
stmt_list
    ::= stmt_list stmt
    | /* empty */
    ;

stmt
    ::= declaration_stmt SEMICOLON
    | assignment_stmt SEMICOLON
    | expr_stmt SEMICOLON
    | if_stmt
    | cruise_stmt
    | orbit_stmt
    | route_stmt
    | transmit_stmt SEMICOLON
    | receive_stmt SEMICOLON
    | verify_stmt SEMICOLON
    | ret_stmt SEMICOLON
    | halt_stmt SEMICOLON
    | skip_stmt SEMICOLON
    ;

declaration_stmt
    ::= KW_CONST IDENT COLON type OP_ASSIGN expr
    | KW_FLUX IDENT COLON type
    | KW_FLUX IDENT COLON type OP_ASSIGN expr
    ;

assignment_stmt
    ::= IDENT OP_ASSIGN expr
    ;

expr_stmt
    ::= expr
    ;

transmit_stmt
    ::= KW_TRANSMIT LPAREN expr RPAREN
    ;

receive_stmt
    ::= KW_RECEIVE LPAREN IDENT RPAREN
    ;

verify_stmt
    ::= KW_VERIFY expr
    ;

ret_stmt
    ::= KW_RET expr
    | KW_RET
    ;

halt_stmt
    ::= KW_HALT
    ;

skip_stmt
    ::= KW_SKIP
    ;
```

### 3.5 Control Flow Statements
```bnf
if_stmt
    ::= KW_IF expr KW_DO stmt_list elif_list opt_else KW_END
    ;

elif_list
    ::= elif_list KW_ELIF expr KW_DO stmt_list
    | /* empty */
    ;

opt_else
    ::= KW_ELSE KW_DO stmt_list
    | /* empty */
    ;

cruise_stmt
    ::= KW_CRUISE expr KW_DO stmt_list KW_END
    ;

orbit_stmt
    ::= KW_ORBIT IDENT KW_IN expr DOTDOT expr opt_step KW_DO stmt_list KW_END
    ;

opt_step
    ::= KW_STEP expr
    | /* empty */
    ;

route_stmt
    ::= KW_ROUTE expr KW_DO case_list opt_default KW_END
    ;

case_list
    ::= case_list case_clause
    | case_clause
    ;

case_clause
    ::= KW_CASE literal COLON stmt_list
    ;

opt_default
    ::= KW_DEFAULT COLON stmt_list
    | /* empty */
    ;
```

### 3.6 Expressions
```bnf
expr
    ::= primary_expr
    | expr OP_POW expr
    | OP_SUB expr %prec UMINUS
    | KW_NOT expr
    | expr OP_MUL expr
    | expr OP_DIV expr
    | expr OP_MOD expr
    | expr OP_ADD expr
    | expr OP_SUB expr
    | expr OP_LT expr
    | expr OP_LE expr
    | expr OP_GT expr
    | expr OP_GE expr
    | expr OP_EQ expr
    | expr OP_NEQ expr
    | expr KW_AND expr
    | expr KW_OR expr
    | proc_call
    ;

primary_expr
    ::= IDENT
    | literal
    | LPAREN expr RPAREN
    ;

proc_call
    ::= IDENT LPAREN opt_arg_list RPAREN
    ;

opt_arg_list
    ::= arg_list
    | /* empty */
    ;

arg_list
    ::= arg_list COMMA expr
    | expr
    ;

literal
    ::= INT_LIT
    | FLOAT_LIT
    | CHAR_LIT
    | STR_LIT
    | KW_TRUE
    | KW_FALSE
    ;
```
