# This module facilitates the generation of FLX card controller apps

# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

# Load configuration types
import moo.otypes

moo.otypes.load_types('flxlibs/felixcardcontroller.jsonnet')

# Import new types
import dunedaq.flxlibs.felixcardcontroller as flx

from daqconf.core.app import App, ModuleGraph
from daqconf.core.daqmodule import DAQModule
from detchannelmaps._daq_detchannelmaps_py import *

#===============================================================================
def get_cardcontroller_app(
        nickname,
        card_id=0,
        emulator_mode=False,
        host="localhost",
        dro_info=None):
    '''
    Here an entire application controlling one physical FLX card is generated. 
    '''

    # Get Elink infos for every SLR on this physical card.
    slrs = {}
    for link in dro_info.links:
        if not link.dro_slr in slrs:
            slrs[link.dro_slr] = []
        slrs[link.dro_slr].append(link.dro_link)

    # Sort elinks in each SLR
    for slr in slrs:
        slrs[slr].sort()

    # RS: Not needed with HW map
    #if len(elinks) != 2:
    #    raise Exception("elinks needs to be supplied two lists, one for each logical unit.")

    # Define modules
    modules = []
    lus = []

    # Create FelixCardController plugin configs
    for slr in slrs:
        elinks = []
        for l in slrs[slr]:
            elinks.append(flx.Link(link_id=l, enabled=True, dma_desc=0, superchunk_factor=12))
        lus.append(flx.LogicalUnit(log_unit_id=slr, emu_fanout=emulator_mode, links=elinks))

    # Create modules
    modules += [DAQModule(name = nickname, 
                          plugin = 'FelixCardController',
                          conf = flx.Conf(card_id = card_id, logical_units = lus)
                             )]

    mgraph = ModuleGraph(modules)
    flx_app = App(modulegraph=mgraph, host=host, name=nickname)

    return flx_app

