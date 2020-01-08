#!/usr/bin/env python
import matplotlib.pyplot as plt
import string
import numpy
from scipy.interpolate import interp1d
from matplotlib.ticker import ScalarFormatter

# The following curve doesn't resemble other people's flux!
#x238 = numpy.array([])
#y238 = numpy.array([])
#with open('U238_nu_flux.txt','r') as f:
  #while True:
    #s = f.readline()
    #if not s: break
    #linelist = string.split(s)
    #x = float(linelist[0])
    #y = float(linelist[1])
    #x238 = numpy.append(x238, x)
    #y238 = numpy.append(y238, y)
#x238smooth = numpy.linspace(x238[0], x238[-1], 1000)
#f238 = interp1d(x238, y238, kind='cubic')


# Fortunately in Bryce's Dissertation, he refers to
# P. Vogel and J. Engel Phys. Rev. D39 3378, (1989).
# There comes the parametrization.
x238smooth = numpy.linspace(2, 10, 1000)
def f238(E_nu):
  return numpy.exp(.976-0.162*E_nu-0.079*E_nu*E_nu)


x235 = numpy.array([])
y235 = numpy.array([])
with open('U235_nu_flux.txt','r') as f:
  while True:
    s = f.readline()
    if not s: break
    linelist = string.split(s)
    x = float(linelist[0])
    y = float(linelist[1])
    x235 = numpy.append(x235, x)
    y235 = numpy.append(y235, y)
x235smooth = numpy.linspace(x235[0], x235[-1], 1000)
f235 = interp1d(x235, y235, kind='cubic')


x239 = numpy.array([])
y239 = numpy.array([])
with open('Pu239_nu_flux.txt','r') as f:
  while True:
    s = f.readline()
    if not s: break
    linelist = string.split(s)
    x = float(linelist[0])
    y = float(linelist[1])
    x239 = numpy.append(x239, x)
    y239 = numpy.append(y239, y)
x239smooth = numpy.linspace(x239[0], x239[-1], 1000)
f239 = interp1d(x239, y239, kind='cubic')


x241 = numpy.array([])
y241 = numpy.array([])
with open('Pu241_nu_flux.txt','r') as f:
  while True:
    s = f.readline()
    if not s: break
    linelist = string.split(s)
    x = float(linelist[0])
    y = float(linelist[1])
    x241 = numpy.append(x241, x)
    y241 = numpy.append(y241, y)
x241smooth = numpy.linspace(x241[0], x241[-1], 1000)
f241 = interp1d(x241, y241, kind='cubic')


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

x_v = numpy.linspace(1.805,10,1000)
y_v = VogelCrossSection(x_v)


fig = plt.figure()
axL = fig.add_subplot(1,1,1)
axL.yaxis.set_ticks_position('left')
p1, = axL.plot(x238smooth,f238(x238smooth))
p2, = axL.plot(x235smooth,f235(x235smooth))
p3, = axL.plot(x239smooth,f239(x239smooth))
p4, = axL.plot(x241smooth,f241(x241smooth))
plt.ylabel(r'number of ${\bar{\nu}_e}$ (MeV$^{-1}$fission$^{-1}$)')
plt.xlabel(r'$E_{\bar{\nu}_e}$ (MeV)')

axR = fig.add_subplot(1,1,1, sharex=axL, frameon=False)
axR.yaxis.tick_right()
axR.yaxis.set_offset_position('right')
axR.yaxis.set_label_position("right")
axR.tick_params(axis='y', colors='cyan')
axR.yaxis.set_major_formatter(FixedOrderFormatter(-42))
axR.spines['right'].set_color('cyan')
axR.yaxis.label.set_color('cyan')
plt.ylabel(r'inverse beta decay cross section ($cm^2$)')
p5, = axR.plot(x_v,y_v,color='cyan')
l1 = plt.legend([p2, p1, p3, p4, p5], [r'$^{235}$U', r'$^{238}$U', r'$^{239}$Pu', r'$^{241}$Pu', 'cross section'], loc=9)
l1.draw_frame(False)
fig.savefig('nu_flux_cross_section.eps')
