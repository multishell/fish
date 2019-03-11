# TODO: Remove the `begin;...;end` brackets since they're not necessary. A `set -l` at file level
# creates a var local to the file which won't be visible elsewhere. I'm not doing so as part of
# fixing issue #3856 because I don't want to take ownership of every line in this script.
#
# TODO: Whether the expensive operations
# done by the module detection really needs to be done every time the completion is invoked is
# unclear. See issue #3856.
begin
    set -l unicode 'commandline | string match -qr -- "-[a-zA-Z]*C[a-zA-Z]*\$"'
    set -l noopt 'commandline | not string match -qr -- "-[a-zA-Z]*C[a-zA-Z]*\$"'
    set -l modules "(find (perl -lE'print for @INC') -name '*.pm' -printf '%P\n' ^/dev/null \
                        | sed -e 's,/,::,g; s,\.pm\$,,' | sort -u)"
    complete -c perl -s 0 -n $noopt --description 'Specify record separator'
    complete -c perl -s a -n $noopt --description 'Turn on autosplit mode'
    complete -c perl -s c -n $noopt --description 'Check syntax'
    complete -c perl -s C -n $noopt --description 'Control Unicode features'
    complete -c perl -s Ca -n $unicode --description 'Debug UTF-8 cache'
    complete -c perl -s CA -n $unicode --description 'ARGV uses UTF-8'
    complete -c perl -s CD -n $unicode --description 'Opened filehandles are UTF-8'
    complete -c perl -s CE -n $unicode --description 'STDERR is UTF-8'
    complete -c perl -s Ci -n $unicode --description 'Filehandles that are read are UTF-8'
    complete -c perl -s CI -n $unicode --description 'STDIN is UTF-8'
    complete -c perl -s CL -n $unicode --description 'Enable Unicode conditionally'
    complete -c perl -s Co -n $unicode --description 'Filehandles written to are UTF-8'
    complete -c perl -s CO -n $unicode --description 'STDOUT is UTF-8'
    complete -c perl -s CS -n $unicode --description 'STDOUT, STDIN, and STDERR are UTF-8'
    complete -c perl -s d -n $noopt --description 'Debugger'
    complete -c perl -s dt -n 'commandline | string match -qr "d\$"' --description 'Debugger, with threads'
    complete -c perl -s D -n $noopt -x --description 'Debug option'
    complete -c perl -s e -n $noopt -x --description 'Execute command'
    complete -c perl -s E -n $noopt -x --description 'Execute command, enable optional features'
    complete -c perl -s f -n $noopt --description 'Disable sitecustomize.pl'
    complete -c perl -s F -n $noopt -x --description 'Set regexp used to split input'
    complete -c perl -s h -n $noopt --description 'Show help and exit'
    complete -c perl -s i -n $noopt -x --description 'Edit files in-place'
    complete -c perl -s I -n $noopt -r --description 'Include path'
    complete -c perl -s l -n $noopt --description 'Automatic line ending processing'
    complete -c perl -s m -n $noopt -x --description 'Require module' -a $modules
    complete -c perl -s M -n $noopt -x --description 'Use module' -a $modules
    complete -c perl -s n -n $noopt --description 'Loop script'
    complete -c perl -s p -n $noopt --description 'Loop script, print $_'
    complete -c perl -s s -n $noopt --description 'Define custom switches'
    complete -c perl -s S -n $noopt --description 'Search $PATH for script'
    complete -c perl -s t -n $noopt --description 'Taint checking, but only with warnings'
    complete -c perl -s T -n $noopt --description 'Taint checking'
    complete -c perl -s u -n $noopt --description 'Dump core'
    complete -c perl -s U -n $noopt --description 'Unsafe mode'
    complete -c perl -s v -n $noopt --description 'Display version and exit'
    complete -c perl -s V -n $noopt --description 'Display configuration and exit'
    complete -c perl -s w -n $noopt --description 'Show warnings'
    complete -c perl -s W -n $noopt --description 'Force warnings'
    complete -c perl -s X -n $noopt --description 'Disable warnings'
    complete -c perl -s x -n $noopt -r --description 'Extract script'
end
