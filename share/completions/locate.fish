#completion for locate

complete -c locate -s A -l all --description 'Print only names which match all non-option arguments'
complete -c locate -s c -l count --description 'Print the total number of matches found'
complete -c locate -s e -l existing --description 'Only  print  out  such names that currently exist'
complete -c locate -s E -l non-existing --description 'Only print out such names that currently do not exist'
complete -c locate -s L -l follow --description 'Consider broken symbolic links to be non-existing files'
complete -c locate -s P -l nofollow --description 'Treat broken symbolic links as if they were existing'
complete -c locate -s H -l nofollow --description 'Treat broken symbolic links as if they were existing'
complete -c locate -s i -l ignore-case --description 'Ignore case distinctions between pattern and file names'
complete -c locate -s m -l mmap --description 'Does nothing. For compatibility with BSD locate'
complete -c locate -s 0 -l null --description 'Use ASCII NUL as a separator, instead of newline'
complete -c locate -s p -l print --description 'Print search results when they normally would not'
complete -c locate -s w -l whole-name --description 'Match against the whole name of the file'
complete -c locate -s b -l base-name --description 'Match against the final component of the file name'
complete -c locate -s r -l regex --description 'The pattern is regular expression instead of glob pattern'
complete -c locate -s s -l stdio --description 'Does nothing. For compatibility with BSD locate'
complete -c locate -s S -l statistics --description 'Print statistics about each locate database and exit'
complete -c locate -l help --description 'Print a summary of the options to locate and exit'
complete -c locate -l version --description 'Print the version number of locate and exit'

complete -r -c locate -s d -l database --description 'Search the file name databases in these directories'
complete -r -c locate -s l -l limit --description 'Limit  the number of matches'
