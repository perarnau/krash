#!/bin/sh
# Test to ensure calling krash without profile fail and show usage
krash=../src/krash
echo "Test krash can stop"
echo "kill 1" >just_kill
# MUST SUCCEED
$krash -p just_kill >/dev/null
ret=$?
rm just_kill
exit $ret
