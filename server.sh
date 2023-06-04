#! /bin/sh

if [ "$1" = "run" ]
then
    docker run -e .env -p 80:8080 -v $(pwd):/home/server cpp_server:dev bin/server
elif [ "$1" = "make" ]
then
    docker run -v $(pwd):/home/server cpp_server:dev make
elif [ "$1" = "build" ]
then
    docker build -t cpp_server:dev .
fi