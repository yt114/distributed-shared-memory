#!/bin/bash

# Written for CSE513-PA1

sudo apt install -y cmake wget git
wget -q -O cmake-linux.sh https://github.com/Kitware/CMake/releases/download/v3.17.0/cmake-3.17.0-Linux-x86_64.sh
sudo sh cmake-linux.sh -- --skip-license --prefix=/usr
sudo apt install -y build-essential autoconf libtool pkg-config
git clone --recurse-submodules -b v1.33.1 https://github.com/grpc/grpc
cd grpc
mkdir -p cmake/build
cd cmake/build
cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=/usr ../..
make -j 3
sudo make install


# Uncomment the lines below to test your installation (:
# cd ../../examples/cpp/helloworld
# mkdir -p cmake/build
# cd cmake/build
# cmake -DCMAKE_PREFIX_PATH=. ../..
# make -j 3
# ./greeter_server
