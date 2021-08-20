#!/bin/bash

usage() {
    cat << EOF
Usage: $(basename $0) [--shell-before] [--shell-after] DOCKERFILE
Options:
    --shell-before   Before the tests start, run a bash shell
    --shell-after    After the tests end, run a bash shell
EOF
    exit 1
}

DOCKER_EXTRA_ARGS=""

# Exit on failure.
set -e

# Get fish source directory.
FISH_SRC_DIR=$(cd "$( dirname "${BASH_SOURCE[0]}" )"/.. && pwd)

# Parse args.
while [ $# -gt 1 ]; do
    case "$1" in
        --shell-before)
            DOCKER_EXTRA_ARGS="$DOCKER_EXTRA_ARGS --env FISH_RUN_SHELL_BEFORE_TESTS=1"
            ;;
        --shell-after)
            DOCKER_EXTRA_ARGS="$DOCKER_EXTRA_ARGS --env FISH_RUN_SHELL_AFTER_TESTS=1"
            ;;
        *)
            usage
            ;;
    esac
    shift
done

DOCKERFILE=${@:$OPTIND:1}
test -n "$DOCKERFILE" || usage

# Construct a docker image.
IMG_TAGNAME="fish_$(basename -s .Dockerfile "$DOCKERFILE")"
docker build \
    -t "$IMG_TAGNAME" \
    -f "$DOCKERFILE" \
    "$FISH_SRC_DIR"/docker/context/

# Run tests in it, allowing them to fail without failing this script.
docker run -it \
    --mount type=bind,source="$FISH_SRC_DIR",target=/fish-source,readonly \
    $DOCKER_EXTRA_ARGS \
    "$IMG_TAGNAME"
