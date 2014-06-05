#!/bin/bash
FILE_NAME=results_`date +%Y_%m_%d_%H_%M_%S`
echo "Starting Experiments ($FILE_NAME)"
./leap-experiments > $FILE_NAME
