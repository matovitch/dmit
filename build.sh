#!/bin/bash

tup && ./bin/test/test -tse=inout && ./test/diff_test.sh
