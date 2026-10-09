# NOVA Formal Grammar Specification

This document defines the formal Context-Free Grammar (CFG) for the NOVA programming language, formatted for translation into **Flex** (lexer) and **Bison** (LALR(1) parser).

---

## 1. Terminal Symbols (Tokens)

### 1.1 Keywords
```text
KW_ENTRY    ::= "entry"
KW_FN       ::= "fn"
KW_RET      ::= "ret"
KW_VAL      ::= "val"
KW_VAR      ::= "var"
KW_DO       ::= "do"
KW_END      ::= "end"

KW_I32      ::= "i32"
KW_F32      ::= "f32"
KW_BOOL     ::= "bool"
KW_CHAR     ::= "char"
KW_STR      ::= "str"
KW_VOID     ::= "void"

KW_IF       ::= "if"
KW_ELIF     ::= "elif"
KW_ELSE     ::= "else"
KW_SELECT   ::= "select"
KW_CASE     ::= "case"
KW_DEFAULT  ::= "default"

KW_LOOP     ::= "loop"
KW_IN       ::= "in"
KW_STEP     ::= "step"
KW_WHILE    ::= "while"
KW_BREAK    ::= "break"
KW_NEXT     ::= "next"

KW_EMIT     ::= "emit"
KW_READ     ::= "read"
KW_ASSERT   ::= "assert"
KW_REQUIRE  ::= "require"

KW_AND      ::= "and"
KW_OR       ::= "or"
KW_NOT      ::= "not"
KW_TRUE     ::= "true"
KW_FALSE    ::= "false"
```

### 1.2 Operators and Punctuators
```text
OP_ADD      ::= "+"
OP_SUB      ::= "-"
OP_MUL      ::= "*"
OP_DIV      ::= "/"
OP_MOD      ::= "%"
OP_POW      ::= "**"

OP_LT       ::= "<"
OP_LE       ::= "<="
OP_GT       ::= ">"
OP_GE       ::= ">="
OP_EQ       ::= "=="
OP_NEQ      ::= "!="
OP_ASSIGN   ::= "="

DOTDOT      ::= ".."
ARROW       ::= "->"
COLON       ::= ":"
SEMICOLON   ::= ";"
COMMA       ::= ","
LPAREN      ::= "("
RPAREN      ::= ")"
```

### 1.3 Literals & Identifiers
```text
IDENT       ::= [a-zA-Z_][a-zA-Z0-9_]*
INT_LIT     ::= [0-9]+
FLOAT_LIT   ::= [0-9]+\.[0-9]+
CHAR_LIT    ::= '[^'\\]' | '\\[ntr\\'0]'
STR_LIT     ::= \"([^"\\]|\\.)*\"
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
    ::= function_decl_list entry_block
    | entry_block
    ;

function_decl_list
    ::= function_decl_list function_decl
    | function_decl
    ;

entry_block
    ::= KW_ENTRY KW_DO stmt_list KW_END
    ;
```

### 3.2 Functions
```bnf
function_decl
    ::= KW_FN IDENT LPAREN opt_param_list RPAREN opt_return_type KW_DO opt_require_list stmt_list KW_END
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
    | while_stmt
    | loop_stmt
    | select_stmt
    | emit_stmt SEMICOLON
    | read_stmt SEMICOLON
    | assert_stmt SEMICOLON
    | ret_stmt SEMICOLON
    | break_stmt SEMICOLON
    | next_stmt SEMICOLON
    ;

declaration_stmt
    ::= KW_VAL IDENT COLON type OP_ASSIGN expr
    | KW_VAR IDENT COLON type
    | KW_VAR IDENT COLON type OP_ASSIGN expr
    ;

assignment_stmt
    ::= IDENT OP_ASSIGN expr
    ;

expr_stmt
    ::= expr
    ;

emit_stmt
    ::= KW_EMIT LPAREN expr RPAREN
    ;

read_stmt
    ::= KW_READ LPAREN IDENT RPAREN
    ;

assert_stmt
    ::= KW_ASSERT expr
    ;

ret_stmt
    ::= KW_RET expr
    | KW_RET
    ;

break_stmt
    ::= KW_BREAK
    ;

next_stmt
    ::= KW_NEXT
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

while_stmt
    ::= KW_WHILE expr KW_DO stmt_list KW_END
    ;

loop_stmt
    ::= KW_LOOP IDENT KW_IN expr DOTDOT expr opt_step KW_DO stmt_list KW_END
    ;

opt_step
    ::= KW_STEP expr
    | /* empty */
    ;

select_stmt
    ::= KW_SELECT expr KW_DO case_list opt_default KW_END
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
    | function_call
    ;

primary_expr
    ::= IDENT
    | literal
    | LPAREN expr RPAREN
    ;

function_call
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

---

## 4. Conflict Resolution Analysis

1. **Dangling Else**: Eliminated. Because all `if`, `elif`, and `else` blocks end with explicit `end`, the grammar provides unambiguous nesting with zero shift/reduce conflicts.
2. **Statement Termination**: Statements with inner blocks (`if`, `while`, `loop`, `select`) terminate cleanly with `end`. Flat single-line statements terminate with `;`. This prevents lookahead ambiguity.
3. **Expression vs Statement**: Function calls and assignments are strictly distinguished via `IDENT` followed by either `OP_ASSIGN` or `LPAREN`.
