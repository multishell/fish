complete -c date -s d -l date -d (N_ "Display date described by string") -x
complete -c date -s f -l file -d (N_ "Display date for each line in file") -r
complete -c date -s I -l iso-8601 -d (N_ "Output in ISO 8601 format") -x -a "date hours minutes seconds"
complete -c date -s s -l set -d (N_ "Set time") -x
complete -c date -s R -l rfc-2822 -d (N_ "Output RFC-2822 compliant date string")
complete -c date -s r -l reference -d (N_ "Display the last modification time of file") -r
complete -c date -s u -l utc -d (N_ "Print or set Coordinated Universal Time")
complete -c date -l universal -d (N_ "Print or set Coordinated Universal Time")
complete -c date -s h -l help -d (N_ "Display help and exit")
complete -c date -s v -l version -d (N_ "Display version and exit")

