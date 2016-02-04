ROOT := ../../../
PWD   := $(shell pwd)

patch :
	cd ${ROOT}; patch -p1 < ${PWD}/ethernet.patch

revert :
	cd ${ROOT}; patch -p1 -R < ${PWD}/ethernet.patch
