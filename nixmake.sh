#!/usr/bin/env bash
if [[ "$1" = "clean" ]]
then
        rm -rf out
        mkdir out
fi
export PLUGINTYPE=so
make build
