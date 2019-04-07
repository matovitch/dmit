#!/bin/bash

ROOT_FOLDER="$(readlink -f $(dirname $0)/..)"
PROJECT_NAME=$(basename $ROOT_FOLDER | perl -pe 's/([^_])([A-Z])/\1_\2/g' | tr '[:upper:]' '[:lower:]')

function makeEnv()
{
    docker build -t ${PROJECT_NAME}:env -f ${ROOT_FOLDER}/env/env.Dockerfile ${ROOT_FOLDER}/env
}

function runEnv()
{
    CONTAINER="${PROJECT_NAME}_env"

    IMAGE="${PROJECT_NAME}:env"

    docker container prune -f 2>&1 > /dev/null

    docker inspect $CONTAINER 2> /dev/null | grep running > /dev/null &&
        docker container rm -f $CONTAINER 2>&1 > /dev/null

    docker run --privileged                                            \
               --cap-add ALL                                           \
               --user guest:guest                                      \
               --device /dev/fuse                                      \
               --mount type=bind,source=${ROOT_FOLDER},target=/mnt     \
               --workdir /mnt                                          \
               --name $CONTAINER                                       \
               --interactive                                           \
               --tty                                                   \
               $IMAGE $@
}
