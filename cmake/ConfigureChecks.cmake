# Detect curses.
FIND_PACKAGE(Curses REQUIRED)

# Get threads.
set(THREADS_PREFER_PTHREAD_FLAG ON)
# FindThreads < 3.4.0 doesn't work for C++-only projects
IF(CMAKE_VERSION VERSION_LESS 3.4.0)
    ENABLE_LANGUAGE(C)
ENDIF()
FIND_PACKAGE(Threads REQUIRED)

IF(APPLE)
  # 10.7+ only.
  SET(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS} "-Werror=unguarded-availability")
ENDIF()

# Detect WSL. Does not match against native Windows/WIN32.
if (CMAKE_HOST_SYSTEM_VERSION MATCHES ".*-Microsoft")
  SET(WSL 1)
endif()

# Set up the config.h file.
SET(PACKAGE_NAME "fish")
SET(PACKAGE_TARNAME "fish")
INCLUDE(CheckCXXSymbolExists)
INCLUDE(CheckIncludeFileCXX)
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckStructHasMember)
INCLUDE(CheckCXXSourceCompiles)
INCLUDE(CheckTypeSize)
CHECK_CXX_SYMBOL_EXISTS(backtrace_symbols execinfo.h HAVE_BACKTRACE_SYMBOLS)
CHECK_CXX_SYMBOL_EXISTS(clock_gettime time.h HAVE_CLOCK_GETTIME)
CHECK_CXX_SYMBOL_EXISTS(ctermid_r stdio.h HAVE_CTERMID_R)
CHECK_STRUCT_HAS_MEMBER("struct dirent" d_type dirent.h HAVE_STRUCT_DIRENT_D_TYPE LANGUAGE CXX)
CHECK_CXX_SYMBOL_EXISTS(dirfd "sys/types.h;dirent.h" HAVE_DIRFD)
CHECK_INCLUDE_FILE_CXX(execinfo.h HAVE_EXECINFO_H)
CHECK_CXX_SYMBOL_EXISTS(flock sys/file.h HAVE_FLOCK)
# futimens is new in OS X 10.13 but is a weak symbol.
# Don't assume it exists just because we can link - it may be null.
CHECK_CXX_SYMBOL_EXISTS(futimens sys/stat.h HAVE_FUTIMENS)
CHECK_CXX_SYMBOL_EXISTS(futimes sys/time.h HAVE_FUTIMES)
CHECK_CXX_SYMBOL_EXISTS(getifaddrs ifaddrs.h HAVE_GETIFADDRS)
CHECK_CXX_SYMBOL_EXISTS(getpwent pwd.h HAVE_GETPWENT)
CHECK_CXX_SYMBOL_EXISTS(gettext libintl.h HAVE_GETTEXT)
CHECK_CXX_SYMBOL_EXISTS(killpg "sys/types.h;signal.h" HAVE_KILLPG)
CHECK_CXX_SYMBOL_EXISTS(lrand48_r stdlib.h HAVE_LRAND48_R)
# mkostemp is in stdlib in glibc and FreeBSD, but unistd on macOS
CHECK_CXX_SYMBOL_EXISTS(mkostemp "stdlib.h;unistd.h" HAVE_MKOSTEMP)
SET(HAVE_CURSES_H ${CURSES_HAVE_CURSES_H})
SET(HAVE_NCURSES_CURSES_H ${CURSES_HAVE_NCURSES_CURSES_H})
SET(HAVE_NCURSES_H ${CURSES_HAVE_NCURSES_H})
CHECK_INCLUDE_FILES("curses.h;term.h" HAVE_TERM_H)
CHECK_INCLUDE_FILE_CXX("ncurses/term.h" HAVE_NCURSES_TERM_H)
CHECK_INCLUDE_FILE_CXX(siginfo.h HAVE_SIGINFO_H)
CHECK_INCLUDE_FILE_CXX(spawn.h HAVE_SPAWN_H)
CHECK_CXX_SYMBOL_EXISTS(std::wcscasecmp wchar.h HAVE_STD__WCSCASECMP)
CHECK_CXX_SYMBOL_EXISTS(std::wcsdup wchar.h HAVE_STD__WCSDUP)
CHECK_CXX_SYMBOL_EXISTS(std::wcsncasecmp wchar.h HAVE_STD__WCSNCASECMP)
CHECK_STRUCT_HAS_MEMBER("struct stat" st_ctime_nsec "sys/stat.h" HAVE_STRUCT_STAT_ST_CTIME_NSEC
    LANGUAGE CXX)
