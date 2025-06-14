#!/bin/bash

gnome-terminal --tab --title="Server" -- bash -c "./build/server $2; exec bash -i"

for ((i=1; i<=$1; i++))
do
  gnome-terminal --tab --title="Client[$i]" -- bash -c "./build/client localhost $2; exec bash -i"
done

