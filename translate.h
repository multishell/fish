/** \file translate.h

Translation library, internally uses catgets

*/

/**
   Shorthand for wgettext call
*/
#define _(wstr) wgettext(wstr)

/**
   Noop, used to tell xgettext that a string should be translated, even though it is not directly sent to wgettext.
*/
#define N_(wstr) wstr

/**
   Wide character wwrapper around the gettext function
*/
const wchar_t *wgettext( const wchar_t *in );

/**
  Initialize (or reinitialize) the translation library
*/
void translate_init();
/**
   Free memory used by the translation library
*/
void translate_destroy();