CHECK_STRUCT_HAS_MEMBER("struct stat" st_mtimespec.tv_nsec "sys/stat.h"
    HAVE_STRUCT_STAT_ST_MTIMESPEC_TV_NSEC LANGUAGE CXX)
CHECK_STRUCT_HAS_MEMBER("struct stat" st_mtim.tv_nsec "sys/stat.h" HAVE_STRUCT_STAT_ST_MTIM_TV_NSEC
    LANGUAGE CXX)
CHECK_CXX_SYMBOL_EXISTS(sys_errlist stdio.h HAVE_SYS_ERRLIST)
CHECK_INCLUDE_FILE_CXX(sys/ioctl.h HAVE_SYS_IOCTL_H)
CHECK_INCLUDE_FILE_CXX(sys/select.h HAVE_SYS_SELECT_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/sysctl.h" HAVE_SYS_SYSCTL_H)
CHECK_INCLUDE_FILE_CXX(termios.h HAVE_TERMIOS_H) # Needed for TIOCGWINSZ
CHECK_CXX_SYMBOL_EXISTS(wcscasecmp wchar.h HAVE_WCSCASECMP)
CHECK_CXX_SYMBOL_EXISTS(wcsdup wchar.h HAVE_WCSDUP)
CHECK_CXX_SYMBOL_EXISTS(wcslcpy wchar.h HAVE_WCSLCPY)
CHECK_CXX_SYMBOL_EXISTS(wcsncasecmp wchar.h HAVE_WCSNCASECMP)
CHECK_CXX_SYMBOL_EXISTS(wcsndup wchar.h HAVE_WCSNDUP)
CHECK_CXX_SYMBOL_EXISTS(wcstod_l wchar.h HAVE_WCSTOD_L)

CHECK_CXX_SYMBOL_EXISTS(_sys_errs stdlib.h HAVE__SYS__ERRS)

SET(CMAKE_EXTRA_INCLUDE_FILES termios.h sys/ioctl.h)
CHECK_TYPE_SIZE("struct winsize" STRUCT_WINSIZE LANGUAGE CXX)
CHECK_CXX_SYMBOL_EXISTS("TIOCGWINSZ" "termios.h;sys/ioctl.h" HAVE_TIOCGWINSZ)
IF(STRUCT_WINSIZE GREATER -1 AND HAVE_TIOCGWINSZ EQUAL 1)
  SET(HAVE_WINSIZE 1)
ENDIF()
SET(CMAKE_EXTRA_INCLUDE_FILES)

IF(EXISTS "/proc/self/stat")
  SET(HAVE__PROC_SELF_STAT 1)
ENDIF()
CHECK_TYPE_SIZE("wchar_t[8]" WCHAR_T_BITS LANGUAGE CXX)

# Solaris, NetBSD and X/Open-conforming systems have a fixed-args tparm
SET(TPARM_INCLUDES)
IF(HAVE_NCURSES_H)
  SET(TPARM_INCLUDES "${TPARM_INCLUDES}#include <ncurses.h>\n")
ELSEIF(HAVE_NCURSES_CURSES_H)
  SET(TPARM_INCLUDES "${TPARM_INCLUDES}#include <ncurses/curses.h>\n")
ELSE()
  SET(TPARM_INCLUDES "${TPARM_INCLUDES}#include <curses.h>\n")
ENDIF()

IF(HAVE_TERM_H)
  SET(TPARM_INCLUDES "${TPARM_INCLUDES}#include <term.h>\n")
ELSEIF(HAVE_NCURSES_TERM_H)
  SET(TPARM_INCLUDES "${TPARM_INCLUDES}#include <ncurses/term.h>\n")
ENDIF()

SET(CMAKE_REQUIRED_LIBRARIES ${CURSES_LIBRARY})
CHECK_CXX_SOURCE_COMPILES("
${TPARM_INCLUDES}

int main () {
  tparm( \"\" );
}
"
  TPARM_TAKES_VARARGS
)
SET(CMAKE_REQUIRED_LIBRARIES)
IF(NOT TPARM_TAKES_VARARGS)
  SET(TPARM_SOLARIS_KLUDGE 1)
ENDIF()

CHECK_CXX_SOURCE_COMPILES("
#include <memory>

int main () {
  std::unique_ptr<int> foo = std::make_unique<int>();
}
"
  HAVE_STD__MAKE_UNIQUE
)

FIND_PROGRAM(SED sed)
