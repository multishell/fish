
function setenv -d (N_ 'Set global variable. Alias for set -g, made for csh compatibility')
	set -gx $argv
end
