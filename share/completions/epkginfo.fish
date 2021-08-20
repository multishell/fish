complete -c epkginfo -xa "(__fish_portage_print_available_pkgs)"
## Opts
complete -c epkginfo -s h -l help -d "display help"
complete -c epkginfo -s d -l description -d "extended pkg description"
complete -c epkginfo -s H -l herd -d "herd(s) for the pkg"
complete -c epkginfo -s k -l keywords -d "keywords for all matching pkg versions"
complete -c epkginfo -s l -l license -d "licenses for the best matching version"
complete -c epkginfo -s m -l maintainer -d "maintainer(s) for the pkg"
complete -c epkginfo -s S -l stablreq -d "STABLEREQ arches (cc's) for all matching pkg versio"
complete -c epkginfo -s u -l useflags -d "per-pkg USE flag descriptions"
complete -c epkginfo -s U -l upstream -d "pkg's upstream information"
complete -c epkginfo -s x -l xml -d "plain metadata.xml file"
