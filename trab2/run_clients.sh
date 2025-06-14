#!/bin/bash

for ((i=1; i<=$1; i++))
do
  gnome-terminal --tab --title="Client[$i]" -- bash -c "./build/client $2 $3; exec bash -i"
done

