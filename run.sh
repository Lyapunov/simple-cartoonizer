#!/bin/sh
set -x
time ./app/cartoonizer testimage.jpg cartoon.png 8 0.5 0.28 8 45
time ./app/cartoonizer testimage2.jpg cartoon2.png 8 0.5 0.28 8 45
time ./app/cartoonizer testimage3.jpg cartoon3.png 8 0.5 0.28 8 45
