#!/bin/bash
exec 2> error.txt
find $1 -size +$2 -size -$3 -type f -printf " %p   %f   %s\n" > $4
find $1 -type f | wc -l
