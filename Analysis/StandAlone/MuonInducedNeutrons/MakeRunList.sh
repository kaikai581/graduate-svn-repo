#!/bin/bash
find /global/project/projectdirs/dayabay/scratch/sklin/RootFile/MuonLookupTable/MuonInducedNeutron -type f|cut -d'.' -f3|sort|uniq > runlist.txt
