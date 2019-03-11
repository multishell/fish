# completion for xterm

function __fish_complete_xterm_encoding -d "Complete encoding information for xterm"
    iconv --list|sed -e 's|//||'
end

complete -c xterm -n '__fish_test_arg "+*"' -a +ah --description 'Never highlight the text cursor'
complete -c xterm -n '__fish_test_arg "+*"' -a +ai --description 'Enable active icon support'
complete -c xterm -n '__fish_test_arg "+*"' -a +aw --description 'Disallow auto wraparound'
complete -c xterm -n '__fish_test_arg "+*"' -a +bc --description 'Turn off cursor blinking'
complete -c xterm -n '__fish_test_arg "+*"' -a +bdc --description 'Enable the display of bold characters'
complete -c xterm -n '__fish_test_arg "+*"' -a +cb --description 'Set the vt100 resource cutToBeginningOfLine to \'true\''
complete -c xterm -n '__fish_test_arg "+*"' -a +cjk_width --description 'Set the cjkWidth resource to \'false\''
complete -c xterm -n '__fish_test_arg "+*"' -a +cm --description 'Enable recognition of ANSI color-change escape sequences'
complete -c xterm -n '__fish_test_arg "+*"' -a +cn --description 'Cut newlines in line-mode selections'
complete -c xterm -n '__fish_test_arg "+*"' -a +cu --description 'Don\'t workaround the bug in more(1)'
complete -c xterm -n '__fish_test_arg "+*"' -a +dc --description 'Disable dynamic color changing'
complete -c xterm -n '__fish_test_arg "+*"' -a +fbb --description 'Don\'t ensure compatibilty between normal and bold fonts bounding boxes'
complete -c xterm -n '__fish_test_arg "+*"' -a +fbx --description 'Normal and bold fonts have VT100 line-drawing characters'
complete -c xterm -n '__fish_test_arg "+*"' -a +hf --description 'Don\'t generate HP Function Key escape codes for function keys'
complete -c xterm -n '__fish_test_arg "+*"' -a +hold --description 'Close xterm\'s window after the shell exits'
complete -c xterm -n '__fish_test_arg "+*"' -a +ie --description 'Don\'t use pseudo-terminal\'s sense of the stty erase value'
complete -c xterm -n '__fish_test_arg "+*"' -a +im --description 'Don\'t force insert mode'
complete -c xterm -n '__fish_test_arg "+*"' -a +j --description 'Don\'t use jump scrolling'
complete -c xterm -n '__fish_test_arg "+*"' -a +k8 --description 'Don\'t treat C1 control characters as printable'
complete -c xterm -n '__fish_test_arg "+*"' -a +l --description 'Turn off logging'
complete -c xterm -n '__fish_test_arg "+*"' -a +lc --description 'Turn off support for encodings according the locale setting'
complete -c xterm -n '__fish_test_arg "+*"' -a +ls --description 'The shell in xterm\'s window will not be login shell'
complete -c xterm -n '__fish_test_arg "+*"' -a +mb --description 'Don\'t ring bell if the user types near the end of line'
complete -c xterm -n '__fish_test_arg "+*"' -a +mesg --description 'Allow write access to the terminal'
complete -c xterm -n '__fish_test_arg "+*"' -a +mk_width --description 'Don\'t use builtin version for the wide-character calculation'
complete -c xterm -n '__fish_test_arg "+*"' -a +nul --description 'Enable underlining'
complete -c xterm -n '__fish_test_arg "+*"' -a +pc --description 'Disable PC style bold colors'
complete -c xterm -n '__fish_test_arg "+*"' -a +pob --description 'Don\'t raise window on Control-G'
complete -c xterm -n '__fish_test_arg "+*"' -a +rvc --description 'Enable characters with reverse attribute as color'
complete -c xterm -n '__fish_test_arg "+*"' -a +rw --description 'Disable reverse-wraparound'
complete -c xterm -n '__fish_test_arg "+*"' -a +s --description 'Turn off asynchronous scrolling'
complete -c xterm -n '__fish_test_arg "+*"' -a +samename --description 'Send title/icon change requests always'
complete -c xterm -n '__fish_test_arg "+*"' -a +sb --description 'Don\'t display scrollbar'
complete -c xterm -n '__fish_test_arg "+*"' -a +sf --description 'Don\'t generate Sun Function Key escape codes for function keys'
complete -c xterm -n '__fish_test_arg "+*"' -a +si --description 'Move the screen to the bottom on input'
complete -c xterm -n '__fish_test_arg "+*"' -a +sk --description 'Don\'t move the screen to the bottom on key press while scrolling'
complete -c xterm -n '__fish_test_arg "+*"' -a +sm --description 'Don\'t setup session manager callbacks'
complete -c xterm -n '__fish_test_arg "+*"' -a +sp --description 'Don\'t assume Sun/PC keyboard'
complete -c xterm -n '__fish_test_arg "+*"' -a +t --description 'VT102 mode'
complete -c xterm -n '__fish_test_arg "+*"' -a +tb --description 'Don\'t display toolbar or menubar'
complete -c xterm -n '__fish_test_arg "+*"' -a +u8 --description 'Don\'t use UTF-8'
complete -c xterm -n '__fish_test_arg "+*"' -a +ulc --description 'Display characters with underline attribute as color'
complete -c xterm -n '__fish_test_arg "+*"' -a +ut --description 'Write to the system utmp log file'
complete -c xterm -n '__fish_test_arg "+*"' -a +vb --description 'Don\'t use visual bell insead of audio bell'
complete -c xterm -n '__fish_test_arg "+*"' -a +wc --description 'Don\'t use wide characters'
complete -c xterm -n '__fish_test_arg "+*"' -a +wf --description 'Don\'t wait the first time for the window to be mapped'

