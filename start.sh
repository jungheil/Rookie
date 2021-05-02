#! /bin/bash

password=$1

function kill() {
    PROCESS=`ps -ef|grep $1|grep -v grep|grep -v PPID|awk '{ print $2 }'`
    for i in $PROCESS
    do
        echo "Kill the $1 process [ $i ]"
        sudo kill -9 $i
    done
}


function CheckSudo() {
    echo -e "\033[33m +++++ CheckSudo +++++ \033[0m"

    if [ -n "$1" ];then
        echo "$1" | sudo -S echo -e "\033[32m[ CheckSudo ] OK!\033[0m" || \
        { echo -e "\033[31m +++++ CheckSudo Failed +++++ \033[0m"; return 1; }
    else
        sudo echo -e "\033[32m[ CheckSudo ] OK!\033[0m" || \
        { echo -e "\033[31m +++++ CheckSudo Failed +++++ \033[0m"; return 1; }
    fi

    return 0
}

trap 'onCtrlC' INT
function onCtrlC() {
    CheckSudo $password
    kill web.py
    kill Rookie
    kill socketimage.py
    exit 0
}

CRTDIR=$(pwd)

CheckSudo $1

cd $CRTDIR/yolov5
python ./socketimage.py &

sleep 3

cd $CRTDIR/bin
sudo ./Rookie &


sleep 3

cd $CRTDIR/script
sudo ./web.py &


while true; do
    sleep 10
done
