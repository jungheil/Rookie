#! /bin/bash

function CheckSudo() {
    echo -e "\033[33m +++++ CheckSudo +++++ \033[0m"
    sudo echo -e "\033[32m[ CheckSudo ] OK!\033[0m" || \
    { echo -e "\033[31m +++++ CheckSudo Failed +++++ \033[0m"; return 1; }

    return 0
}

function kill() {
    PROCESS=`ps -ef|grep $1|grep -v grep|grep -v PPID|awk '{ print $2 }'`
    for i in $PROCESS
    do
        echo "Kill the $1 process [ $i ]"
        sudo kill -9 $i
    done
}

CRTDIR=$(pwd)

CheckSudo

kill web.py
kill Rookie
kill socketimage.py
