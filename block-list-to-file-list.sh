#!/usr/bin/env bash

die() {
    echo $@
    exit 1
}

TEMPFILE=/tmp/$$
trap "rm -f ${TEMPFILE}; exit" INT TERM EXIT
cat "$1" | while read line; do
    find /data/*/cmccabe -name "$line" > "$TEMPFILE"
    NUMLINES=$(wc -l "$TEMPFILE" | awk '{print $1}')
    if [ $NUMLINES == 1 ]; then
        cat /tmp/$$
    else
        :
        #echo "failed to locate block ${line}"
    fi
done
exit 0
