#!/bin/sh
# Test for help output of krash
# help output must contain the line "Usage"
krash=../src/krash
#ask for help
$krash --help | grep 'Usage' >/dev/null || exit 1
echo "--help works"
#ask for help with short option
$krash -h | grep 'Usage' >/dev/null || exit 1
echo "-h works"
