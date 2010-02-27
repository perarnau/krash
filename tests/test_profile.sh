#!/bin/sh
# Test to ensure calling krash without profile fail and show usage
krash=../src/krash
# MUST FAIL
$krash >usage
ret=$?
grep "Usage" usage >/dev/null || exit 0
echo "Usage output without profile argument"
exit $ret
