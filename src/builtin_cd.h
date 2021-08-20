// Prototypes for executing builtin_cd function.
#ifndef FISH_BUILTIN_CD_H
#define FISH_BUILTIN_CD_H

#include "maybe.h"

class parser_t;
struct io_streams_t;

maybe_t<int> builtin_cd(parser_t &parser, io_streams_t &streams, wchar_t **argv);
#endif
