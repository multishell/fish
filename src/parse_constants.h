// Constants used in the programmatic representation of fish code.
#ifndef FISH_PARSE_CONSTANTS_H
#define FISH_PARSE_CONSTANTS_H

#include "common.h"
#include "config.h"

#define PARSE_ASSERT(a) assert(a)
#define PARSER_DIE()                  \
    do {                              \
        debug(0, "Parser dying!");    \
        exit_without_destructors(-1); \
    } while (0)

// IMPORTANT: If the following enum table is modified you must also update token_enum_map below.
enum parse_token_type_t {
    token_type_invalid = 1,
    // Non-terminal tokens
    symbol_job_list,
    symbol_job_conjunction,
    symbol_job_conjunction_continuation,
    symbol_job_decorator,
    symbol_job,
    symbol_job_continuation,
    symbol_statement,
    symbol_block_statement,
    symbol_block_header,
    symbol_for_header,
    symbol_while_header,
    symbol_begin_header,
    symbol_function_header,
    symbol_if_statement,
    symbol_if_clause,
    symbol_else_clause,
    symbol_else_continuation,
    symbol_switch_statement,
    symbol_case_item_list,
    symbol_case_item,
    symbol_not_statement,
    symbol_decorated_statement,
    symbol_plain_statement,
    symbol_arguments_or_redirections_list,
    symbol_andor_job_list,
    symbol_argument_list,
    // Freestanding argument lists are parsed from the argument list supplied to 'complete -a'.
    // They are not generated by parse trees rooted in symbol_job_list.
    symbol_freestanding_argument_list,
    symbol_argument,
    symbol_redirection,
    symbol_optional_background,
    symbol_optional_newlines,
    symbol_end_command,
    // Terminal types.
    parse_token_type_string,
    parse_token_type_pipe,
    parse_token_type_redirection,
    parse_token_type_background,
    parse_token_type_andand,
    parse_token_type_oror,
    parse_token_type_end,
    // Special terminal type that means no more tokens forthcoming.
    parse_token_type_terminate,
    // Very special terminal types that don't appear in the production list.
    parse_special_type_parse_error,
    parse_special_type_tokenizer_error,
    parse_special_type_comment,

    LAST_TOKEN_TYPE = parse_special_type_comment,
    FIRST_TERMINAL_TYPE = parse_token_type_string,
    LAST_TERMINAL_TYPE = parse_token_type_terminate,
    LAST_TOKEN_OR_SYMBOL = parse_token_type_terminate,
    FIRST_PARSE_TOKEN_TYPE = parse_token_type_string,
    LAST_PARSE_TOKEN_TYPE = parse_token_type_end
} __packed;

const enum_map<parse_token_type_t> token_enum_map[] = {
    {parse_special_type_comment, L"parse_special_type_comment"},
    {parse_special_type_parse_error, L"parse_special_type_parse_error"},
    {parse_special_type_tokenizer_error, L"parse_special_type_tokenizer_error"},
    {parse_token_type_background, L"parse_token_type_background"},
    {parse_token_type_end, L"parse_token_type_end"},
    {parse_token_type_pipe, L"parse_token_type_pipe"},
    {parse_token_type_redirection, L"parse_token_type_redirection"},
    {parse_token_type_string, L"parse_token_type_string"},
    {parse_token_type_andand, L"parse_token_type_andand"},
    {parse_token_type_oror, L"parse_token_type_oror"},
    {parse_token_type_terminate, L"parse_token_type_terminate"},
// Define all symbols
#define ELEM(sym) {symbol_##sym, L"symbol_" #sym},
#include "parse_grammar_elements.inc"
    {token_type_invalid, L"token_type_invalid"},
    {token_type_invalid, NULL}};
#define token_enum_map_len (sizeof token_enum_map / sizeof *token_enum_map)

