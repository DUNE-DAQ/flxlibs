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
@click.option('--host-flx', default='localhost', help='Server hosing the FLX card')
@click.option('--ncards', default=1, help='Number of FLX cards in host')
@click.option('-e','--emulator-mode', is_flag=True, help='Emulator mode')
@click.option('--opmon-impl', type=click.Choice(['json','cern','pocket'], case_sensitive=False),default='json', help="Info collector service implementation to use")
@click.option('--ers-impl', type=click.Choice(['local','cern','pocket'], case_sensitive=False), default='local', help="ERS destination (Kafka used for cern and pocket)")
@click.option('--pocket-url', default='127.0.0.1', help="URL for connecting to Pocket services")
@click.option('--image', default="", type=str, help="Which docker image to use")
@click.option('--use-k8s', is_flag=True, default=False, help="Whether to use k8s")
@click.argument('json_dir', type=click.Path())

def cli(host_flx, ncards, emulator_mode, opmon_impl, ers_impl, pocket_url, image, use_k8s, json_dir):

    if exists(json_dir):
        raise RuntimeError(f"Directory {json_dir} already exists")

    console.log('Loading cardcontrollerapp config generator')
    from flxlibs.cardcontrollerapp import cardcontrollerapp_gen

    the_system = System()

    if opmon_impl == 'cern':
         info_svc_uri = "kafka://monkafka.cern.ch:30092/opmon"
    elif opmon_impl == 'pocket':
         info_svc_uri = "kafka://" + pocket_url + ":31002/opmon"
    else:
         info_svc_uri = "file://info_{APP_NAME}_{APP_PORT}.json"

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
        nickname = 'flxcard' + host_flx + str(i*2)
        nickname = nickname.replace('-', '')
        app = cardcontrollerapp_gen.get_cardcontroller_app(
            nickname = nickname,
            card_id = i*2,
            emulator_mode = emulator_mode,
            host = host_flx,
        )
        the_system.apps[nickname] = app

    ####################################################################
    # Application command data generation
    ####################################################################

    # Arrange per-app command data into the format used by util.write_json_files()
    app_command_datas = {}
    for name,app in the_system.apps.items():
        print(name)
        app_command_datas[name] = make_app_command_data(the_system, app, name)

    # Make boot.json config
    from daqconf.core.conf_utils import make_system_command_datas,generate_boot_common, write_json_files
    system_command_datas = make_system_command_datas(the_system)
    # Override the default boot.json with the one from minidaqapp
    boot = generate_boot_common(
        ers_settings=ers_settings,
        info_svc_uri=info_svc_uri,
        disable_trace=True,
        external_connections = [],
        daq_app_exec_name = "daq_application_ssh" if not use_k8s else "daq_application_k8s",
        use_kafka=use_kafka
    )
    base_command_port = 3333
    if use_k8s:
        from daqconf.core.conf_utils import update_with_k8s_boot_data
        console.log("Generating k8s boot.json")
        update_with_k8s_boot_data(
            boot_data = boot,
            apps = the_system.apps,
            base_command_port = base_command_port,
            boot_order = list(the_system.apps.keys()),
            image = image,
            verbose = False,
        )
    else:
        from daqconf.core.conf_utils import update_with_ssh_boot_data
        console.log("Generating ssh boot.json")
        update_with_ssh_boot_data(
            boot_data = boot,
            apps = the_system.apps,
            base_command_port = base_command_port,
            verbose = False,
        )


    system_command_datas['boot'] = boot

    write_json_files(app_command_datas, system_command_datas, json_dir)

    console.log(f"FLX card controller apps config generated in {json_dir}")

    write_metadata_file(json_dir, "flxcardcontrollers_gen")

if __name__ == '__main__':
    try:
        cli(show_default=True, standalone_mode=True)
    except Exception as e:
        console.print_exception()
