import xmlrpclib

flx711 = xmlrpclib.ServerProxy('http://<felix_rpc_host>:9999/RPC2')
flx712 = xmlrpclib.ServerProxy('http://<felix_rpc_host>:9999/RPC2')

print("### Check link status...")
print("  -> BNL711...")
ret = flx711.isAnyLinkDown()
print ret
if ret[0]:
  print flx711.init(True, True)
print flx711.configLinks()

print("  -> BNL712...")
ret = flx712.isAnyLinkDown()
print ret
if ret[0]:
  print flx712.init(True, True)
print flx712.configLinks()
