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

#===============================================================================
def get_cardcontroller_app(
        nickname,
        card_id,
        elinks = [[0,1,2,3,4,5]]*2,
        emulator_mode=False,
        host="localhost"):
    '''
    Here an entire application controlling one physical FLX card is generated. 
    '''
    if len(elinks) != 2:
        raise Exception("elinks needs to be supplied two lists, one for each logical unit.")

    # Define modules

    modules = []

    lus = []
    for i in range(2):
        links = []
        for j in elinks[i]:
            links+=[flx.Link(link_id=j, enabled=True, dma_desc=0, superchunk_factor=12)]
        lus+=[flx.LogicalUnit(log_unit_id=i, emu_fanout=emulator_mode, links=links)]


    modules += [DAQModule(name = nickname, 
                          plugin = 'FelixCardController',
                          conf = flx.Conf(card_id = card_id, logical_units = lus)
                             )]

    mgraph = ModuleGraph(modules)
    flx_app = App(modulegraph=mgraph, host=host, name=nickname)

    return flx_app

