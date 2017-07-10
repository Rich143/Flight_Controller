#!/usr/bin/env sh
set -evx
env | sort

cd ./test
pwd
ls -R ../common
make run
