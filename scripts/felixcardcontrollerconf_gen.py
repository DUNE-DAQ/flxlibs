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

# Add -h as default help option
CONTEXT_SETTINGS = dict(help_option_names=['-h', '--help'])

console = Console()

# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

import click

@click.command(context_settings=CONTEXT_SETTINGS)
@click.option('-P', '--partition-name', default="global", help="Name of the partition to use, for ERS and OPMON")
@click.option('--host-flx', default='localhost', help='Server hosing the FLX card')
@click.option('--ncards', default=1, help='Number of FLX cards in host')
@click.option('--opmon-impl', type=click.Choice(['json','cern','pocket'], case_sensitive=False),default='json', help="Info collector service implementation to use")
@click.option('--ers-impl', type=click.Choice(['local','cern','pocket'], case_sensitive=False), default='local', help="ERS destination (Kafka used for cern and pocket)")
@click.option('--pocket-url', default='127.0.0.1', help="URL for connecting to Pocket services")
@click.argument('json_dir', type=click.Path())

def cli(partition_name, host_flx, ncards, opmon_impl, ers_impl, pocket_url, json_dir):

    if exists(json_dir):
        raise RuntimeError(f"Directory {json_dir} already exists")

    console.log('Loading cardcontrollerapp config generator')
    from flxlibs.cardcontrollerapp import cardcontrollerapp_gen

    the_system = System()

    if opmon_impl == 'cern':
        info_svc_uri = "influx://opmondb.cern.ch:31002/write?db=influxdb"
    elif opmon_impl == 'pocket':
        info_svc_uri = "influx://" + pocket_url + ":31002/write?db=influxdb"
    else:
        info_svc_uri = "file://info_${APP_NAME}_${APP_PORT}.json"

    ers_settings=dict()

    if ers_impl == 'cern':
        use_kafka = True
        ers_settings["INFO"] =    "erstrace,throttle,lstdout,erskafka(monkafka.cern.ch:30092)"
        ers_settings["WARNING"] = "erstrace,throttle,lstdout,erskafka(monkafka.cern.ch:30092)"
        ers_settings["ERROR"] =   "erstrace,throttle,lstdout,erskafka(monkafka.cern.ch:30092)"
        ers_settings["FATAL"] =   "erstrace,lstdout,erskafka(monkafka.cern.ch:30092)"
    elif ers_impl == 'pocket':
        use_kafka = True
        ers_settings["INFO"] =    "erstrace,throttle,lstdout,erskafka(" + pocket_url + ":30092)"
        ers_settings["WARNING"] = "erstrace,throttle,lstdout,erskafka(" + pocket_url + ":30092)"
        ers_settings["ERROR"] =   "erstrace,throttle,lstdout,erskafka(" + pocket_url + ":30092)"
        ers_settings["FATAL"] =   "erstrace,lstdout,erskafka(" + pocket_url + ":30092)"
    else:
        use_kafka = False
        ers_settings["INFO"] =    "erstrace,throttle,lstdout"
        ers_settings["WARNING"] = "erstrace,throttle,lstdout"
        ers_settings["ERROR"] =   "erstrace,throttle,lstdout"
        ers_settings["FATAL"] =   "erstrace,lstdout"
   

    for i in (0,ncards-1):
        nickname = 'flx_card_' + host_flx + '_' + str(i*2)
        nickname = nickname.replace('-','_')
        app = cardcontrollerapp_gen.get_cardcontroller_app(nickname, i*2, host_flx)
        the_system.apps[nickname] = app

    ####################################################################
    # Application command data generation
    ####################################################################

    # Arrange per-app command data into the format used by util.write_json_files()
    app_command_datas = {
        name : make_app_command_data(the_system, app, name)
        for name,app in the_system.apps.items()
    }

    # Make boot.json config
    from daqconf.core.conf_utils import make_system_command_datas,generate_boot, write_json_files
    system_command_datas = make_system_command_datas(the_system)
    # Override the default boot.json with the one from minidaqapp
    boot = generate_boot(the_system.apps, ers_settings=ers_settings, info_svc_uri=info_svc_uri,
                              disable_trace=True, use_kafka=use_kafka)

    system_command_datas['boot'] = boot

    write_json_files(app_command_datas, system_command_datas, json_dir)

    console.log(f"FLX card controller apps config generated in {json_dir}")
    
    write_metadata_file(json_dir, "flxcardcontrollers_gen")

if __name__ == '__main__':
    try:
        cli(show_default=True, standalone_mode=True)
    except Exception as e:
        console.print_exception()
