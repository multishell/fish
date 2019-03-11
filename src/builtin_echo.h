// Prototypes for executing builtin_echo function.
#ifndef FISH_BUILTIN_ECHO_H
#define FISH_BUILTIN_ECHO_H

class parser_t;
struct io_streams_t;

int builtin_echo(parser_t &parser, io_streams_t &streams, wchar_t **argv);
#endif