// IMPORTANT: If the following enum is modified you must update the corresponding keyword_enum_map
// array below.
//
// IMPORTANT: These enums must start at zero.
enum parse_keyword_t {
    parse_keyword_none = 0,
    parse_keyword_and,
    parse_keyword_begin,
    parse_keyword_builtin,
    parse_keyword_case,
    parse_keyword_command,
    parse_keyword_else,
    parse_keyword_end,
    parse_keyword_exec,
    parse_keyword_for,
    parse_keyword_function,
    parse_keyword_if,
    parse_keyword_in,
    parse_keyword_not,
    parse_keyword_exclam,
    parse_keyword_or,
    parse_keyword_switch,
    parse_keyword_while,
} __packed;

const enum_map<parse_keyword_t> keyword_enum_map[] = {{parse_keyword_exclam, L"!"},
                                                      {parse_keyword_and, L"and"},
                                                      {parse_keyword_begin, L"begin"},
                                                      {parse_keyword_builtin, L"builtin"},
                                                      {parse_keyword_case, L"case"},
                                                      {parse_keyword_command, L"command"},
                                                      {parse_keyword_else, L"else"},
                                                      {parse_keyword_end, L"end"},
                                                      {parse_keyword_exec, L"exec"},
                                                      {parse_keyword_for, L"for"},
                                                      {parse_keyword_function, L"function"},
                                                      {parse_keyword_if, L"if"},
                                                      {parse_keyword_in, L"in"},
                                                      {parse_keyword_not, L"not"},
                                                      {parse_keyword_or, L"or"},
                                                      {parse_keyword_switch, L"switch"},
                                                      {parse_keyword_while, L"while"},
                                                      {parse_keyword_none, NULL}};
#define keyword_enum_map_len (sizeof keyword_enum_map / sizeof *keyword_enum_map)

// Node tag values.

// Statement decorations, stored in node tag.
enum parse_statement_decoration_t {
    parse_statement_decoration_none,
    parse_statement_decoration_command,
    parse_statement_decoration_builtin,
    parse_statement_decoration_exec
};

// Boolean statement types, stored in node tag.
enum parse_bool_statement_type_t { parse_bool_none, parse_bool_and, parse_bool_or };

// Whether a statement is backgrounded.
enum parse_optional_background_t { parse_no_background, parse_background };

// Parse error code list.
enum parse_error_code_t {
    parse_error_none,

    // Matching values from enum parser_error.
    parse_error_syntax,
    parse_error_eval,
    parse_error_cmdsubst,

    parse_error_generic,  // unclassified error types

    // Tokenizer errors.
    parse_error_tokenizer_unterminated_quote,
    parse_error_tokenizer_unterminated_subshell,
    parse_error_tokenizer_unterminated_slice,
    parse_error_tokenizer_unterminated_escape,
    parse_error_tokenizer_other,

    parse_error_unbalancing_end,   // end outside of block
    parse_error_unbalancing_else,  // else outside of if
    parse_error_unbalancing_case   // case outside of switch
};

enum { PARSER_TEST_ERROR = 1, PARSER_TEST_INCOMPLETE = 2 };
typedef unsigned int parser_test_error_bits_t;

struct parse_error_t {
    /// Text of the error.
    wcstring text;
    /// Code for the error.
    enum parse_error_code_t code;
    /// Offset and length of the token in the source code that triggered this error.
    size_t source_start;
    size_t source_length;
    /// Return a string describing the error, suitable for presentation to the user. If skip_caret
    /// is false, the offending line with a caret is printed as well.
    wcstring describe(const wcstring &src) const;
    /// Return a string describing the error, suitable for presentation to the user, with the given
    /// prefix. If skip_caret is false, the offending line with a caret is printed as well.
    wcstring describe_with_prefix(const wcstring &src, const wcstring &prefix, bool is_interactive,
                                  bool skip_caret) const;
};
typedef std::vector<parse_error_t> parse_error_list_t;

// Special source_start value that means unknown.
#define SOURCE_LOCATION_UNKNOWN (static_cast<size_t>(-1))

/// Helper function to offset error positions by the given amount. This is used when determining
/// errors in a substring of a larger source buffer.
void parse_error_offset_source_start(parse_error_list_t *errors, size_t amt);

