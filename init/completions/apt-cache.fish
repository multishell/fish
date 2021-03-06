#apt-cache
complete -c apt-cache -s h -l help -d "apt-cache command help"
complete -c apt-cache -a add -d "add index files Debug only"
complete -f -c apt-cache -a gencaches -d "build apt cache"
complete -x -c apt-cache -a showpkg -d "show package info"
complete -f -c apt-cache -a stats -d "show cache statistics"
complete -x -c apt-cache -a showsrc -d "show source package"
complete -f -c apt-cache -a dump -d "show packages in cache"
complete -f -c apt-cache -a dumpavail -d "print available list"
complete -f -c apt-cache -a unmet -d "list unmet dep in cache"
complete -x -c apt-cache -a show -d "display package record"
complete -x -c apt-cache -a search -d "search pkgname by REGEX"
complete -c apt-cache -l full -a search -d "search full package name"
complete -x -c apt-cache -l names-only -a search -d "search pkgname only"
complete -x -c apt-cache -a depends -d "list dep for the package"
complete -x -c apt-cache -a rdepends -d "list reverse dep for the package"
complete -x -c apt-cache -a pkgnames -d "print package name by prefix"
complete -x -c apt-cache -a dotty -d "generate dotty output for packages"
complete -x -c apt-cache -a policy -d "debug preferences file"
complete -x -c apt-cache -a madison -d "mimic madison"
complete -r -c apt-cache -s p -l pkg-cache -d "select file to store pkg cache"
complete -r -c apt-cache -s s -l src-cache -d "select file to store src cache"
complete -f -c apt-cache -s q -l quiet -d "quiet output"
complete -f -c apt-cache -s i -l important -d "print important deps"
complete -f -c apt-cache -s a -l all-versions -d "print full records"
complete -f -c apt-cache -s g -l generate -d "auto-gen package cache"
complete -f -c apt-cache -l all-names -d "print all names"
complete -f -c apt-cache -l recurse -d "dep and rdep recursive"
complete -f -c apt-cache -l installed -d "limit to installed"
complete -f -c apt-cache -s v -l version -d "show version"
complete -r -c apt-cache -s c -l config-file -d "specify config file"
complete -x -c apt-cache -s o -l option -d "specify options"

