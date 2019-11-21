#!/usr/bin/env bash


if [ "$TRAVIS_OS_NAME" == "linux" ]; then
	 #lcov doesn't work with clang on ubuntu out-of-the-box
	 #also, coverage should be collected without optimizations

	if [ "$TARGET_CPU" == "amd64" ] && [ "$CC" == "gcc" ];
	then
      export CC=gcc-8
      export CXX=g++-8
	elif [ "$TARGET_CPU" == "x86"] &&  [ "$CC" == "gcc" ]
	then
      export CC=gcc-8
      export CXX=g++-8
 	fi
fi
