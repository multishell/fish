#completion for arp
complete -c arp -s v -l verbose -d "verbose mode"
complete -c arp -s n -l numeric -d "numerical address"
complete -x -c arp -s H -l tw-type -a "ether arcnet pronet ax25 netrom" -d "class of hw type"
complete -c arp -s a -l display -x -a "(__fish_print_hostnames)" -d "show arp entries"
complete -x -c arp -s d -l delete -a "(__fish_print_hostnames)" -d "remove an entry for hostname"
complete -c arp -s D -l use-device -d "use hardware address"
complete -x -c arp -s i -l device -a "(__fish_print_interfaces)" -d "select interface"
complete -x -c arp -s s -l set -d "Manually create ARP address" -a "(__fish_print_hostnames)"
complete -f -c arp -s f -l file -d "taken addr from filename, default /etc/ethers" 

