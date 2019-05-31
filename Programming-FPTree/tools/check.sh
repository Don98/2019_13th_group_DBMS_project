#!/bin/bash

var=`ldconfig -p | grep libpmem.so`
if [ -z "$var" ]; then
    echo -e "error, maybe you have not installed pmdk? \033[31m[error]\033[0m"
else
    echo -e "you have intalled pmdk \033[32m                   [pass]\033[0m"
fi

var=`ldconfig -p | grep libfptree.so`
if [ -z "$var" ]; then
    echo -e "checking if you have created libfptree.so \033[31m[error]\033[0m"
else
    echo -e "checking if you have created libfptree.so \033[32m[pass]\033[0m"
fi
