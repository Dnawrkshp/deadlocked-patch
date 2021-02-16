#!/bin/sh
# 

# add dependencies
apk add make git

## Download the source code.
REPO_URL="https://github.com/Dnawrkshp/libdl"
REPO_FOLDER="libdl"
REPO_REFERENCE="origin/main"

if test ! -d "$REPO_FOLDER"; then
	git clone $REPO_URL || exit 1
fi
cd $REPO_FOLDER && git fetch origin && git reset --hard ${REPO_REFERENCE} || exit 1
make install && cd .. && rm -rf $REPO_FOLDER || exit 2
