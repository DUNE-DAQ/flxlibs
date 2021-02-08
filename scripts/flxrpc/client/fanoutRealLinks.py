# Roland Sipos - rsipos@cern.ch
# Dummy FELIX BoardReader commander scripts.

import xmlrpclib
import sys
import time

flx711 = xmlrpclib.ServerProxy('http://:9999/RPC2')
flx709 = xmlrpclib.ServerProxy('http://:9999/RPC2')

print("### FELIX fanouts to real links...")
print("  -> BNL711...")
out,err,retc = flx711.setEmulator(False, 0);
out,err,retc = flx711.setEmulator(False, 1);
print out
print("  -> VC709...")
out,err,retc = flx709.setEmulator(False, 0);
print out

