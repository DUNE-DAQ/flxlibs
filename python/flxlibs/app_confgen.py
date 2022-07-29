# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

# Load configuration types
import moo.otypes
moo.otypes.load_types('cmdlib/cmd.jsonnet')
moo.otypes.load_types('rcif/cmd.jsonnet')
moo.otypes.load_types('appfwk/cmd.jsonnet')
moo.otypes.load_types('appfwk/app.jsonnet')
moo.otypes.load_types('readoutlibs/readoutconfig.jsonnet')
moo.otypes.load_types('readoutlibs/recorderconfig.jsonnet')
moo.otypes.load_types('flxlibs/felixcardreader.jsonnet')
moo.otypes.load_types('flxlibs/felixcardcontroller.jsonnet')

# Import new types
import dunedaq.cmdlib.cmd as basecmd # AddressedCmd, 
import dunedaq.rcif.cmd as rccmd # AddressedCmd, 
import dunedaq.appfwk.app as app # AddressedCmd, 
import dunedaq.appfwk.cmd as cmd # AddressedCmd, 
import dunedaq.readoutlibs.readoutconfig as rconf 
import dunedaq.readoutlibs.recorderconfig as bfs
import dunedaq.flxlibs.felixcardreader as flxcr
import dunedaq.flxlibs.felixcardcontroller as flxcc
import dunedaq.iomanager.connection as conn

from appfwk.utils import mrccmd, mspec

from daqconf.core.system import System
from daqconf.core.conf_utils import make_app_command_data
from daqconf.core.metadata import write_metadata_file
from daqconf.core.app import App, ModuleGraph
from daqconf.core.conf_utils import Direction, Queue
from daqconf.core.daqmodule import DAQModule

import json
import re
from rich.console import Console
from os.path import exists
# Time to waait on pop()
QUEUE_POP_WAIT_MS=100;
# local clock speed Hz
CLOCK_SPEED_HZ = 50000000;

def parse_linkmask(string, n_links):
    # check if format is correct
    if not re.search("[a-zA-Z]", string):
        if ":" in string:
            # split string, one per SLR
            slrs = string.split(":")
            masks = []
            # parse each string
            for slr in slrs:
                mask = []
                if re.search("\d", slr):
                    links = slr.split(",")
                    for link in links:
                        r = list(map(int, link.split("-")))
                        if len(r) == 1:
                            mask.extend(r)
                        else:
                            mask.extend(range(r[0], r[-1]+1))
                    masks.append(sorted(mask))
                else:
                    masks.append([])
            if len(masks[0]) + len(masks[1]) == n_links:
                return masks
            else:
                raise Exception(f"Number of links defined in link masks ({len(masks[0])} + {len(masks[1])}) is not equal to the number of data+tp producers: {n_links}")
        else:
            raise Exception("Need to define link mask for both SLR's using \":\" e.g. \"slr1 : slr2\"")
    else:
        raise Exception("No letters allowed in link mask")

def CountTPLink(link_mask):
    if 5 in link_mask:
        return 1
    else:
        return 0

