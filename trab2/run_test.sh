#!/bin/bash

# --- Configuration ---
# 1. The path to your application.
#    (e.g., "./my_program", "/usr/local/bin/app", etc.)
APP_TO_RUN="./build/client"

# 2. The first two arguments, which will remain the same for every run.
ARG1="192.168.18.46"
ARG2="udp"

# 3. The configuration for the third argument, which will change.
#    This script will run your program for each value from START to END.
ARG3_START=1024
ARG3_END=32768
ARG3_INCREMENT=1024

# 4. The delay in seconds before each run.
#    You can use fractions, e.g., 0.5 for half a second.
DELAY_SECONDS=1


# --- Script Logic ---
# No need to edit anything below this line.

echo "--- Batch Runner Script Started ---"
echo "Application:      $APP_TO_RUN"
echo "Static Args:      '$ARG1', '$ARG2'"
echo "Variable Arg 3:   From $ARG3_START to $ARG3_END, incrementing by $ARG3_INCREMENT"
echo "Delay Between Runs: ${DELAY_SECONDS}s"
echo "------------------------------------"

# The 'for' loop iterates from the start value to the end value.
# The `seq` command generates the sequence of numbers for us.
for ((i = ARG3_START; i <= ARG3_END; i += ARG3_INCREMENT)); do

  # This is the variable third argument for the current run.
  current_arg3=$i

  for ((j = 0; j < 3; j++)); do
    # Print a message indicating the current run number.
    echo "" # Add a blank line for readability
    echo "===> Starting run $((j + 1)) with argument $current_arg3..."
    echo "===> Dry RUNNING: $APP_TO_RUN $ARG1 $ARG2 $current_arg3"

    # --- ADDED DELAY ---
    # Pause the script for the specified number of seconds.
    echo "Pausing for $DELAY_SECONDS second(s)..."
    sleep $DELAY_SECONDS

    # Execute the application with the three arguments.
    # The script will automatically pause here and wait for your program to finish
    # before continuing to the next iteration of the loop.
    $APP_TO_RUN "$ARG1" "$ARG2" "$current_arg3"

    echo "===> FINISHED run with argument $current_arg3."
  done

done

echo ""
echo "--- All runs completed. ---"