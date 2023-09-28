#!/usr/bin/env bash

set -eo pipefail

SCRIPT_DIR="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"
cd "$SCRIPT_DIR"

TYPES=(
    boolean
    byte
    short
    int
    long
    float
    double
    char
    Object
)

TEMPLATES=(
    header2.ets.j2
    fill.ets.j2
    copyToOf.ets.j2
    sort.ets.j2
    indexOf.ets.j2
    lastIndexOf.ets.j2
    search.ets.j2
    concat_reverse.ets.j2
    join.ets.j2
    forEach.ets.j2
    map.ets.j2
    filter.ets.j2
)

GENPATH=../../stdlib/std/core

mkdir -p "$GENPATH"

filename="$GENPATH/Array.ets"
cat header.ets.j2 > "$filename"

for atype in "${TYPES[@]}"
do
    echo "Generating $atype..."

    for template in "${TEMPLATES[@]}"
    do
        jinja2 "-DT=$atype" "$template" >> "$filename"
        echo "" >> "$filename"
    done
done

echo Done.
