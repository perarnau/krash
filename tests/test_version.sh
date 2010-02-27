#!/bin/sh
# Tests for the version command line option
# Version output should contain a Copyright line
krash=../bin/krash
#ask for version
$krash --version | grep "Copyright" >/dev/null || exit 1
echo "--version works"
#ask for version with short opt
$krash -V | grep "Copyright" >/dev/null || exit 1
echo "-V works"