def generate(
        FRONTEND_TYPE='wib',
        NUMBER_OF_DATA_PRODUCERS=1,          
        NUMBER_OF_TP_PRODUCERS=1,      
        FELIX_ELINK_MASK="0:0",    
        DATA_RATE_SLOWDOWN_FACTOR = 1,
        EMULATOR_MODE = False,
        ENABLE_SOFTWARE_TPG=False,
        RUN_NUMBER = 333,
        SUPERCHUNK_FACTOR=12,
        EMU_FANOUT=False,
        DATA_FILE="./frames.bin",
        TPG_CHANNEL_MAP="ProtoDUNESP1ChannelMap",
        HOST="localhost"
    ):

    link_mask = parse_linkmask(FELIX_ELINK_MASK, NUMBER_OF_DATA_PRODUCERS+NUMBER_OF_TP_PRODUCERS)
    n_links_0 = len(link_mask[0])
    n_links_1 = len(link_mask[1])
    n_tp_link_0 = CountTPLink(link_mask[0])
    n_tp_link_1 = CountTPLink(link_mask[1])

    lb_size = 3*CLOCK_SPEED_HZ/(25*12*DATA_RATE_SLOWDOWN_FACTOR)
    lb_remainder = lb_size % 4096
    lb_size -= lb_remainder # ensure latency buffer size is always a multiple of 4096, so should be 4k aligned

    # Define modules and queues
    modules = []
    queues = []

    if n_links_0 > 0:
        modules += [DAQModule(name = 'flxcard_0',
                            plugin = 'FelixCardReader',
                            conf = flxcr.Conf(card_id = 0,
                                              logical_unit = 0,
                                              dma_id = 0,
                                              chunk_trailer_size = 32,
                                              dma_block_size_kb = 4,
                                              dma_memory_size_gb = 4,
                                              numa_id = 0,
                                              links_enabled = link_mask[0]))]
        modules += [DAQModule(name = f"datahandler_{idx}",
                            plugin = "DataLinkHandler",
                            conf = rconf.Conf(readoutmodelconf = rconf.ReadoutModelConf(
                                                                                    source_queue_timeout_ms= QUEUE_POP_WAIT_MS,
                                                                                    fake_trigger_flag=1,
                                                                                    timesync_connection_name=f"timesync_{idx}",
                                                                                    timesync_topic_name = "Timesync",
                                                                                    region_id = 0,
                                                                                    element_id = idx),
                                              latencybufferconf = rconf.LatencyBufferConf(
                                                                                    latency_buffer_alignment_size = 4096,
                                                                                    latency_buffer_size = lb_size,
                                                                                    region_id = 0,
                                                                                    element_id = idx),
                                              rawdataprocessorconf = rconf.RawDataProcessorConf(
                                                                                    region_id = 0,
                                                                                    element_id = idx,
                                                                                    enable_software_tpg = ENABLE_SOFTWARE_TPG,
                                                                                    emulator_mode = EMULATOR_MODE,
                                                                                    channel_map_name = TPG_CHANNEL_MAP),
                                              requesthandlerconf = rconf.RequestHandlerConf(
                                                                                    latency_buffer_size = lb_size,
                                                                                    pop_limit_pct = 0.8,
                                                                                    pop_size_pct = 0.1,
                                                                                    region_id = 0,
                                                                                    element_id = idx,
                                                                                    output_file = f"raw_output_{idx}.out",
                                                                                    stream_buffer_size = 8388608,
                                                                                    enable_raw_recording = True)
                                            )) for idx in range(min(5, n_links_0-n_tp_link_0))]
        modules += [DAQModule(name = f"datahandler_{idx}",
                            plugin = "DataLinkHandler",
                            conf = rconf.Conf(readoutmodelconf = rconf.ReadoutModelConf(
                                                                                    source_queue_timeout_ms= QUEUE_POP_WAIT_MS,
                                                                                    fake_trigger_flag=1,
                                                                                    timesync_connection_name=f"timesync_{idx}",
                                                                                    timesync_topic_name = "Timesync",
                                                                                    region_id = 0,
                                                                                    element_id = idx),
                                              latencybufferconf = rconf.LatencyBufferConf(
                                                                                    latency_buffer_alignment_size = 4096,
                                                                                    latency_buffer_size = lb_size,
                                                                                    region_id = 0,
                                                                                    element_id = idx),
                                              rawdataprocessorconf = rconf.RawDataProcessorConf(
                                                                                    region_id = 0,
                                                                                    element_id = idx,
                                                                                    enable_software_tpg = False,
                                                                                    enable_firmware_tpg = True,
                                                                                    emulator_mode = EMULATOR_MODE,
                                                                                    channel_map_name = TPG_CHANNEL_MAP),
                                              requesthandlerconf = rconf.RequestHandlerConf(
                                                                                    latency_buffer_size = lb_size,
                                                                                    pop_limit_pct = 0.8,
                                                                                    pop_size_pct = 0.1,
                                                                                    region_id = 0,
                                                                                    element_id = idx,
                                                                                    output_file = f"raw_output_{idx}.out",
                                                                                    stream_buffer_size = 8388608,
                                                                                    enable_raw_recording = True)
                                            )) for idx in range(n_links_0-1, n_links_0) if 5 in link_mask[0]]
        queues += [Queue(f'flxcard_0.output_{idx}',f"datahandler_{idx}.raw_input",f'{FRONTEND_TYPE}_link_{idx}', 100000 ) for idx in range(min(5, n_links_0-n_tp_link_0))]
        queues += [Queue(f'flxcard_0.output_{idx}',f"datahandler_{idx}.raw_input",f'raw_tp_link_{idx}', 100000 ) for idx in range(n_links_0-1, n_links_0) if 5 in link_mask[0]]
        queues += [Queue(f"datahandler_{idx}.errored_frames", 'errored_frame_consumer.input_queue', "errored_frames_q", 10000) for idx in range(min(5, n_links_0-n_tp_link_0))]
        queues += [Queue(f"datahandler_{idx}.errored_frames", 'errored_frame_consumer.input_queue', "errored_frames_q", 10000) for idx in range(n_links_0-1, n_links_0) if 5 in link_mask[0]]


    if NUMBER_OF_DATA_PRODUCERS > 5 or n_links_1 > 0:
        modules += [DAQModule(name = 'flxcard_1',
                            plugin = 'FelixCardReader',
                            conf = flxcr.Conf(card_id = 0,
                                              logical_unit = 1,
                                              dma_id = 0,
                                              chunk_trailer_size = 32,
                                              dma_block_size_kb = 4,
                                              dma_memory_size_gb = 4,
                                              numa_id = 0,
                                              links_enabled = link_mask[1]))]
        modules += [DAQModule(name = f"datahandler_{idx}",
                            plugin = "DataLinkHandler",
                            conf = rconf.Conf(readoutmodelconf = rconf.ReadoutModelConf(
                                                                                    source_queue_timeout_ms= QUEUE_POP_WAIT_MS,
                                                                                    fake_trigger_flag=1,
                                                                                    timesync_connection_name=f"timesync_{idx}",
                                                                                    timesync_topic_name = "Timesync",
                                                                                    region_id = 0,
                                                                                    element_id = idx),
                                              latencybufferconf = rconf.LatencyBufferConf(
                                                                                    latency_buffer_alignment_size = 4096,
                                                                                    latency_buffer_size = lb_size,
                                                                                    region_id = 0,
                                                                                    element_id = idx),
                                              rawdataprocessorconf = rconf.RawDataProcessorConf(
                                                                                    region_id = 0,
                                                                                    element_id = idx,
                                                                                    enable_software_tpg = ENABLE_SOFTWARE_TPG,
                                                                                    emulator_mode = EMULATOR_MODE,
                                                                                    channel_map_name = TPG_CHANNEL_MAP),
                                              requesthandlerconf = rconf.RequestHandlerConf(
                                                                                    latency_buffer_size = lb_size,
                                                                                    pop_limit_pct = 0.8,
                                                                                    pop_size_pct = 0.1,
                                                                                    region_id = 0,
                                                                                    element_id = idx,
                                                                                    output_file = f"raw_output_{idx}.out",
                                                                                    stream_buffer_size = 8388608,
                                                                                    enable_raw_recording = True)
                                            )) for idx in range(6, 6+n_links_1-n_tp_link_1)]
        modules += [DAQModule(name = f"datahandler_{idx}",
                            plugin = "DataLinkHandler",
                            conf = rconf.Conf(readoutmodelconf = rconf.ReadoutModelConf(
                                                                                    source_queue_timeout_ms= QUEUE_POP_WAIT_MS,
                                                                                    fake_trigger_flag=1,
                                                                                    timesync_connection_name=f"timesync_{idx}",
                                                                                    timesync_topic_name = "Timesync",
                                                                                    region_id = 0,
                                                                                    element_id = idx),
                                              latencybufferconf = rconf.LatencyBufferConf(
                                                                                    latency_buffer_alignment_size = 4096,
                                                                                    latency_buffer_size = lb_size,
                                                                                    region_id = 0,
                                                                                    element_id = idx),
                                              rawdataprocessorconf = rconf.RawDataProcessorConf(
                                                                                    region_id = 0,
                                                                                    element_id = idx,
                                                                                    enable_software_tpg = False,
                                                                                    enable_firmware_tpg = True,
                                                                                    emulator_mode = EMULATOR_MODE,
                                                                                    channel_map_name = TPG_CHANNEL_MAP),
                                              requesthandlerconf = rconf.RequestHandlerConf(
                                                                                    latency_buffer_size = lb_size,
                                                                                    pop_limit_pct = 0.8,
                                                                                    pop_size_pct = 0.1,
                                                                                    region_id = 0,
                                                                                    element_id = idx,
                                                                                    output_file = f"raw_output_{idx}.out",
                                                                                    stream_buffer_size = 8388608,
                                                                                    enable_raw_recording = True)
                                            )) for idx in range(5+n_links_1, 5+n_links_1+1) if 5 in link_mask[1]]
        queues += [Queue(f'flxcard_1.output_{idx}',f"datahandler_{idx}.raw_input",f'{FRONTEND_TYPE}_link_{idx}', 100000 ) for idx in range(6, 6+n_links_1-n_tp_link_1)]
        queues += [Queue(f'flxcard_1.output_{idx}',f"datahandler_{idx}.raw_input",f'raw_tp_link_{idx}', 100000 ) for idx in range(5+n_links_1, 5+n_links_1+1) if 5 in link_mask[1]]
        queues += [Queue(f"datahandler_{idx}.errored_frames", 'errored_frame_consumer.input_queue', "errored_frames_q", 10000) for idx in range(6, 6+n_links_1-n_tp_link_1)]
        queues += [Queue(f"datahandler_{idx}.errored_frames", 'errored_frame_consumer.input_queue', "errored_frames_q", 10000) for idx in range(5+n_links_1, 5+n_links_1+1) if 5 in link_mask[1]]

    modules += [DAQModule(name="timesync_consumer", plugin="TimeSyncConsumer")]
    modules += [DAQModule(name="fragment_consumer", plugin="FragmentConsumer")]

    if FRONTEND_TYPE == 'wib' and NUMBER_OF_DATA_PRODUCERS != 0:
        modules += [DAQModule(name="errored_frame_consumer", plugin="ErroredFrameConsumer")]

    # jstr = json.dumps(confcmd.pod(), indent=4, sort_keys=True)
    # print(jstr)

    # startpars = rccmd.StartParams(run=RUN_NUMBER)
    # startcmd = mrccmd("start", "CONFIGURED", "RUNNING", [
    #         ("datahandler_.*", startpars),
    #         ("flxcard_.*", startpars),
    #         ("data_recorder_.*", startpars),
    #         ("fragment_consumer", startpars)
    #     ])

    # jstr = json.dumps(startcmd.pod(), indent=4, sort_keys=True)
    # print("="*80+"\nStart\n\n", jstr)


    # stopcmd = mrccmd("stop", "RUNNING", "CONFIGURED", [
    #         ("flxcard_.*", None),
    #         ("datahandler_.*", None),
    #         ("data_recorder_.*", None),
    #         ("fragment_consumer", None)
    #     ])

    # jstr = json.dumps(stopcmd.pod(), indent=4, sort_keys=True)
    # print("="*80+"\nStop\n\n", jstr)

    # scrapcmd = mrccmd("scrap", "CONFIGURED", "INITIAL", [
    #         ("", None)
    #     ])

    # jstr = json.dumps(scrapcmd.pod(), indent=4, sort_keys=True)
    # print("="*80+"\nScrap\n\n", jstr)

    # # Create a list of commands
    # cmd_seq = [initcmd, confcmd, startcmd, stopcmd, scrapcmd]

    # record_cmd = mrccmd("record", "RUNNING", "RUNNING", [
    #     ("datahandler_.*", rconf.RecordingParams(
    #         duration=10
    #     ))
    # ])

    # jstr = json.dumps(record_cmd.pod(), indent=4, sort_keys=True)
    # print("="*80+"\nRecord\n\n", jstr)

    # cmd_seq.append(record_cmd)

    # get_reg_cmd = mrccmd("getregister", "RUNNING", "RUNNING", [
    #     ("flxcardctrl_.*", flxcc.GetRegisters(
    #         card_id=0,
    #         log_unit_id=0,
    #         reg_names=(
    #             "REG_MAP_VERSION",
    #         )
    #     ))
    # ])

    # jstr = json.dumps(get_reg_cmd.pod(), indent=4, sort_keys=True)
    # print("="*80+"\nGet Register\n\n", jstr)

    # cmd_seq.append(get_reg_cmd)

    # set_reg_cmd = mrccmd("setregister", "RUNNING", "RUNNING", [
    #     ("flxcardctrl_.*", flxcc.SetRegisters(
    #         card_id=0,
    #         log_unit_id=0,
    #         reg_val_pairs=(
    #             flxcc.RegValPair(reg_name="REG_MAP_VERSION", reg_val=0),
    #         )
    #     ))
    # ])

    # jstr = json.dumps(set_reg_cmd.pod(), indent=4, sort_keys=True)
    # print("="*80+"\nSet Register\n\n", jstr)

    # cmd_seq.append(set_reg_cmd)

    # get_bf_cmd = mrccmd("getbitfield", "RUNNING", "RUNNING", [
    #     ("flxcardctrl_.*", flxcc.GetBFs(
    #         card_id=0,
    #         log_unit_id=0,
    #         bf_names=(
    #             "REG_MAP_VERSION",
    #         )
    #     ))
    # ])

    # jstr = json.dumps(get_bf_cmd.pod(), indent=4, sort_keys=True)
    # print("="*80+"\nGet Bitfield\n\n", jstr)

    # cmd_seq.append(get_bf_cmd)

    # set_bf_cmd = mrccmd("setbitfield", "RUNNING", "RUNNING", [
    #     ("flxcardcontrol_.*", flxcc.SetBFs(
    #         card_id=0,
    #         log_unit_id=0,
    #         bf_val_pairs=(
    #             flxcc.RegValPair(reg_name="REG_MAP_VERSION", reg_val=0),
    #         )
    #     ))
    # ])

    # jstr = json.dumps(set_bf_cmd.pod(), indent=4, sort_keys=True)
    # print("="*80+"\nSet Bitfield\n\n", jstr)

    # cmd_seq.append(set_bf_cmd)

    # # Print them as json (to be improved/moved out)
    # jstr = json.dumps([c.pod() for c in cmd_seq], indent=4, sort_keys=True)
    mgraph = ModuleGraph(modules, queues=queues)
    for idx in range(n_links_0):
        mgraph.connect_modules(f"datahandler_{idx}.timesync_output", "timesync_consumer.input_queue", "timesync_q")
        mgraph.connect_modules(f"datahandler_{idx}.fragment_queue", "fragment_consumer.input_queue", "data_fragments_q", 100)
        mgraph.add_endpoint(f"requests_{idx}", f"datahandler_{idx}.request_input", Direction.IN)
        mgraph.add_endpoint(f"requests_{idx}", None, Direction.OUT) # Fake request endpoint
    for idx in range(n_links_1):
        i = 6 + idx
        mgraph.connect_modules(f"datahandler_{i}.timesync_output", "timesync_consumer.input_queue", "timesync_q")
        mgraph.connect_modules(f"datahandler_{i}.fragment_queue", "fragment_consumer.input_queue", "data_fragments_q", 100)
        mgraph.add_endpoint(f"requests_{i}", f"datahandler_{i}.request_input", Direction.IN)
        mgraph.add_endpoint(f"requests_{i}", None, Direction.OUT) # Fake request endpoint

    ru_app = App(modulegraph=mgraph, host=HOST, name="readout_app")
    return ru_app


