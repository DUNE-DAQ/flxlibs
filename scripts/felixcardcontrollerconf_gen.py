#!/usr/bin/env python3

import json
import os
import math
import sys
import glob
import rich.traceback
from rich.console import Console
from os.path import exists, join
from daqconf.core.system import System
from daqconf.core.conf_utils import make_app_command_data
from daqconf.core.metadata import write_metadata_file
from daqconf.core.sourceid import *
from daqconf.core.config_file import generate_cli_from_schema

# Add -h as default help option
CONTEXT_SETTINGS = dict(help_option_names=['-h', '--help'])

console = Console()

# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

import click

@click.command(context_settings=CONTEXT_SETTINGS)
@generate_cli_from_schema('flxlibs/confgen.jsonnet', 'flxlibs_gen')
@click.option('--hardware-map-file', default=None, help="File containing detector hardware map for configuration to run")
@click.option('-e/-ne','--emulator-mode/--no-emulator-mode', default=None, is_flag=True, help='Emulator mode')
@click.option('--debug', default=False, is_flag=True, help="Switch to get a lot of printout and dot files")
@click.argument('json_dir', type=click.Path())
def cli(config, hardware_map_file, emulator_mode, json_dir, debug):

    if exists(json_dir):
        raise RuntimeError(f"Directory {json_dir} already exists")

    config_data = config[0]
    config_file = config[1]

    moo.otypes.load_types('flxlibs/confgen.jsonnet')
    import dunedaq.flxlibs.confgen as confgen
    moo.otypes.load_types('daqconf/confgen.jsonnet')
    import dunedaq.daqconf.confgen as daqconf

    boot = daqconf.boot(**config_data.boot)
    if debug: console.log(f"boot configuration object: {boot.pod()}")

    ## etc...
    flxconf = confgen.flxcardcontroller(**config_data.flxcardcontroller)
    if debug: console.log(f"flxconf configuration object: {flxconf.pod()}")

    if hardware_map_file is not None:
        flxconf.hardware_map_file = hardware_map_file

    if emulator_mode is not None:
        flxconf.emulator_mode = emulator_mode
    console.print(f'flxconf.emulator_mode {flxconf.emulator_mode}')
    console.log('Loading cardcontrollerapp config generator')
    from flxlibs.cardcontrollerapp import cardcontrollerapp_gen

    the_system = System() 


    # Load the hw map file here to extract ru hosts, cards, slr, links, forntend types, sourceIDs and geoIDs
    # The ru apps are determined by the combinations of hostname and card_id, the SourceID determines the 
    # DLH (with physical slr+link information), the detId acts as system_type allows to infer the frontend_type
    hw_map_service = HardwareMapService(flxconf.hardware_map_file)

    # Get the list of RU processes
    dro_infos = hw_map_service.get_all_dro_info()

    # Build card controllers based on DRO_INFO, not by parameters
    for dro_info in dro_infos:
        console.log(f"Generate controller for {dro_info.host} reading card {dro_info.card}.")
        nickname = 'flxcard_' + dro_info.host + '_card_' + str(dro_info.card)
        nickname = nickname.replace('-', '_')
        app = cardcontrollerapp_gen.get_cardcontroller_app(
            nickname = nickname,
            card_id = dro_info.card*2,
            emulator_mode = flxconf.emulator_mode,
            host = dro_info.host,
            dro_info = dro_info
        )
        the_system.apps[nickname] = app
        if boot.use_k8s:
            the_system.apps[nickname].resources = {
                "felix.cern/flx0-ctrl": "1", # requesting FLX0
            }

    ####################################################################
    # Application command data generation
    ####################################################################

    # Arrange per-app command data into the format used by util.write_json_files()
    app_command_datas = {}
    for name,app in the_system.apps.items():
        app_command_datas[name] = make_app_command_data(the_system, app, name)


    # Make boot.json config
    from daqconf.core.conf_utils import make_system_command_datas, write_json_files
    system_command_datas = make_system_command_datas(
        boot,
        the_system,
        verbose=debug,
    )
    write_json_files(app_command_datas, system_command_datas, json_dir)

    console.log(f"FLX card controller apps config generated in {json_dir}")

    write_metadata_file(json_dir, "flxcardcontrollers_gen", config_file)

if __name__ == '__main__':
    try:
        cli(show_default=True, standalone_mode=True)
    except Exception as e:
        console.print_exception()
