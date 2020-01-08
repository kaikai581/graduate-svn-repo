#!/usr/bin/python

import DybPython.Catalog as Catalog

for runnum in range(35000, 39501):
  goodruns = Catalog.good('12').appl('P13A-P').runs[runnum]
  f = open('goodrunlist_p13a', 'a')
  for item in goodruns:
    print >> f, item
  f.close()
