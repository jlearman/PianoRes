#!/usr/bin/env bash

# Add priority lines to LV2 TTL file

DIR=Artifacts/PianoRes.lv2
TTL=dsp.ttl

set -e

let "PRIO = 200"
cat $DIR/$TTL | while IFS= read -r LINE ; do
    if [[ "$LINE" =~ ^"plug:_" ]] ; then
        printf "%s\n" "$LINE"
        printf "\t%s\n" "lv2:displayPriority $PRIO ;"
        let "PRIO = PRIO - 10"
    elif [[ "$LINE" == *displayPriority* ]] ; then
        :
    else
        printf "%s\n" "$LINE"
    fi
done
