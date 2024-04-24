#!/bin/bash

docker start TPEARQUI
docker exec -w /root/Toolchain -it TPEARQUI make clean all
docker exec -w /root/ -it TPEARQUI make clean all
docker stop TPEARQUI
