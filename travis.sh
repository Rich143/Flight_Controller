#!/usr/bin/env sh
set -evx
env | sort

cd ./test
make run
