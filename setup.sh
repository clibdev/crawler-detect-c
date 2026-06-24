#!/bin/bash

if [[ ! $1 =~ ^(build-image|build-deps)$ ]]; then
  echo 'Available arguments: build-image, build-deps'
  exit 1
fi

if [[ $1 == build-image ]]; then
  docker build -t crawler-detect-c scripts

  exit 0
fi

if [[ $1 == build-deps ]]; then
  rm -rf deps
  docker run -it --rm -v ./:/app crawler-detect-c scripts/compile.sh
  sudo chown -R $USER:$USER deps

  exit 0
fi
