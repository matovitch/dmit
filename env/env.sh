#!/bin/bash

ROOT_FOLDER="$(readlink -f $(dirname ${BASH_SOURCE[${#BASH_SOURCE[@]} - 1]})/..)"
PROJECT_NAME=$(basename $ROOT_FOLDER | perl -pe 's/([^_])([A-Z])/\1_\2/g' | tr '[:upper:]' '[:lower:]')

function make()
{
    tup && $HOST_ROOT/bin/test/test -tse=inout && $HOST_ROOT/test/data/diff.sh
}

function base64id2u64pair()
{
    echo $1 | if read id; then echo "${id}==" | base64 -d | xxd -e -g8; fi | awk '{print "{0x"$2", 0x"$3"};"}'
}

function cleanSource()
{
    find $HOST_ROOT -type f \( -name "*.cpp"   -o \
                               -name "*.hpp"   -o \
                               -name "*.h"     -o \
                               -name "*.sh"    -o \
                               -name "*.gv"    -o \
                               -name "Tupfile" -o \
                               -name "*.tup"      \
                                                  \) | while read file; do sed -i 's/ *$//' $file; done
}

function dmitServer()
{
    $HOST_ROOT/bin/src/dmit_server.bin --url tcp://127.0.0.1:4567 > dmit_server.log&
}

function dmitClient()
{
    $HOST_ROOT/bin/src/dmit_client.bin --url tcp://127.0.0.1:4567 $@
}

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

    docker run --privileged                                                \
               --cap-add ALL                                               \
               --user guest:guest                                          \
               --device /dev/fuse                                          \
               --mount type=bind,source=${ROOT_FOLDER},target=$ROOT_FOLDER \
               --workdir $ROOT_FOLDER                                      \
               --name $CONTAINER                                           \
               --interactive                                               \
               --tty                                                       \
               --env HOST_ROOT=$ROOT_FOLDER                                \
               -p 5678:5678                                                \
               $IMAGE $@
}
