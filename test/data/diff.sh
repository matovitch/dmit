#!/bin/bash

function wasmify()
{
    echo $1 | tr '|' '\n' | while read obj; do
                                wasm-objdump -x <(echo $obj | base64 -d) | tail -n +5
                                wasm2wat <(echo $obj | base64 -d)
                            done
}

ROOT="$(readlink -f $(dirname $0)/../..)"

pushd $ROOT > /dev/null

./bin/test/test -fc -ni -ts=json | grep -vP 'CLK[0-9] '   \
                                 | tee /dev/stderr        \
                                 | grep 'test/data' -A 1  \
                                 | grep -E '(test|CHECK)' \
                                 | paste - -              \
                                 | while read line; do
                                       outputFile=$(echo $line | grep -oE 'test/data/[A-Za-z0-9_" /]*\.out' | tr -d ' "');
                                       lhs=$(echo $line | grep -oP '\( [\{\[].* == [\[\{]' | cut -c 3- | rev | cut -c 5- | rev);
                                       rhs=$(echo $line | grep -oP ' == [\[\{].*[\}\]] \)' | rev | cut -c 3- | rev | cut -c 5-);
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

./bin/test/test -fc -ni -ts=wasm  | grep -vP 'CLK[0-9] '   \
                                  | tee /dev/stderr        \
                                  | grep 'test/data' -A 1  \
                                  | grep -E '(test|CHECK)' \
                                  | paste - -              \
                                  | while read line; do
                                        outputFile=$(echo $line | grep -oE 'test/data/[A-Za-z0-9_" /]*\.out' | tr -d ' "');
                                        lhs=$(echo $line | grep -oP 'values: .*' | grep -oP 'CHECK.*' | grep -oP '.* ==' | cut -c 7- | rev | cut -c 3- | rev);
                                        rhs=$(echo $line | grep -oP 'values: .*' | grep -oP 'CHECK.*' | grep -oP '== .*' | cut -c 7- | rev | cut -c 3- | rev);

                                        diff -u <(wasmify $rhs) <(wasmify $lhs) | ydiff -s;
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
