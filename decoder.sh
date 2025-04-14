#!/bin/bash

echo "╔=====================================================╗";
echo "║   D J I - F L I G H T L O G - D E C O D E R CLI     ║";
echo "╚================= by Innov'ATM ======================╝";


usage() {
    usage="
USAGE: $(basename "$0") [command] [options]

    Docker container management :
        docker build                 build the docker image
        docker start                 starts the docker container
        docker stop                  stops the docker container
    "
    echo "$usage"
}

case "$1" in
    docker)
        case "$2" in
            build)
                printf "\nBUILD DOCKER IMAGE\n\n"
                ./scripts/docker-build.sh
                exit
            ;;
            start)
                printf "\nSTART DOCKER CONTAINER\n\n"
                if [ -n "$3" ]; then
                    ./scripts/docker-start.sh $3
                else
                    echo "\n    ERROR: SDK Key required\n"
                fi
                exit
            ;;
            stop)
                printf "\nSTOP DOCKER CONTAINER\n\n"
                ./scripts/docker-stop.sh
                exit
            ;;
            *)
                printf "\n    ERROR: Unknown option name '$2'\n"
                usage
                exit
            ;;
        esac
    ;;
    *)
        printf "\n    ERROR: Unknown command name '$1'\n"
        usage
        exit
    ;;
esac