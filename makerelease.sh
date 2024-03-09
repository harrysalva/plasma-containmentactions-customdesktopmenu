#!/bin/bash

if [ "${1}" == "" ]; then
	echo "Error: No version provided"
	echo "./makerelease 0.0.1"
	exit 1
fi

ssh -T git@github.com
if [ ! "${?}" = "1" ]; then
  echo "No Github ssh key loaded exiting..."
  exit 1
fi

clear
branch=$(git rev-parse --abbrev-ref HEAD)
version=${1}
read -r -p "Current branch is ${branch}. Continue ? (y/N)" choice
case "${choice}" in 
  n|N|'' )
    echo "cancel build !"
    exit 1
  ;;
  y|Y ) echo "Begin Build Release...";;
  * )
    echo "cancel build !"
    exit 1
  ;;
esac
echo "Building version $version..."
echo ""

if [ ! "${branch}" = "master" ]; then
	echo "Merge branch to master..."
	git checkout master
	git merge "${branch}"
	git push
fi

read -r -p "Tag version ? (Y/n)" choice
case "${choice}" in 
  n|N )
    exit 1
  ;;
esac

echo "Tag version..."
git tag -a "v${version}" -m "Version ${version}"
git push --tags

if [ ! "${branch}" = "master" ]; then
	git checkout "${branch}"
fi
