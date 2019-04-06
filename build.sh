#!/bin/bash

if [ -f libj.a ]; then
	echo 'libj.a already exists; doing nothing' >&2
	exit 0
fi

# Setup an isolated home directory, since J assumes we want to dump a
# bunch of stuff in ~.
DIR=/tmp/build_pyj_$RANDOM
mkdir $DIR
trap "rm -rf $DIR" exit
export HOME=$DIR

# Setup the fake home directory to look like how J expects.
mkdir $DIR/git
cp -r jsource $DIR/git/jsource
PLATFORM=$(uname | tr 'A-Z' 'a-z')
cat $DIR/git/jsource/jsrc/jversion-x.h |
	sed -E "s/jplatform\s*\"unknown\"/jplatform \"$platform\"/g" \
	> $DIR/git/jsource/jsrc/jversion.h
cat $DIR/git/jsource/make/jvars.sh |
	sed -E 's/clang/gcc/g' \
	> $DIR/jvars.sh

# Build the object files and shared libraries.
. $DIR/jvars.sh
$DIR/git/jsource/make/build_all.sh

# Build a static version of libj.
if [ $(uname -m) = "x86_64" ]; then
	ARCH=j64
else
	ARCH=j32
fi
CMD=$(cat $DIR/jbld/$ARCH/bin/build_libj.so.txt |
	      grep '\-shared' |
	      sed -E 's/-shared.*//g' |
	      cut -f 2- -d ' ')
OLD_DIR=$(pwd)
cd $DIR/jbld/jout/libj.*/$ARCH
ar r $OLD_DIR/libj.a $CMD
