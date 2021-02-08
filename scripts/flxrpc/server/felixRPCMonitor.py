# Roland Sipos - rsipos@cern.ch
# FELIX health checks through XMLRPC

from SimpleXMLRPCServer import SimpleXMLRPCServer
from SimpleXMLRPCServer import SimpleXMLRPCRequestHandler
from subprocess import call, Popen, PIPE
import re

pathBase="<felix_dist_dir>"

def shell_source(script):
    """Sometime you want to emulate the action of "source" in bash,
    settings some environment variables. Here is a way to do it."""
    import subprocess, os
    pipe = subprocess.Popen(". %s; env" % script, stdout=subprocess.PIPE, shell=True)
    output = pipe.communicate()[0]
    #print output
    env = dict((line.split("=", 1) for line in output.splitlines()))
    os.environ.update(env)

def callSubp(appDict):
  p = Popen(appDict, stdin=PIPE, stdout=PIPE, stderr=PIPE)
  output, err = p.communicate(b"input data that is passed to subprocess' stdin")
  return output, err, p.returncode

shell_source("<felix_dist_dir>/setup.sh")

# Restrict to a particular path.
class RequestHandler(SimpleXMLRPCRequestHandler):
    rpc_paths = ('/RPC2',)

# Create server
server = SimpleXMLRPCServer(("0.0.0.0", 10001),requestHandler=RequestHandler)
server.register_introspection_functions()

class FelixMonitors:

  def heartbeat(self):
    return "OK"

  def lspci(self, card=0):
    ret1 = callSubp(["lspci"])
    ret1Split = ret1[0].split('\n') 
    devid = [line for line in ret1Split if "FPGA Card" in line]
    if not devid:
      return " ", " ", " ", " "
    ret2 = callSubp(["lspci", "-s", devid[0].split(' ')[0], "-vvvvv"])
    ret2Split = ret2[0].split('\n')
    control = [line for line in ret2Split if "Control" in line]
    status = [line for line in ret2Split if "Status" in line]
    numa = [line for line in ret2Split if "NUMA" in line]
    return devid, control, status, numa

  def driver(self, card=0):
    ret = callSubp(["/etc/init.d/drivers_flx", "status"])
    retSplit = ret[0].split('\n')
    version = [line for line in retSplit if "FLX driver" in line]
    cmem = [line for line in retSplit if "GFPBPA" in line]
    githash = [line for line in retSplit if "GIT hash" in line]
    return version, cmem, githash

  def links(self, card=0):
    c0 = "-d "+str(card)
    c1 = "-d "+str(card+1)
    ret = callSubp([pathBase+"flxcard/flx-info", "GBT"])
    return ret
    retSplit = ret[0].split('\n')
    channel = [line for line in retSplit if "Channel" in line]
    aligned = [line for line in retSplit if "Aligned" in line]
    return channel, aligned
      
server.register_instance(FelixMonitors())

# Run the server's main loop
server.serve_forever()


