import xmlrpclib
import sys

felix = xmlrpclib.ServerProxy('http://:9999/RPC2')

print "This is the name of the script: ", sys.argv[0]
print "Number of arguments: ", len(sys.argv)
print "The arguments are: " , str(sys.argv)


print("### FELIX INIT -T 2 ###")
out,err,retc = felix.init(True, True)
print out
print err
print retc

print("### FELIX INIT ###")
out,err,retc = felix.init()
print out
print err
print retc

print("### FELIX FANOUT SELECTOR ###")
out,err,retc = felix.setEmulator(False);
print out
print err
print retc

mode=sys.argv[1]
out=""
err=""
retc=0
if mode=="ALL":
  print("### FELIX HARD RESET (ALL) ###")
  out,err,retc = felix.reset(mode);
elif mode=="SOFT_RESET":
  print("### FELIX SOFT RESET ###")
  out,err,retc = felix.reset(mode);
else:
  print("### FELIX NO RESET (provided mode is: "+mode+") ###") 
print out
print err
print retc

