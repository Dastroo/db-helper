#!/bin/bash
if [ "$(whoami)" != root ]; then
    echo Please run this script as root or using sudo
    exit
fi

cd build || exit
make uninstall