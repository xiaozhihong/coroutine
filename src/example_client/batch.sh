#!/bin/bash

for((i=1;i<=$1;i++));
do
    nohup ./main 1.62.213.12 8788 ./input > /dev/null 2>&1 &
done