complete -c xterm -o version --description 'Print version number to the standard output'
complete -c xterm -o help --description 'Print out a verbose message describing the options'
complete -c xterm -o 123 --description 'Don\'t ignore the VT102 DECCOLM escape sequence'
complete -c xterm -o ah --description 'Always highlight the text cursor'
complete -c xterm -o ai --description 'Disable active icon support'
complete -c xterm -o aw --description 'Allow auto wraparound'
complete -c xterm -o bc --description 'Turn on cursor blinking'
complete -c xterm -o bdc --description 'Disable the display of bold characters'
complete -c xterm -o cb --description 'Set the vt100 resource cutToBeginningOfLine to \'false\''
complete -c xterm -o cc --description 'Set character class'
complete -c xterm -o cjk_width --description 'Set the cjkWidth resource to \'true\''
complete -c xterm -o cm --description 'Disable recognition of ANSI color-change escape sequences'
complete -c xterm -o cn --description 'Don\'t cut newlines in line-mode selections'
complete -c xterm -o cu --description 'Workaround for bug in more(1)'
complete -c xterm -o dc --description 'Enable dynamic color changing'
complete -c xterm -o fbb --description 'Ensure compatibilty between normal and bold fonts bounding boxes'
complete -c xterm -o fbx --description 'Normal and bold fonts don\'t have VT100 line-drawing characters'
complete -c xterm -o hf --description 'Generate HP Function Key escape codes for function keys'
complete -c xterm -o hold --description 'Don\'t close xterm\'s window after the shell exits'
complete -c xterm -o ie --description 'Use pseudo-terminal\'s sense of the stty erase value'
complete -c xterm -o im --description 'Force insert mode'
complete -c xterm -o k8 --description 'Treat C1 control characters as printable'
complete -c xterm -o lc --description 'Turn on support for encodings according the locale setting'
complete -c xterm -o leftbar --description 'Force scrollbar to the left side'
complete -c xterm -o ls --description 'The shell in xterm\'s window will be login shell'
complete -c xterm -o mb --description 'Ring bell if the user types near the end of line'
complete -c xterm -o mesg --description 'Disallow write access to the terminal'
complete -c xterm -o mk_width --description 'Use builtin version for the wide-character calculation'
complete -c xterm -o nul --description 'Disable underlining'
complete -c xterm -o pc --description 'Enable PC style bold colors'
complete -c xterm -o pob --description 'Raise window on Control-G'
complete -c xterm -o rightbar --description 'Force scrollbar to the right side'
complete -c xterm -o rvc --description 'Disable characters with reverse attribute as color'
complete -c xterm -o rw --description 'Enable reverse-wraparound'
complete -c xterm -o samename --description 'Don\'t send title/icon change requests always'
complete -c xterm -o sb --description 'Display scrollbar'
complete -c xterm -o sf --description 'Generate Sun Function Key escape codes for function keys'
complete -c xterm -o si --description 'Don\'t move the screen to the bottom on input'
complete -c xterm -o sk --description 'Move the screen to the bottom on key press while scrolling'
complete -c xterm -o sm --description 'Setup session manager callbacks'
complete -c xterm -o sp --description 'Assume Sun/PC keyboard'
complete -c xterm -o tb --description 'Display toolbar or menubar'
complete -c xterm -o u8 --description 'Use UTF-8'
complete -c xterm -o ulc --description 'Don\'t display characters with underline attribute as color'
complete -c xterm -o ut --description 'Don\'t write to the system utmp log file'
complete -c xterm -o vb --description 'Use visual bell insead of audio bell'
complete -c xterm -o wc --description 'Use wide characters'
complete -c xterm -o wf --description 'Wait the first time for the window to be mapped'
complete -c xterm -o Sccn --description 'Use as input/output channel for an existing program'

