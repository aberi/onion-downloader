#!/bin/bash
for i in $(cat $1)
do
	./client $i -r --show-response
done
