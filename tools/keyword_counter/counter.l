%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Structure to track keyword frequencies */
typedef struct {
    const char* name;
    int count;
} KeywordEntry;

static KeywordEntry g_keywords[] = {
    {"and", 0},
    {"bool", 0},
    {"case", 0},
    {"char", 0},
    {"const", 0},
    {"cruise", 0},
    {"default", 0},
    {"detect", 0},
    {"do", 0},
    {"end", 0},
    {"f32", 0},
    {"fallback", 0},
    {"false", 0},
    {"flux", 0},
    {"halt", 0},
    {"i32", 0},
    {"ignite", 0},
    {"in", 0},
    {"not", 0},
    {"orbit", 0},
    {"or", 0},
    {"proc", 0},
    {"receive", 0},
    {"redetect", 0},
    {"require", 0},
    {"ret", 0},
    {"route", 0},
    {"skip", 0},
    {"step", 0},
    {"str", 0},
    {"transmit", 0},
    {"true", 0},
    {"verify", 0},
    {"void", 0}
};

static const int NUM_KEYWORDS = sizeof(g_keywords) / sizeof(g_keywords[0]);

static void record_keyword(const char* name) {
    for (int i = 0; i < NUM_KEYWORDS; ++i) {
        if (strcmp(g_keywords[i].name, name) == 0) {
            g_keywords[i].count++;
            return;
        }
    }
}
%}

%option noyywrap
%option nounput
%option noinput

%x MULTI_COMMENT

%%

    /* Skip single-line comments */
"//"[^\n]*          { /* ignore */ }

    /* Skip multi-line comments */
"/*"                { BEGIN(MULTI_COMMENT); }
<MULTI_COMMENT>"*/" { BEGIN(INITIAL); }
<MULTI_COMMENT>\n   { /* ignore */ }
<MULTI_COMMENT>.    { /* ignore */ }

    /* Skip string and char literals so words inside literals are not counted */
\"([^"\\\n]|\\.)*\" { /* ignore */ }
'([^'\\\n]|\\.)'    { /* ignore */ }

    /* NOVA Reserved Keywords */
"ignite"            { record_keyword("ignite"); }
"proc"              { record_keyword("proc"); }
"ret"               { record_keyword("ret"); }
"const"             { record_keyword("const"); }
"flux"              { record_keyword("flux"); }
"do"                { record_keyword("do"); }
"end"               { record_keyword("end"); }

"i32"               { record_keyword("i32"); }
"f32"               { record_keyword("f32"); }
"bool"              { record_keyword("bool"); }
"char"              { record_keyword("char"); }
"str"               { record_keyword("str"); }
"void"              { record_keyword("void"); }

"detect"            { record_keyword("detect"); }
"redetect"          { record_keyword("redetect"); }
"fallback"          { record_keyword("fallback"); }
"route"             { record_keyword("route"); }
"case"              { record_keyword("case"); }
"default"           { record_keyword("default"); }

"orbit"             { record_keyword("orbit"); }
"in"                { record_keyword("in"); }
"step"              { record_keyword("step"); }
"cruise"            { record_keyword("cruise"); }
"halt"              { record_keyword("halt"); }
"skip"              { record_keyword("skip"); }

"transmit"          { record_keyword("transmit"); }
"receive"           { record_keyword("receive"); }
"verify"            { record_keyword("verify"); }
"require"           { record_keyword("require"); }

"and"               { record_keyword("and"); }
"or"                { record_keyword("or"); }
"not"               { record_keyword("not"); }
"true"              { record_keyword("true"); }
"false"             { record_keyword("false"); }

    /* Ignore identifiers, numbers, operators, whitespace */
[a-zA-Z_][a-zA-Z0-9_]* { /* regular identifier */ }
[0-9]+(\.[0-9]+)?      { /* numbers */ }
[ \t\r\n]+             { /* whitespace */ }
.                      { /* other tokens */ }

%%

int main(int argc, char* argv[]) {
    const char* input_path = (argc > 1) ? argv[1] : "examples/demo.nova";
    const char* output_path = (argc > 2) ? argv[2] : "keyword_counts.txt";

    FILE* in = fopen(input_path, "r");
    if (!in) {
        fprintf(stderr, "error: cannot open input file '%s'\n", input_path);
        return 1;
    }

    yyin = in;
    yylex();
    fclose(in);

    FILE* out = fopen(output_path, "w");
    if (!out) {
        fprintf(stderr, "error: cannot open output file '%s' for writing\n", output_path);
        return 1;
    }

    int total_occurrences = 0;
    int unique_detected = 0;

    for (int i = 0; i < NUM_KEYWORDS; ++i) {
        if (g_keywords[i].count > 0) {
            unique_detected++;
            total_occurrences += g_keywords[i].count;
        }
    }

    /* Write formatted frequency report to text file */
    fprintf(out, "==================================================\n");
    fprintf(out, "           NOVA KEYWORD FREQUENCY REPORT          \n");
    fprintf(out, "==================================================\n");
    fprintf(out, "Source File   : %s\n", input_path);
    fprintf(out, "Unique Found  : %d / %d keywords\n", unique_detected, NUM_KEYWORDS);
    fprintf(out, "Total Occurred: %d times\n", total_occurrences);
    fprintf(out, "--------------------------------------------------\n");
    fprintf(out, "%-16s %s\n", "KEYWORD", "COUNT");
    fprintf(out, "--------------------------------------------------\n");

    for (int i = 0; i < NUM_KEYWORDS; ++i) {
        fprintf(out, "%-16s %d\n", g_keywords[i].name, g_keywords[i].count);
    }
    fprintf(out, "==================================================\n");
    fclose(out);

    /* Also print summary to stdout */
    printf("Successfully analyzed '%s':\n", input_path);
    printf("  Unique keywords detected: %d / %d\n", unique_detected, NUM_KEYWORDS);
    printf("  Total keyword tokens    : %d\n", total_occurrences);
    printf("  Output written to       : %s\n", output_path);

    return 0;
}
