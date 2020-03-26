#!/bin/bash

tup && ./test/diff_test.sh && ./bin/test/test -tse=inout
