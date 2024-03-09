#!/bin/bash

BUILDDIR=../_build

echo "cmake prepare ..."
if cmake --fresh -B ${BUILDDIR} .; then
	echo ""
	echo "begin make ..."
	cd ${BUILDDIR}
	make clean
	if make; then
		echo ""
		ls -lah bin/plasma/containmentactions/
	else
		exit 1
	fi
else
	echo ""
	echo "cmake error"
	exit 1
fi