complete -c xterm -s e -a "(commandline -ct)(complete -C(commandline -ct))" -x --description 'Run program in xterm'

complete -r -c xterm -o bcf --description 'Blinking cursor will be off for that many milliseconds'
complete -r -c xterm -o bcn --description 'Blinking cursor will be on for that many milliseconds'
complete -r -c xterm -o class --description 'Override xterm resource class'
complete -r -c xterm -o cr --description 'Color for the text cursor'
complete -r -c xterm -o en -xa "(__fish_complete_xterm_encoding )" --description 'xterm encoding'
complete -r -c xterm -o fb --description 'Bold font'
complete -r -c xterm -o fa --description 'FreeType font pattern'
complete -r -c xterm -o fd --description 'FreeType double-width font pattern'
complete -r -c xterm -o fi --description 'Font for active icons'
complete -r -c xterm -o fs --description 'Font size for FreeType font'
complete -r -c xterm -o fw --description 'Font for displaying wide text'
complete -r -c xterm -o fwb --description 'Font for displaying bold wide text'
complete -r -c xterm -o fx --description 'Font for the preedit string in "OverTheSpot"'
complete -r -c xterm -o hc --description 'Color for highlighted text'
complete -r -c xterm -o into --description 'Embed xterm into window'
complete -r -c xterm -o kt --description 'Set keyboard type' -a "unknown default hp sco sun vt220"
complete -r -c xterm -o lcc --description 'File name for the encoding converter'
complete -r -c xterm -o lf --description 'Log filename'
complete -r -c xterm -o mc --description 'Maximum time in milliseconds between multi-click selections'
complete -r -c xterm -o ms --description 'Color for the pointer cursor'
complete -r -c xterm -o nb --description 'Distance from the right end for ringing the margin bell'
complete -r -c xterm -o sl --description 'Number of scrolled off lines'
complete -r -c xterm -o ti --description 'Terminal identification' -a "vt52 vt100 vt101 vt102 vt220"
complete -r -c xterm -o tm --description 'Terminal name for $TERM'
complete -r -c xterm -o ziconbeep --description 'zIconBeep percentage'

complete -r -c xterm -s b --description 'Size of the inner border'

complete -c xterm -s j --description 'Use jump scrolling'
complete -c xterm -s l --description 'Turn on logging'
complete -c xterm -s s --description 'Turn on asynchronous scrolling'
complete -c xterm -s t --description 'Tektronix mode'
complete -c xterm -s C --description 'This window should receive console output'

