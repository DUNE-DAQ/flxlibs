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
server = SimpleXMLRPCServer(("0.0.0.0", 9999),requestHandler=RequestHandler)
server.register_introspection_functions()

class FelixFuncs:

  def heartbeat(self):
    return "OK"

  def init(self, card=0):
    card = "-d "+str(card)
    ret1 = call([pathBase+"flxcard/flx-init", card])
    ret2 = call([pathBase+"flxcard/flx-reset", card, "GTH"])
    # automatic link recovery (sponzio)
    ret3 = call([pathBase+"flxcard/flx-config", card, "set", "GBT_CHANNEL_DISABLE=0x7DF"])
    return ret1, ret2, ret3

  def driverRestart(self):
    retStop = callSubp(["/etc/init.d/drivers_flx", "stop"])
    retStart = callSubp(["/etc/init.d/drivers_flx", "start"])
    return retStop, retStart

#  def setEmulator(self, ena=True):
#    if ena:
#      out, err, retC = callSubp([pathBase+"ftools/femu", "-e", "-d 0"])
#    else:
#      out, err, retC = callSubp([pathBase+"ftools/femu", "-n", "-d 0"])
#    return out, err, retC

#  def softReset(self, card=0):
#    card = "-d "+str(card)
#    out, err, retC = callSubp([pathBase+"flxcard/flx-reset", card, "SOFT_RESET"])
#    return out, err, retC

#  def driverStatus(self):
#    out, err, retC = callSubp(["/etc/init.d/drivers_flx", "status"])   
#    return out, err, retC 

  def isAnyLinkDown(self, card=0):
    card = "-d "+str(card)
    out, err, ret = callSubp([pathBase+"flxcard/flx-info", card, "gbt"])
    p = re.compile("YES|NO")
    links = p.findall(out)
    linkDown = False
    for link in links[0:5]:
        if link == 'NO':
            linkDown = linkDown or True
    for link in links[6:11]:
        if link == 'NO':
            linkDown = linkDown or True
    return linkDown, out, err, ret

  def configLinks(self, card=0):
    c0 = "-d "+str(card)
    c1 = "-d "+str(card+1)
    ret1 = call([pathBase+"flxcard/flx-config", c0, "set", "TIMEOUT_CTRL_ENABLE=0"])
    ret2 = call([pathBase+"flxcard/flx-config", c0, "set", "CR_TTC_TOHOST_TIMEOUT_ENABLE=0"])
    ret3 = call([pathBase+"flxcard/flx-config", c0, "set", "GBT_TOHOST_FANOUT_SEL=0"])
    ret4 = call([pathBase+"flxcard/flx-config", c0, "set", "CR_FM_PATH_ENA=0x1F"])
    ret5 = call([pathBase+"flxcard/flx-config", c1, "set", "TIMEOUT_CTRL_ENABLE=0"])
    ret6 = call([pathBase+"flxcard/flx-config", c1, "set", "CR_TTC_TOHOST_TIMEOUT_ENABLE=0"])
    ret7 = call([pathBase+"flxcard/flx-config", c1, "set", "GBT_TOHOST_FANOUT_SEL=0"])
    ret8 = call([pathBase+"flxcard/flx-config", c1, "set", "CR_FM_PATH_ENA=0x1F"])
    return ret1, ret2, ret3, ret4, ret5, ret6, ret7, ret8

  def disableLinks(self, card=0):
    c0 = "-d "+str(card)
    c1 = "-d "+str(card+1)
    ret1 = call([pathBase+"flxcard/flx-config", c0, "set", "CR_FM_PATH_ENA=0"])
    ret2 = call([pathBase+"flxcard/flx-config", c1, "set", "CR_FM_PATH_ENA=0"])
    return ret1, ret2
      
server.register_instance(FelixFuncs())

# Run the server's main loop
server.serve_forever()


