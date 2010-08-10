#!/bin/bash

bold=`tput bold`
normal=`tput sgr0`

echo -n "${bold}Testing $@ ... ${normal}"

./opencc -i $@.in -o $@.out -c $@.ini

if [ "$?" -eq "0" ]; then
	ret=`diff $@.out $@.ans`
	if [ "$ret" = "" ]; then
		echo "${bold}passed${normal}"
	else
		echo "${bold}failed${normal}"
		echo $ret
		false
	fi
else
	false
fi