if __name__ == '__main__':
    # Add -h as default help option
    CONTEXT_SETTINGS = dict(help_option_names=["-h", "--help"])

    console = Console()

    import click


    @click.command(context_settings=CONTEXT_SETTINGS)
    @click.option("-f", "--frontend-type", type=click.Choice(["wib", "wib2", "pds_queue", "pds_list"], case_sensitive=True), default="wib")
    @click.option("-n", "--number-of-data-producers", default=1)
    @click.option("-t", "--number-of-tp-producers", default=0)
    @click.option('-m', '--felix-elink-mask', default="0:0")
    @click.option("-s", "--data-rate-slowdown-factor", default=10)
    @click.option('-e', '--emulator-mode', is_flag=True)
    @click.option("-g", "--enable-software-tpg", is_flag=True)
    @click.option('-r', '--run-number', default=333)
    @click.option('-S', '--superchunk-factor', default=12)
    @click.option('-E', '--emu-fanout', is_flag=True)
    @click.option('-c', '--tpg-channel-map', type=click.Choice(["VDColdboxChannelMap", "ProtoDUNESP1ChannelMap"]), default="ProtoDUNESP1ChannelMap")
    @click.option("-d", "--data-file", type=click.Path(), default="./frames.bin")
    @click.option(
        "--opmon-impl",
        type=click.Choice(["json", "cern", "pocket"], case_sensitive=False),
        default="json",
        help="Info collector service implementation to use",
    )
    @click.option(
        "--ers-impl",
        type=click.Choice(["local", "cern", "pocket"], case_sensitive=False),
        default="local",
        help="ERS destination (Kafka used for cern and pocket)",
    )
    @click.option(
        "--pocket-url", default="127.0.0.1", help="URL for connecting to Pocket services"
    )
    @click.option("--host", default="localhost", help="Host to run app on")
    @click.argument("json_dir", type=click.Path())
    def cli(
        frontend_type,
        number_of_data_producers,
        number_of_tp_producers,
        felix_elink_mask,
        data_rate_slowdown_factor,
        emulator_mode,
        enable_software_tpg,
        run_number,
        superchunk_factor,
        emu_fanout,
        tpg_channel_map,
        data_file,
        opmon_impl,
        ers_impl,
        pocket_url,
        json_dir,
        host
    ):

        if exists(json_dir):
            raise RuntimeError(f"Directory {json_dir} already exists")

        console.log("Loading app_confgen config generator")
        from flxlibs import app_confgen

        the_system = System()

        if opmon_impl == "cern":
            info_svc_uri = "influx://opmondb.cern.ch:31002/write?db=influxdb"
        elif opmon_impl == "pocket":
            info_svc_uri = "influx://" + pocket_url + ":31002/write?db=influxdb"
        else:
            info_svc_uri = "file://info_${APP_NAME}_${APP_PORT}.json"

        ers_settings = dict()

        if ers_impl == "cern":
            use_kafka = True
            ers_settings[
                "INFO"
            ] = "erstrace,throttle,lstdout,erskafka(monkafka.cern.ch:30092)"
            ers_settings[
                "WARNING"
            ] = "erstrace,throttle,lstdout,erskafka(monkafka.cern.ch:30092)"
            ers_settings[
                "ERROR"
            ] = "erstrace,throttle,lstdout,erskafka(monkafka.cern.ch:30092)"
            ers_settings["FATAL"] = "erstrace,lstdout,erskafka(monkafka.cern.ch:30092)"
        elif ers_impl == "pocket":
            use_kafka = True
            ers_settings["INFO"] = (
                "erstrace,throttle,lstdout,erskafka(" + pocket_url + ":30092)"
            )
            ers_settings["WARNING"] = (
                "erstrace,throttle,lstdout,erskafka(" + pocket_url + ":30092)"
            )
            ers_settings["ERROR"] = (
                "erstrace,throttle,lstdout,erskafka(" + pocket_url + ":30092)"
            )
            ers_settings["FATAL"] = "erstrace,lstdout,erskafka(" + pocket_url + ":30092)"
        else:
            use_kafka = False
            ers_settings["INFO"] = "erstrace,throttle,lstdout"
            ers_settings["WARNING"] = "erstrace,throttle,lstdout"
            ers_settings["ERROR"] = "erstrace,throttle,lstdout"
            ers_settings["FATAL"] = "erstrace,lstdout"

        # add app
        the_system.apps["readout_app"] = generate(
        FRONTEND_TYPE = frontend_type,
        NUMBER_OF_DATA_PRODUCERS = number_of_data_producers,
        NUMBER_OF_TP_PRODUCERS = number_of_tp_producers,
        FELIX_ELINK_MASK = felix_elink_mask,
        DATA_RATE_SLOWDOWN_FACTOR = data_rate_slowdown_factor,
        EMULATOR_MODE = emulator_mode,
        ENABLE_SOFTWARE_TPG = enable_software_tpg,
        RUN_NUMBER = run_number,
        SUPERCHUNK_FACTOR = superchunk_factor,
        EMU_FANOUT = emu_fanout,
        DATA_FILE = data_file,
        TPG_CHANNEL_MAP = tpg_channel_map,
        HOST=host)


        ####################################################################
        # Application command data generation
        ####################################################################

        # Arrange per-app command data into the format used by util.write_json_files()
        app_command_datas = {
            name: make_app_command_data(the_system, app, name)
            for name, app in the_system.apps.items()
        }

        # Make boot.json config
        from daqconf.core.conf_utils import (
            make_system_command_datas,
            generate_boot_common,

            write_json_files,
        )

        system_command_datas = make_system_command_datas(the_system)

        boot = generate_boot_common(
            ers_settings = ers_settings,
            info_svc_uri = info_svc_uri,
            disable_trace = False,
            use_kafka = use_kafka,
            external_connections = [],
            daq_app_exec_name = "daq_application_ssh",
            verbose = False,
        )

        from daqconf.core.conf_utils import update_with_ssh_boot_data
        console.log("Generating ssh boot.json")
        update_with_ssh_boot_data(
            boot_data = boot,
            apps = the_system.apps,
            base_command_port = 3333,
            verbose = False,
        )

        system_command_datas["boot"] = boot

        write_json_files(app_command_datas, system_command_datas, json_dir, verbose=True)

        console.log(f"Readout app config generated in {json_dir}")

        write_metadata_file(json_dir, "readoutapp_gen")


    if __name__ == "__main__":
        try:
            cli(show_default=True, standalone_mode=True)
        except Exception as e:
            console.print_exception()
    ()
