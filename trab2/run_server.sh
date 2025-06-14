#!/bin/bash

gnome-terminal --tab --title="Server" -- bash -c "./build/server $1; exec bash -i"



