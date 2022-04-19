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

from appfwk.app import App, ModuleGraph
from appfwk.daqmodule import DAQModule
#from appfwk.conf_utils import Direction, Connection

#===============================================================================
def get_cardcontroller_app(nickname, card_id, 
                host="localhost"):
    '''
    Here an entire application controlling one physical FLX card is generated. 
    '''

    # Define modules

    modules = []

    links = [];
    lus = [];
    # Prepare standard config with 6 links per logical unit
    for i in range(6):
        links+=[flx.Link(link_id=i, enabled=True, dma_desc=0, superchunk_factor=12)]
    # Prepare standard config with 2 logical units per card
    for j in range(2):
        lus+=[flx.LogicalUnit(log_unit_id=j, emu_fanout=False, links=links)]


    modules += [DAQModule(name = nickname, 
                          plugin = 'FelixCardController',
                          conf = flx.Conf(card_id = card_id, logical_units = lus)
                             )]

    mgraph = ModuleGraph(modules)
    flx_app = App(modulegraph=mgraph, host=host, name=nickname)

    return flx_app

