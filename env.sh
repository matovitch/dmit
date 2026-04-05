#!/bin/bash

function make()
{
    tup && $PIXI_PROJECT_ROOT/bin/test/test -fc -ni -tse=json,wasm | grep -vP 'CLK[0-9] ' && $PIXI_PROJECT_ROOT/test/data/diff.sh
}

function base64archive2Wasm()
{
    cat $1 | tr '|' '\n' | while read line
                           do
                               file=$(mktemp)
                               echo $line | base64 -d > $file
                               echo -n "$file "
                           done \
                                | xargs ar r /tmp/wasm.a && wasm-ld --export-all --no-entry --whole-archive /tmp/wasm.a -o /tmp/a.wasm
    rm /tmp/wasm.a
}

function path2u64pair()
{
    echo -e $1 | $PIXI_PROJECT_ROOT/bin/src/mangler.bin  | tail -n 1 | { read id; echo "$id=="; } | base64 -d | xxd -g8 -e | awk '{print "{0x"$2", 0x"$3"};"}'
}

function makeDigraph()
{
    make 2>&1 | grep digraph | sed -e $'s/\x1b\[[0-9;]*m//g' | while read line
                                                               do
                                                                   echo $line > $(echo $line | md5sum | cut -d ' ' -f1).dot
                                                               done
}

function cleanSource()
{
    find $HOST_ROOT -path '*/.*' -prune -o \
                    -type f \( -name "*.cpp"   -o \
                               -name "*.hpp"   -o \
                               -name "*.h"     -o \
                               -name "*.sh"    -o \
                               -name "*.json"  -o \
                               -name "*.in"    -o \
                               -name "*.out"   -o \
                               -name "*.gv"    -o \
                               -name "Tupfile" -o \
                               -name "*.tup"      \
                                                  \) -print | grep -v whitespace | while read file; do \
                                                                                       dos2unix $file;
                                                                                       sed -i 's/ *$//' $file;
                                                                                   done
}

function dmitStart()
{
    $PIXI_PROJECT_ROOT/bin/src/dmit_server.bin --url tcp://127.0.0.1:4567 > dmit_server.log&
}

function dmitClient()
{
    $PIXI_PROJECT_ROOT/bin/src/dmit_client.bin --url tcp://127.0.0.1:4567 $@
}
