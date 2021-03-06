#
# Initializations that should only be performed when in interactive mode
#

if not status --is-interactive
	exit
end

#
# Print a greeting 
#

printf 'Welcome to fish, the friendly interactive shell\nType ' 
set_color green 
printf 'help ' 
set_color normal 
printf 'for instructions on how to use fish\n' 

#
# Set exit message
#

function fish_on_exit -d "Commands to execute when fish exits"
	echo Good bye
end

# Set the default prompt command. Make sure that every terminal escape
# string has a newline before and after, so that fish will know how
# long it is.

function fish_prompt -d "Write out the prompt"
	printf '%s@%s \n%s\n%s\n%s\n> ' (whoami) (hostname|cut -d . -f 1) (set_color $fish_color_cwd) (prompt_pwd) (set_color normal)
end

#
# Set INPUTRC to something nice
#
# We override INPUTRC if already set, since it may be set by a shell 
# other than fish, which may use a different file. The new value should
# be exported, since the fish inputrc file plays nice with other files 
# by including them when found.
#

for i in ~/.fish_inputrc ~/etc/fish_inputrc /etc/fish_inputrc ~/.inputrc /etc/inputrc
	if test -f $i
		set -xg INPUTRC $i
		break
	end
end


#
# Set various color values
#

function set_default -d "Set an universal variable, unless it has already been set"
	if not set -q $argv[1] 
		set -U -- $argv	
	end
end

function set_exported_default -d "Set an exported universal variable, unless it has already been set"
	if not set -q $argv[1]
		set -Ux -- $argv	
	end
end


# Regular syntax highlighting colors
set_default fish_color_normal normal
set_default fish_color_command green
set_default fish_color_redirection normal
set_default fish_color_comment brown
set_default fish_color_error red

set_default fish_color_cwd green

# Background color for matching quotes and parenthesis
set_default fish_color_match cyan

# Background color for search matches
set_default fish_color_search_match purple

# Pager colors
set_default fish_pager_color_prefix cyan
set_default fish_pager_color_completion normal
set_default fish_pager_color_description normal
set_default fish_pager_color_progress cyan

# Directory history colors
set_default fish_color_history_current cyan


#
# Setup the CDPATH variable
#

set_exported_default CDPATH . ~

#
# Match colors for grep, if supported
#

if grep --color=auto --help 1>/dev/null 2>/dev/null
	set_exported_default GREP_COLOR '97;45'
	set_exported_default GREP_OPTIONS '--color=auto'
end

#
# Color definitions for ls, if supported
#

if command ls --color=auto --help 1>/dev/null 2>/dev/null
	set_exported_default LS_COLORS $LS_COLORS '*.jar=01;31' '*.doc=35' '*.pdf=35' '*.ps=35' '*.xls=35' '*.swf=35' '*~=37'
end


functions -e set_default
functions -e set_exported_default
