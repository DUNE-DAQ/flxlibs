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
import dunedaq.networkmanager.nwmgr as nwmgr

from appfwk.utils import mcmd, mrccmd, mspec

import json
import math
import re
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
        DATA_FILE="./frames.bin"
    ):

    link_mask = parse_linkmask(FELIX_ELINK_MASK, NUMBER_OF_DATA_PRODUCERS+NUMBER_OF_TP_PRODUCERS)
    n_links_0 = len(link_mask[0])
    n_links_1 = len(link_mask[1])
    n_tp_link_0 = CountTPLink(link_mask[0])
    n_tp_link_1 = CountTPLink(link_mask[1])
    # Define modules and queues
    queue_bare_specs = [
            app.QueueSpec(inst="time_sync_q", kind='FollyMPMCQueue', capacity=100),
            app.QueueSpec(inst="data_fragments_q", kind='FollyMPMCQueue', capacity=100),
            app.QueueSpec(inst="errored_chunks_q", kind='FollyMPMCQueue', capacity=100),
            app.QueueSpec(inst="errored_frames_q", kind="FollyMPMCQueue", capacity=10000),
        ] + [
            app.QueueSpec(inst=f"data_requests_{idx}", kind='FollySPSCQueue', capacity=1000)
                for idx in range(min(5, n_links_0-n_tp_link_0))
        ] + [
            app.QueueSpec(inst=f"data_requests_{idx}", kind='FollySPSCQueue', capacity=1000)
                for idx in range(6, 6+n_links_1-n_tp_link_1)
        ] + [
            app.QueueSpec(inst=f"data_requests_{idx}", kind='FollySPSCQueue', capacity=1000)
                for idx in range(n_links_0-1, n_links_0) if 5 in link_mask[0]
        ] + [
            app.QueueSpec(inst=f"data_requests_{idx}", kind='FollySPSCQueue', capacity=1000)
                for idx in range(5+n_links_1, 5+n_links_1+1) if 5 in link_mask[1]
        ] + [
            app.QueueSpec(inst=f"{FRONTEND_TYPE}_link_{idx}", kind='FollySPSCQueue', capacity=100000)
                for idx in range(min(5, n_links_0-n_tp_link_0))
        ] + [
            app.QueueSpec(inst=f"{FRONTEND_TYPE}_link_{idx}", kind='FollySPSCQueue', capacity=100000)
                for idx in range(6, 6+n_links_1-n_tp_link_1)
        ] + [
            app.QueueSpec(inst=f"raw_tp_link_{idx}", kind='FollySPSCQueue', capacity=100000)
                for idx in range(n_links_0-1, n_links_0) if 5 in link_mask[0]
        ] + [
            app.QueueSpec(inst=f"raw_tp_link_{idx}", kind='FollySPSCQueue', capacity=100000)
                for idx in range(5+n_links_1, 5+n_links_1+1) if 5 in link_mask[1]
        ] + [
            app.QueueSpec(inst=f"tp_fake_link_{idx}", kind='FollySPSCQueue', capacity=100000)
                for idx in range(n_links_0-1, n_links_0) if 5 in link_mask[0]
        ] + [
            app.QueueSpec(inst=f"tp_fake_link_{idx}", kind='FollySPSCQueue', capacity=100000)
                for idx in range(5+n_links_1, 5+n_links_1+1) if 5 in link_mask[1]
        ] + [
            app.QueueSpec(inst=f"{FRONTEND_TYPE}_recording_link_{idx}", kind='FollySPSCQueue', capacity=100000)
                for idx in range(min(5, n_links_0-n_tp_link_0))
        ] + [
            app.QueueSpec(inst=f"{FRONTEND_TYPE}_recording_link_{idx}", kind='FollySPSCQueue', capacity=100000)
                for idx in range(6, 6+n_links_1-n_tp_link_1)
        ]

    # Only needed to reproduce the same order as when using jsonnet
    queue_specs = app.QueueSpecs(sorted(queue_bare_specs, key=lambda x: x.inst))

    #! omit DataRecorder for TP's for now
    mod_specs = [
                mspec(f"datahandler_{idx}", "DataLinkHandler", [
                            app.QueueInfo(name="raw_input", inst=f"{FRONTEND_TYPE}_link_{idx}", dir="input"),
                            app.QueueInfo(name="timesync", inst="time_sync_q", dir="output"),
                            app.QueueInfo(name="requests", inst=f"data_requests_{idx}", dir="input"),
                            app.QueueInfo(name="fragment_queue", inst="data_fragments_q", dir="output"),
                            app.QueueInfo(name="raw_recording", inst=f"{FRONTEND_TYPE}_recording_link_{idx}", dir="output"),
                            app.QueueInfo(name="errored_frames", inst="errored_frames_q", dir="output"),
                            ]) for idx in range(min(5, n_links_0-n_tp_link_0))
        ] + [
                mspec(f"datahandler_{idx}", "DataLinkHandler", [
                            app.QueueInfo(name="raw_input", inst=f"{FRONTEND_TYPE}_link_{idx}", dir="input"),
                            app.QueueInfo(name="timesync", inst="time_sync_q", dir="output"),
                            app.QueueInfo(name="requests", inst=f"data_requests_{idx}", dir="input"),
                            app.QueueInfo(name="fragment_queue", inst="data_fragments_q", dir="output"),
                            app.QueueInfo(name="raw_recording", inst=f"{FRONTEND_TYPE}_recording_link_{idx}", dir="output"),
                            app.QueueInfo(name="errored_frames", inst="errored_frames_q", dir="output"),
                            ]) for idx in range(6, 6+n_links_1-n_tp_link_1)
        ] + [
                mspec(f"datahandler_{idx}", "DataLinkHandler", [
                            app.QueueInfo(name="raw_input", inst=f"raw_tp_link_{idx}", dir="input"),
                            app.QueueInfo(name="timesync", inst="time_sync_q", dir="output"),
                            app.QueueInfo(name="requests", inst=f"data_requests_{idx}", dir="input"),
                            app.QueueInfo(name="fragment_queue", inst="data_fragments_q", dir="output"),
                            app.QueueInfo(name="errored_frames", inst="errored_frames_q", dir="output"),
                            ]) for idx in range(n_links_0-1, n_links_0) if 5 in link_mask[0]
        ] + [
                mspec(f"datahandler_{idx}", "DataLinkHandler", [
                            app.QueueInfo(name="raw_input", inst=f"raw_tp_link_{idx}", dir="input"),
                            app.QueueInfo(name="timesync", inst="time_sync_q", dir="output"),
                            app.QueueInfo(name="requests", inst=f"data_requests_{idx}", dir="input"),
                            app.QueueInfo(name="fragment_queue", inst="data_fragments_q", dir="output"),
                            app.QueueInfo(name="errored_frames", inst="errored_frames_q", dir="output"),
                            ]) for idx in range(5+n_links_1, 5+n_links_1+1) if 5 in link_mask[1]
        ] + [
                mspec(f"data_recorder_{idx}", "DataRecorder", [
                            app.QueueInfo(name="raw_recording", inst=f"{FRONTEND_TYPE}_recording_link_{idx}", dir="input")
                            ]) for idx in range(min(5, n_links_0-n_tp_link_0))
        ] + [
                mspec(f"data_recorder_{idx}", "DataRecorder", [
                            app.QueueInfo(name="raw_recording", inst=f"{FRONTEND_TYPE}_recording_link_{idx}", dir="input")
                            ]) for idx in range(6, 6+n_links_1-n_tp_link_1)
        ] + [            
                mspec(f"timesync_consumer", "TimeSyncConsumer", [
                                            app.QueueInfo(name="input_queue", inst=f"time_sync_q", dir="input")
                                            ])
        ] + [
                mspec(f"fragment_consumer", "FragmentConsumer", [
                                            app.QueueInfo(name="input_queue", inst=f"data_fragments_q", dir="input")
                                            ])
        ]

    
    if n_links_0 > 0:
        mod_specs.append(mspec("flxcard_0", "FelixCardReader", [
                        app.QueueInfo(name=f"output_{idx}", inst=f"{FRONTEND_TYPE}_link_{idx}", dir="output")
                            for idx in range(min(5, n_links_0-n_tp_link_0))
                        ] + [
                        app.QueueInfo(name=f"output_{idx}", inst=f"raw_tp_link_{idx}", dir="output")
                            for idx in range(n_links_0-1, n_links_0) if 5 in link_mask[0]
                        ] + [
                        app.QueueInfo(name="errored_chunks", inst="errored_chunks_q", dir="output")
                        ]))
        mod_specs.append(mspec("flxcardctrl_0", "FelixCardController", [
                        ]))
    if NUMBER_OF_DATA_PRODUCERS > 5 or n_links_1 > 0:
        mod_specs.append(mspec("flxcard_1", "FelixCardReader", [
                        app.QueueInfo(name=f"output_{idx}", inst=f"{FRONTEND_TYPE}_link_{idx}", dir="output")
                            for idx in range(6, 6+n_links_1-n_tp_link_1)
                        ] + [
                        app.QueueInfo(name=f"output_{idx}", inst=f"raw_tp_link_{idx}", dir="output")
                            for idx in range(5+n_links_1, 5+n_links_1+1) if 5 in link_mask[1]
                        ] + [
                        app.QueueInfo(name="errored_chunks", inst="errored_chunks_q", dir="output")
                        ]))
        mod_specs.append(mspec("flxcardctrl_1", "FelixCardController", [
                        ]))

    nw_specs = [nwmgr.Connection(name="timesync", topics=["Timesync"], address="tcp://127.0.0.1:6000")]
    init_specs = app.Init(queues=queue_specs, modules=mod_specs, nwconnections=nw_specs)

    jstr = json.dumps(init_specs.pod(), indent=4, sort_keys=True)
    print(jstr)

    initcmd = rccmd.RCCommand(
        id=basecmd.CmdId("init"),
        entry_state="NONE",
        exit_state="INITIAL",
        data=init_specs
    )

    CARDID = 0

    lb_size = 3*CLOCK_SPEED_HZ/(25*12*DATA_RATE_SLOWDOWN_FACTOR)
    lb_remiander = lb_size % 4096
    lb_size -= lb_remiander # ensure latency buffer size is always a multiple of 4096, so should be 4k aligned

    confcmd = mrccmd("conf", "INITIAL", "CONFIGURED", [
                ("flxcard_0",flxcr.Conf(card_id=CARDID,
                            logical_unit=0,
                            dma_id=0,
                            chunk_trailer_size= 32,
                            dma_block_size_kb= 4,
                            dma_memory_size_gb= 4,
                            numa_id=0,
                            links_enabled=link_mask[0])),
                ("flxcard_1",flxcr.Conf(card_id=CARDID,
                            logical_unit=1,
                            dma_id=0,
                            chunk_trailer_size= 32,
                            dma_block_size_kb= 4,
                            dma_memory_size_gb= 4,
                            numa_id=0,
                            links_enabled=link_mask[1])),
                ("flxcardctrl_0",flxcc.Conf(
                            card_id=CARDID,
                            logical_units=[flxcc.LogicalUnit(
                                log_unit_id=0,
                                emu_fanout=False,
                                links=[flxcc.Link(
                                    link_id=i, 
                                    enabled=True, 
                                    dma_desc=0, 
                                    superchunk_factor=12
                                    ) for i in link_mask[0]])])),
                ("flxcardctrl_1",flxcc.Conf(
                            card_id=CARDID,
                            logical_units=[flxcc.LogicalUnit(
                                log_unit_id=1,
                                emu_fanout=False,
                                links=[flxcc.Link(
                                    link_id=i, 
                                    enabled=True, 
                                    dma_desc=0, 
                                    superchunk_factor=12
                                    ) for i in link_mask[1]])])),
            ] + [
                (f"datahandler_{idx}", rconf.Conf(
                        readoutmodelconf= rconf.ReadoutModelConf(
                            source_queue_timeout_ms= QUEUE_POP_WAIT_MS,
                            fake_trigger_flag=1,
                            timesync_connection_name="timesync",
                            region_id = 0,
                            element_id = idx,
                        ),
                        latencybufferconf= rconf.LatencyBufferConf(
                            latency_buffer_alignment_size = 4096,
                            latency_buffer_size = lb_size,
                            region_id = 0,
                            element_id = idx,
                        ),
                        rawdataprocessorconf= rconf.RawDataProcessorConf(
                            region_id = 0,
                            element_id = idx,
                            enable_software_tpg = ENABLE_SOFTWARE_TPG,
                            emulator_mode = EMULATOR_MODE,
                        ),
                        requesthandlerconf= rconf.RequestHandlerConf(
                            latency_buffer_size = lb_size,
                            pop_limit_pct = 0.8,
                            pop_size_pct = 0.1,
                            region_id = 0,
                            element_id = idx,
                            output_file = f"raw_output_{idx}.out",
                            stream_buffer_size = 8388608,
                            enable_raw_recording = True
                        )
                        )) for idx in range(min(5, n_links_0-n_tp_link_0))
            ] + [
                (f"datahandler_{idx}", rconf.Conf(
                        readoutmodelconf= rconf.ReadoutModelConf(
                            source_queue_timeout_ms= QUEUE_POP_WAIT_MS,
                            fake_trigger_flag=1,
                            timesync_connection_name="timesync",
                            region_id = 0,
                            element_id = idx,
                        ),
                        latencybufferconf= rconf.LatencyBufferConf(
                            latency_buffer_alignment_size = 4096,
                            latency_buffer_size = lb_size,
                            region_id = 0,
                            element_id = idx,
                        ),
                        rawdataprocessorconf= rconf.RawDataProcessorConf(
                            region_id = 0,
                            element_id = idx,
                            enable_software_tpg = ENABLE_SOFTWARE_TPG,
                            emulator_mode = EMULATOR_MODE,
                        ),
                        requesthandlerconf= rconf.RequestHandlerConf(
                            latency_buffer_size = lb_size,
                            pop_limit_pct = 0.8,
                            pop_size_pct = 0.1,
                            region_id = 0,
                            element_id = idx,
                            output_file = f"raw_output_{idx}.out",
                            stream_buffer_size = 8388608,
                            enable_raw_recording = True
                        )
                        )) for idx in range(6, 6+n_links_1-n_tp_link_1)
            ] + [
                (f"datahandler_{idx}", rconf.Conf(
                        readoutmodelconf= rconf.ReadoutModelConf(
                            source_queue_timeout_ms= QUEUE_POP_WAIT_MS,
                            fake_trigger_flag=1,
                            timesync_connection_name="timesync",
                            region_id = 0,
                            element_id = idx,
                        ),
                        latencybufferconf= rconf.LatencyBufferConf(
                            latency_buffer_alignment_size = 4096,
                            latency_buffer_size = lb_size,
                            region_id = 0,
                            element_id = idx,
                        ),
                        rawdataprocessorconf= rconf.RawDataProcessorConf(
                            region_id = 0,
                            element_id = idx,
                            enable_software_tpg = False,
                            emulator_mode = EMULATOR_MODE,
                        ),
                        requesthandlerconf= rconf.RequestHandlerConf(
                            latency_buffer_size = lb_size,
                            pop_limit_pct = 0.8,
                            pop_size_pct = 0.1,
                            region_id = 0,
                            element_id = idx,
                            output_file = f"raw_output_{idx}.out",
                            stream_buffer_size = 8388608,
                            enable_raw_recording = True
                        )
                        )) for idx in range(n_links_0-1, n_links_0) if 5 in link_mask[0]
            ] + [
                (f"datahandler_{idx}", rconf.Conf(
                        readoutmodelconf= rconf.ReadoutModelConf(
                            source_queue_timeout_ms= QUEUE_POP_WAIT_MS,
                            fake_trigger_flag=1,
                            timesync_connection_name="timesync",
                            region_id = 0,
                            element_id = idx,
                        ),
                        latencybufferconf= rconf.LatencyBufferConf(
                            latency_buffer_alignment_size = 4096,
                            latency_buffer_size = lb_size,
                            region_id = 0,
                            element_id = idx,
                        ),
                        rawdataprocessorconf= rconf.RawDataProcessorConf(
                            region_id = 0,
                            element_id = idx,
                            enable_software_tpg = False,
                            emulator_mode = EMULATOR_MODE,
                        ),
                        requesthandlerconf= rconf.RequestHandlerConf(
                            latency_buffer_size = lb_size,
                            pop_limit_pct = 0.8,
                            pop_size_pct = 0.1,
                            region_id = 0,
                            element_id = idx,
                            output_file = f"raw_output_{idx}.out",
                            stream_buffer_size = 8388608,
                            enable_raw_recording = True
                        )
                        )) for idx in range(5+n_links_1, 5+n_links_1+1) if 5 in link_mask[1]
            ] + [
                (f"data_recorder_{idx}", bfs.Conf(
                        output_file = f"output_{idx}.out",
                        stream_buffer_size = 8388608
                        )) for idx in range(NUMBER_OF_DATA_PRODUCERS)
            ])
    
    jstr = json.dumps(confcmd.pod(), indent=4, sort_keys=True)
    print(jstr)

    startpars = rccmd.StartParams(run=RUN_NUMBER)
    startcmd = mrccmd("start", "CONFIGURED", "RUNNING", [
            ("datahandler_.*", startpars),
            ("flxcard_.*", startpars),
            ("flxcardctrl_.*", startpars),
            ("data_recorder_.*", startpars),
            ("timesync_consumer", startpars),
            ("fragment_consumer", startpars)
        ])

    jstr = json.dumps(startcmd.pod(), indent=4, sort_keys=True)
    print("="*80+"\nStart\n\n", jstr)


    stopcmd = mrccmd("stop", "RUNNING", "CONFIGURED", [
            ("flxcard_.*", None),
            ("flxcardctrl_.*", None),
            ("datahandler_.*", None),
            ("data_recorder_.*", None),
            ("timesync_consumer", None),
            ("fragment_consumer", None)
        ])

    jstr = json.dumps(stopcmd.pod(), indent=4, sort_keys=True)
    print("="*80+"\nStop\n\n", jstr)

    scrapcmd = mrccmd("scrap", "CONFIGURED", "INITIAL", [
            ("", None)
        ])

    jstr = json.dumps(scrapcmd.pod(), indent=4, sort_keys=True)
    print("="*80+"\nScrap\n\n", jstr)

    # Create a list of commands
    cmd_seq = [initcmd, confcmd, startcmd, stopcmd, scrapcmd]

    record_cmd = mrccmd("record", "RUNNING", "RUNNING", [
        ("datahandler_.*", rconf.RecordingParams(
            duration=10
        ))
    ])

    jstr = json.dumps(record_cmd.pod(), indent=4, sort_keys=True)
    print("="*80+"\nRecord\n\n", jstr)

    cmd_seq.append(record_cmd)

    get_reg_cmd = mrccmd("getregister", "RUNNING", "RUNNING", [
        ("flxcardctrl_.*", flxcc.GetRegisters(
            card_id=0,
            log_unit_id=0,
            reg_names=(
                "REG_MAP_VERSION",
            )
        ))
    ])

    jstr = json.dumps(get_reg_cmd.pod(), indent=4, sort_keys=True)
    print("="*80+"\nGet Register\n\n", jstr)

    cmd_seq.append(get_reg_cmd)

    set_reg_cmd = mrccmd("setregister", "RUNNING", "RUNNING", [
        ("flxcardctrl_.*", flxcc.SetRegisters(
            card_id=0,
            log_unit_id=0,
            reg_val_pairs=(
                flxcc.RegValPair(reg_name="REG_MAP_VERSION", reg_val=0),
            )
        ))
    ])

    jstr = json.dumps(set_reg_cmd.pod(), indent=4, sort_keys=True)
    print("="*80+"\nSet Register\n\n", jstr)

    cmd_seq.append(set_reg_cmd)

    get_bf_cmd = mrccmd("getbitfield", "RUNNING", "RUNNING", [
        ("flxcardctrl_.*", flxcc.GetBFs(
            card_id=0,
            log_unit_id=0,
            bf_names=(
                "REG_MAP_VERSION",
            )
        ))
    ])

    jstr = json.dumps(get_bf_cmd.pod(), indent=4, sort_keys=True)
    print("="*80+"\nGet Bitfield\n\n", jstr)

    cmd_seq.append(get_bf_cmd)

    set_bf_cmd = mrccmd("setbitfield", "RUNNING", "RUNNING", [
        ("flxcardcontrol_.*", flxcc.SetBFs(
            card_id=0,
            log_unit_id=0,
            bf_val_pairs=(
                flxcc.RegValPair(reg_name="REG_MAP_VERSION", reg_val=0),
            )
        ))
    ])

    jstr = json.dumps(set_bf_cmd.pod(), indent=4, sort_keys=True)
    print("="*80+"\nSet Bitfield\n\n", jstr)

    cmd_seq.append(set_bf_cmd)

    #? is this feature depriciated?
    # gth_reset_cmd = mrccmd("gthreset", "RUNNING", "RUNNING", [
    #     ("flxcardctrl_.*", flxcc.GTHResetParams(
    #         quads=0
    #     ))
    # ])
    # jstr = json.dumps(gth_reset_cmd.pod(), indent=4, sort_keys=True)
    # print("="*80+"\nGTH Reset\n\n", jstr)
    # cmd_seq.append(gth_reset_cmd)

    # Print them as json (to be improved/moved out)
    jstr = json.dumps([c.pod() for c in cmd_seq], indent=4, sort_keys=True)
    return jstr
        
