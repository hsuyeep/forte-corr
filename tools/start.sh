#!/bin/bash
# Script to start correlator and collator on different cores of a single node.
# pep/28Sep12
ioproc &
sleep 1;
for i = 1:$1 do
  echo "Starting reader process $i...";
  cfx&
done
