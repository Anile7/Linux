#!/bin/bash

cd /opt

g++-o kubsh src/kubsh.c -lreadline -lfuse3
chmod +x kubsh

make deb

apt update && apt install -y ./kubsh.deb

pytest -v --log-cli-level=10 .
