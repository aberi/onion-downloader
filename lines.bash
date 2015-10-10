#!/bin/bash

echo "" > src.txt
for i in $(ls src)
do
	cat src/$i >> src.txt
done