/// Maximum number of function calls.
#define FISH_MAX_STACK_DEPTH 128

/// Error message on a function that calls itself immediately.
#define INFINITE_FUNC_RECURSION_ERR_MSG \
    _(L"The function '%ls' calls itself immediately, which would result in an infinite loop.")

/// Error message on reaching maximum call stack depth.
#define CALL_STACK_LIMIT_EXCEEDED_ERR_MSG                                                     \
    _(L"The function call stack limit has been exceeded. Do you have an accidental infinite " \
      L"loop?")

/// Error message when encountering an illegal command name.
#define ILLEGAL_CMD_ERR_MSG _(L"Illegal command name '%ls'")

/// Error message when encountering an unknown builtin name.
#define UNKNOWN_BUILTIN_ERR_MSG _(L"Unknown builtin '%ls'")

/// Error message when encountering a failed expansion, e.g. for the variable name in for loops.
#define FAILED_EXPANSION_VARIABLE_NAME_ERR_MSG _(L"Unable to expand variable name '%ls'")

/// Error message when encountering a failed process expansion, e.g. %notaprocess.
#define FAILED_EXPANSION_PROCESS_ERR_MSG _(L"Unable to find a process '%ls'")

/// Error message when encountering an illegal file descriptor.
#define ILLEGAL_FD_ERR_MSG _(L"Illegal file descriptor in redirection '%ls'")

/// Error message for wildcards with no matches.
#define WILDCARD_ERR_MSG _(L"No matches for wildcard '%ls'. See `help expand`.")

/// Error when using break outside of loop.
#define INVALID_BREAK_ERR_MSG _(L"'break' while not inside of loop")

/// Error when using continue outside of loop.
#define INVALID_CONTINUE_ERR_MSG _(L"'continue' while not inside of loop")

/// Error when using return builtin outside of function definition.
#define INVALID_RETURN_ERR_MSG _(L"'return' outside of function definition")

// Error messages. The number is a reminder of how many format specifiers are contained.

/// Error for $^.
#define ERROR_BAD_VAR_CHAR1 _(L"$%lc is not a valid variable in fish.")

/// Error for ${a}.
#define ERROR_BRACKETED_VARIABLE1 _(L"Variables cannot be bracketed. In fish, please use {$%ls}.")

/// Error for "${a}".
#define ERROR_BRACKETED_VARIABLE_QUOTED1 \
    _(L"Variables cannot be bracketed. In fish, please use \"$%ls\".")

/// Error issued on $?.
#define ERROR_NOT_STATUS _(L"$? is not the exit status. In fish, please use $status.")

/// Error issued on $$.
#define ERROR_NOT_PID _(L"$$ is not the pid. In fish, please use $fish_pid.")

/// Error issued on $#.
#define ERROR_NOT_ARGV_COUNT _(L"$# is not supported. In fish, please use 'count $argv'.")

/// Error issued on $@.
#define ERROR_NOT_ARGV_AT _(L"$@ is not supported. In fish, please use $argv.")

/// Error issued on $(...).
#define ERROR_BAD_VAR_SUBCOMMAND1 _(L"$(...) is not supported. In fish, please use '(%ls)'.")

/// Error issued on $*.
#define ERROR_NOT_ARGV_STAR _(L"$* is not supported. In fish, please use $argv.")

/// Error issued on $.
#define ERROR_NO_VAR_NAME _(L"Expected a variable name after this $.")

/// Error on foo=bar.
#define ERROR_BAD_EQUALS_IN_COMMAND5                                                        \
    _(L"Unsupported use of '='. To run '%ls' with a modified environment, please use 'env " \
      L"%ls=%ls %ls%ls'")

/// Error message for Posix-style assignment: foo=bar.
#define ERROR_BAD_COMMAND_ASSIGN_ERR_MSG \
    _(L"Unsupported use of '='. In fish, please use 'set %ls %ls'.")

#endif
