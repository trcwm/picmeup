#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright N.A. Moseley 2022

mkdir -p build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug .. 
