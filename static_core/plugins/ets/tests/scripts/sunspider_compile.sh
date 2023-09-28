#!/bin/bash -e

build=$1
stdlib=$2

if [[ -z "$1" || -z "$2" ]]; then
  echo "Build ark and es2panda. \n Provide args: ./sunspider_compile.sh /build/path /stdlib/path"
  exit 1
fi

script_path=$(dirname "$0")
script_path=$(cd "$script_path" && pwd)
tests_path=$(cd "$script_path/../Sunspider" && pwd)

cd $build
$build/bin/es2panda --extension=ets --output=etsstdlib.abc --gen-stdlib=true
for f in $tests_path/*.ets; do
  name=$(basename $f .ets)
  echo $name

  $build/bin/es2panda --extension=ets --output=out.abc \
  --gen-stdlib=false $f

  $build/bin/ark --boot-panda-files=etsstdlib.abc --load-runtimes=ets out.abc ETSGLOBAL::main

done
