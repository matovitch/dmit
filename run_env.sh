#!/bin/bash

ROOT="$(readlink -f $(dirname $0))"

pushd $ROOT > /dev/null
pushd env   > /dev/null

. env.sh

popd > /dev/null

runEnv bash --init-file ./env/env.sh

popd > /dev/null
