#! /bin/bash

LIMIT=$1

echo "
************ OS 2차과제 테스트 스크립트 ************

이 스크립트는 서버와 클라이언트, 셸 클라이언트의 접속을
자동으로 하도록 도와주는 스크립트입니다.

"
echo "shell: 테스트 디렉토리 생성"

for ((a=0; a < LIMIT ; a++))
do
    if [ -d "./client_$a" ] ; then
        rm -rf "./client_$a"
    fi
    echo "mkdir ./client_$a"
    mkdir "./client_$a"
    cp "./client_p" "./client_$a/client_p"
done

if [ -d "./server" ] ; then
    rm -rf "./server"
fi
mkdir "./server"
cp "./server_p" "./server/server_p"

if [ -d "./shell" ] ; then
    rm -rf "./shell"
fi
mkdir "./shell"
cp "./client_p" "./shell/client_p"
cp "./test.txt" "./shell/test.txt"

echo "shell: 서버 생성"
(
	cd ./server
	./server_p
) &
sleep 1

echo "shell: 일반 client 접속 시작"
for ((a=0; a < LIMIT ; a++))
do
    (
    	cd ./client_$a
    	./client_p > /dev/null
    ) &
    sleep 0.5
done
echo "shell: 일반 client 접속 끝"

echo "shell: 셸 클라이언트 접속 시작"

(
	cd ./shell
	./client_p shell
)

wait
sleep 1
echo "shell: 테스트 완료"

sleep 2
echo "shell: send kill signal"
kill -9 -$(ps -o pgid= -p $$ | tr -d ' ')
