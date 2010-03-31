#!/bin/sh
#
#  This file is part of KRASH.
#
#  KRASH is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  KRASH is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with KRASH.  If not, see <http://www.gnu.org/licenses/>.
#
#  Copyright Swann Perarnau, 2008
#  Contact: firstname.lastname@imag.fr
#
# Test to ensure calling krash without profile fail and show usage
krash=../src/krash
# MUST FAIL
$krash >usage
ret=$?
grep "Usage" usage >/dev/null || exit 0
echo "Usage output without profile argument"
rm usage
exit $ret
