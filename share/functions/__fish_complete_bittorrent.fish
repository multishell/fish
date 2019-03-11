# Bittorrent commands

function __fish_complete_bittorrent
	complete -c $argv -l max_uploads -x -d (N_ "Maximum uploads at once")
	complete -c $argv -l keepalive_interval -x -d (N_ "Number of seconds between keepalives")
	complete -c $argv -l download_slice_size -x -d (N_ "Bytes per request")
	complete -c $argv -l request_backlog -x -d (N_ "Requests per pipe")
	complete -c $argv -l max_message_length -x -d (N_ "Maximum length prefix encoding")
	complete -c $argv -l ip -d (N_ "IP to report to the tracker") -x -a "(__fish_print_addresses)"
	complete -c $argv -l minport -d (N_ "Minimum port to listen to")
	complete -c $argv -l maxport -d (N_ "Maximum port to listen to")
	complete -c $argv -l responsefile -r -d (N_ "File for server response")
	complete -c $argv -l url -x -d (N_ "URL to get file from")
	complete -c $argv -l saveas -r -d (N_ "Local file target")
	complete -c $argv -l timeout -x -d (N_ "Time to close inactive socket")
	complete -c $argv -l timeout_check_interval -x -d (N_ "Time between checking timeouts")
	complete -c $argv -l max_slice_length -x -d (N_ "Maximum outgoing slice length")
	complete -c $argv -l max_rate_period -x -d (N_ "Maximum time to guess rate")
	complete -c $argv -l bind -x -d (N_ "IP to bind to locally") -a "(__fish_print_addresses)"
	complete -c $argv -l display_interval -x -d (N_ "Time between screen updates")
	complete -c $argv -l rerequest_interval -x -d (N_ "Time to wait between requesting more peers")
	complete -c $argv -l min_peers -x -d (N_ "Minimum number of peers to not do requesting")
	complete -c $argv -l http_timeout -x -d (N_ "Number of seconds before assuming http timeout")
	complete -c $argv -l max_initiate -x -d (N_ "Number of peers at which to stop initiating new connections")
	complete -c $argv -l max_allow_in -x -d (N_ "Maximum number of connections to allow")
	complete -c $argv -l check_hashes -x -d (N_ "Whether to check hashes on disk")
	complete -c $argv -l max_upload_rate -x -d (N_ "Maximum kB/s to upload at")
	complete -c $argv -l snub_time -x -d (N_ "Seconds to wait for data to come in before assuming choking")
	complete -c $argv -l spew -x -d (N_ "Whether to display diagnostic info")
	complete -c $argv -l rarest_first_cutoff -x -d (N_ "Number of downloads at which to switch from random to rarest first")
	complete -c $argv -l min_uploads -x -d (N_ "Number of uploads to fill out to with optimistic unchokes")
	complete -c $argv -l report_hash_failiures -x -d (N_ "Whether to inform the user that hash failures occur")
end