if __name__ == '__main__':
    # Add -h as default help option
    CONTEXT_SETTINGS = dict(help_option_names=['-h', '--help'])

    import click

    @click.command(context_settings=CONTEXT_SETTINGS)
    @click.option('-f', '--frontend-type', type=click.Choice(['wib', 'wib2', 'pds_queue', 'pds_list'], case_sensitive=True), default='wib')
    @click.option('-n', '--number-of-data-producers', default=2)
    @click.option('-t', '--number-of-tp-producers', default=0)
    @click.option('-m', '--felix-elink-mask', default="0:0")
    @click.option('-s', '--data-rate-slowdown-factor', default=10)
    @click.option('-e', '--emulator_mode', is_flag=True)   
    @click.option('-g', '--enable-software-tpg', is_flag=True)
    @click.option('-r', '--run-number', default=333)
    @click.option('-d', '--data-file', type=click.Path(), default='./frames.bin')
    @click.argument('json_file', type=click.Path(), default='flx_readout.json')
    def cli(frontend_type, number_of_data_producers, number_of_tp_producers, felix_elink_mask, data_rate_slowdown_factor, emulator_mode, enable_software_tpg, run_number, data_file, json_file):
        """
          JSON_FILE: Input raw data file.
          JSON_FILE: Output json configuration file.
        """

        with open(json_file, 'w') as f:
            f.write(generate(
                    FRONTEND_TYPE = frontend_type,
                    NUMBER_OF_DATA_PRODUCERS = number_of_data_producers,
                    NUMBER_OF_TP_PRODUCERS = number_of_tp_producers,
                    FELIX_ELINK_MASK = felix_elink_mask,
                    DATA_RATE_SLOWDOWN_FACTOR = data_rate_slowdown_factor,
                    EMULATOR_MODE = emulator_mode,
                    ENABLE_SOFTWARE_TPG = enable_software_tpg,
                    RUN_NUMBER = run_number,
                    DATA_FILE = data_file,
                ))

        print(f"'{json_file}' generation completed.")

    cli()
    
