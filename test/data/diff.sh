#!/bin/bash

ROOT="$(readlink -f $(dirname $0)/../..)"

pushd $ROOT > /dev/null

./bin/test/test -fc -ts=inout | tee /dev/stderr        \
                              | grep 'test/data' -A 1  \
                              | grep -E '(test|CHECK)' \
                              | paste - -              \
                              | while read line; do
                                    outputFile=$(echo $line | grep -oE 'test/data/[A-Za-z0-9_" /]*\.out' | tr -d ' "');
                                    lhs=$(echo $line | grep -oE '\{.*\} == {' | rev | cut -c 5- | rev);
                                    rhs=$(echo $line | grep -oE '\} == \{.*\} )' | rev | cut -c 3- | rev | cut -c 6-);
                                    clear;
                                    diff -u <(echo $rhs | jq .) <(echo $lhs | jq .) | ydiff -s;
                                    read -r -p "Do you want to update $outputFile? [Y/n] " input </dev/tty;

                                    case $input in
                                        [yY][eE][sS]|[yY])
                                      echo -n $lhs > ${ROOT}/$outputFile
                                      ;;
                                        [nN][oO]|[nN])
                                      ;;
                                        *)
                                      echo "Invalid input..."
                                      exit 1
                                      ;;
                                    esac
                                done
popd > /dev/null
