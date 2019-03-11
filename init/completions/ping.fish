complete -c ping -a "(__fish_print_hostnames)" -x
complete -c ping -s a -d "Audible ping"
complete -c ping -s A -d "Adaptive ping"
complete -c ping -s b -d "Allow pinging a broadcast address"
complete -c ping -s B -d "Do not allow ping to change source address of probes"
complete -c ping -s c -d "Stop after sending count ECHO_REQUEST packets" -x
complete -c ping -s d -d "Set the SO_DEBUG option on the socket being used"
complete -c ping -s F -d "Allocate and set 20 bit flow label on echo request packets" -x
complete -c ping -s f -d "Flood ping"
complete -c ping -s i -d "Wait  interval  seconds  between sending each packet" -x
complete -c ping -s I -d "Set source address to specified interface address" -x -a "(__fish_print_interfaces; fish_print_addresses)"
complete -c ping -s l -d "If preload is specified, ping sends that many packets not waiting for reply" -x
complete -c ping -s L -d "Suppress loopback of multicast packets"
complete -c ping -s n -d "Numeric output only"
complete -c ping -s p -d "You may specify up to 16 ‘‘pad’’ bytes to fill out the packet you send" -x
complete -c ping -s Q -d "Set Quality of Service -related bits in ICMP datagrams" -x 
complete -c ping -s q -d "Quiet output"
complete -c ping -s R -d "Record route"
complete -c ping -s r -d "Bypass  the  normal routing tables and send directly to a host on an attached interface"
complete -c ping -s s -d "Specifies  the  number of data bytes to be sent" -x
complete -c ping -s S -d "Set socket sndbuf" -x
complete -c ping -s t -d "Set the IP Time to Live" -x
complete -c ping -s T -d "Set special IP timestamp options" -x
complete -c ping -s M -d "Select  Path  MTU  Discovery  strategy" -x -a "do want dont"
complete -c ping -s U -d "Print full user-to-user latency"
complete -c ping -s v -d "Verbose output"
complete -c ping -s V -d "Show version and exit"
complete -c ping -s w -d "Specify  a timeout, in seconds, before ping exits regardless of how many packets have been sent or received" -x
complete -c ping -s W -d "Time to wait for a response, in seconds" -x
