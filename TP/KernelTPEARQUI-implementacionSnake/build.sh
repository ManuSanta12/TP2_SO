#!/bin/bash

docker start SO
docker exec -w /root/Toolchain -it SO make clean all
docker exec -w /root/ -it SO make clean all
docker stop SO
