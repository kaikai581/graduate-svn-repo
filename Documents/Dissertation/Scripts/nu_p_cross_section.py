#!/usr/bin/env python
import matplotlib
matplotlib.use('Agg') # exempt the need of an X server
import numpy
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter
import math

class FixedOrderFormatter(ScalarFormatter):
  """Formats axis ticks using scientific notation with a constant order of 
  magnitude"""
  def __init__(self, order_of_mag=0, useOffset=True, useMathText=True):
    self._order_of_mag = order_of_mag
    ScalarFormatter.__init__(self, useOffset=useOffset, 
                              useMathText=useMathText)
  def _set_orderOfMagnitude(self, range):
    """Over-riding this to avoid having orderOfMagnitude reset elsewhere"""
    self.orderOfMagnitude = self._order_of_mag


def VogelCrossSection(E_nu):
  m_e = 0.510998910
  M_n = 939.565378
  M_p = 938.272046
  E_e = E_nu - (M_n-M_p)
  p_e = numpy.sqrt(E_e*E_e-m_e*m_e)
  return 0.0952*1e-42*E_e*p_e


x = numpy.linspace(1,10,1000) # 1000 evenly spaced numbers
y = 9.23*(x/10.)*(x/10.)*1e-42
x_v = numpy.linspace(1.805,10,1000)
y_v = VogelCrossSection(x_v)

#pylab.show() # show the plot
fig = plt.figure()
#plt.gca().yaxis.set_major_formatter(ScalarFormatter(useMathText=True))
plt.gca().yaxis.set_major_formatter(FixedOrderFormatter(-42))
plt.plot(x,y)
plt.plot(x_v,y_v)
plt.xlabel(r'$E_{\bar{\nu}_e}$ (MeV)')
fig.savefig('nu_p_cross_section.eps')
