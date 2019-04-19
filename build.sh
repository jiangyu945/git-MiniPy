#!/bin/sh

make clean
make
arm-none-linux-gnueabi-strip MiniPy
mv MiniPy /home/jiangyu/Linux+QtE5.7/root/app/
