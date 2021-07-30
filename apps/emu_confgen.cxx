#include <iostream>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

//uint32_t g_chunk_count;

// IDLE=K28.5, SOP=K28.1, EOP=K28.6, SOB=K28.2, EOB=K28.3
const constexpr uint64_t FM_KCHAR_IDLE = (((uint64_t)1 << 32) | 0xBC);
const constexpr uint64_t FM_KCHAR_SOP  = (((uint64_t)1 << 32) | 0x3C);
const constexpr uint64_t FM_KCHAR_EOP  = (((uint64_t)1 << 32) | 0xDC);
const constexpr uint64_t FM_KCHAR_SOB  = (((uint64_t)1 << 32) | 0x5C);
const constexpr uint64_t FM_KCHAR_EOB  = (((uint64_t)1 << 32) | 0x7C);

#define CRC_WIDTH 20
#define CRC_MASK ((1 << CRC_WIDTH) - 1)
#define CRC_POLYNOM_1 0xC1ACF
#define CRC_POLYNOM_2 0x8359F
#define CRC_INITVAL 0xFFFFF
#define CHUNKHDR_SIZE 8

uint64_t
crc20(uint64_t* data, uint64_t length, bool crc_new)
{
  // Initialize
  uint64_t crc = CRC_INITVAL;
  uint64_t polynomial;
  if (crc_new)
    polynomial = CRC_POLYNOM_2;
  else
    polynomial = CRC_POLYNOM_1;

  unsigned int i, k;
  for (k = 0; k < CRC_WIDTH; ++k) {
    if ((crc & 1))
      crc = (crc >> 1) ^ ((1 << (CRC_WIDTH - 1)) | (polynomial >> 1));
    else
      crc = (crc >> 1);
  }

  // Calculate CRC
  for (i = 0; i < length; i++) {
    for (k = 1; k <= 32; k++) {
      if (crc & (1 << (CRC_WIDTH - 1)))
        crc = ((crc << 1) | ((data[i] >> (32 - k)) & 1)) ^ polynomial;
      else
        crc = ((crc << 1) | ((data[i] >> (32 - k)) & 1));
    }
    crc &= CRC_MASK;
  }

  // One more loop
  for (k = 0; k < CRC_WIDTH; k++) {
    if (crc & (1 << (CRC_WIDTH - 1)))
      crc = (crc << 1) ^ polynomial;
    else
      crc = (crc << 1);
  }
  crc &= CRC_MASK;

  return crc;
}

bool
generateFm(uint64_t* emudata,
           uint64_t emusize,
           uint32_t req_chunksize,
           uint32_t pattern_id,
           uint32_t idle_chars,
           bool random_sz,
           bool crc_new,
           bool use_streamid,
           bool add_busy,
           bool omit_one_soc,
           bool omit_one_eoc,
           bool add_crc_err)
{
  // Initialize emudata to all zeroes
  unsigned i;
  for (i = 0; i < emusize; ++i)
    emudata[i] = 0;

  // Determine the number of chunks that will fit
  // (chunk size includes 8-byte header): 2 IDLEs, SOP, chunk, EOP
  uint32_t max_chunkcnt = (emusize - 2) / (1 + req_chunksize / 4 + 1 + idle_chars);
  uint32_t index = 0;
  bool success = true;
  //g_chunk_count = max_chunkcnt;

  // Start with some IDLE symbols
  emudata[index++] = FM_KCHAR_IDLE;
  emudata[index++] = FM_KCHAR_IDLE;

  // Multiple chunks
  uint32_t next_index, chunkcntr = 0, chunksz, chunk_datasz;
  while (index < emusize && chunkcntr < max_chunkcnt) {
    if (random_sz && req_chunksize > 8) // Size not less than 8
    {
      // Determine a random (data) size to use for the next chunk
      // (here: size between req_chunksize/2 and req_chunksize,
      //  but rounded up to a multiple of 4 bytes)
      uint32_t sz = (req_chunksize + 1) / 2;
      double r = ((double)rand()) / RAND_MAX;
      double d = 0.5 * (double)(1 - (req_chunksize & 1));
      chunksz = ((sz + (uint32_t)((double)sz * r + d) + 3) / 4) * 4;
    } else {
      chunksz = req_chunksize;
    }

    // Check if the next chunk will fit
    // (chunksz includes header)
    next_index = index + (1 + chunksz / 4 + 1);
    if (next_index >= emusize) {
      // It won't fit, so forget it: from here onwards fill with IDLEs
      for (; index < emusize; ++index)
        emudata[index] = FM_KCHAR_IDLE;
      // Should exit the while-loop on the basis of the chunk counter
      // so we consider this an error...
      success = false;
      continue; // Jump to start of while-loop
    }

    // SOP
    emudata[index++] = FM_KCHAR_SOP;
    if (omit_one_soc && chunkcntr == 2)
      --index; // For testing

    // Add chunk header
    chunk_datasz = chunksz - CHUNKHDR_SIZE;
    if (use_streamid)
      emudata[index++] = ((chunkcntr & 0xFF) | // Chunk counter = StreamID
                          (chunk_datasz & 0xF00) | ((chunk_datasz & 0x0FF) << 16) | ((chunkcntr & 0xFF) << 24));
    else
      emudata[index++] = (0xAA | (chunk_datasz & 0xF00) | ((chunk_datasz & 0x0FF) << 16) | ((chunkcntr & 0xFF) << 24));
    emudata[index++] = 0x10AABB00; // ewidth=0x10=16 bits

    // Add chunk data according to 'pattern_id'
    if (pattern_id == 1)
      for (i = 0; i < chunk_datasz / 4; ++i)
        emudata[index++] = 0xAA55AA55;
    else if (pattern_id == 2)
      for (i = 0; i < chunk_datasz / 4; ++i)
        emudata[index++] = 0xFFFFFFFF;
    else if (pattern_id == 3)
      for (i = 0; i < chunk_datasz / 4; ++i)
        emudata[index++] = 0x00000000;
    else {
      unsigned int cntr = 0;
      for (i = 0; i < chunk_datasz / 4; ++i, cntr += 4)
        emudata[index++] = ((((cntr + 3) & 0xFF) << 24) | (((cntr + 2) & 0xFF) << 16) | (((cntr + 1) & 0xFF) << 8) |
                            (((cntr + 0) & 0xFF) << 0));
    }

    // EOP (+ 20-bits CRC)
    uint64_t crc = crc20(&emudata[index - chunksz / 4], chunksz / 4, crc_new);
    if (add_crc_err && chunkcntr == 3)
      ++crc; // For testing
    emudata[index++] = FM_KCHAR_EOP | (crc << 8);
    if (omit_one_eoc && chunkcntr == 2)
      --index; // For testing

    if (add_busy && chunkcntr == 0)
      emudata[index++] = FM_KCHAR_SOB;

    // A configurable number of comma symbols in between chunks
    for (i = 0; i < idle_chars; ++i)
      emudata[index++] = FM_KCHAR_IDLE;

    if (add_busy && chunkcntr == 0)
      emudata[index++] = FM_KCHAR_EOB;

    ++chunkcntr;
  }

  // Fill any remaining uninitialised array locations with IDLE symbols
  for (; index < emusize; ++index)
    emudata[index] = FM_KCHAR_IDLE;

  // We expect to have generated max_chunkcnt chunks!
  if (chunkcntr < max_chunkcnt) {
    success = false;
  }

  return success;
}

int
main(int argc, char* argv[])
{

  /*"-h", "--help", "--filename", "--emuSize", "--chunkSize", "--idles", "--pattern"*/

  const std::vector<std::string> cmdArgs = { argv,
                                             argv + argc }; // store arguments, options and flags from the command line

  // set default values
  uint32_t emusize = 1024;
  uint32_t req_chunksize = 464;
  uint32_t pattern_id = 0;
  uint32_t idle_chars = 1;
  bool random_sz = false;
  bool crc_new = true;
  bool use_streamid = false;
  bool add_busy = false;
  bool omit_one_soc = false;
  bool omit_one_eoc = false;
  bool add_crc_err = false;
  std::string filename = "emuconfigreg";

  // parse command line information
  for (unsigned j = 0; j < cmdArgs.size(); j++) { // NOLINT
    std::string arg = cmdArgs[j];
    if (arg == "-h" || arg == "--help") {
      std::cout << "used to create basic emulator configurations for the FELIX to use with flx-config. Usage: \n"
                << " -h/--help   : display help messege \n"
                << " --filename  : output configuration filename \n"
                << " --emuSize   : total number of lines of data \n"
                << " --chunkSize : chunk sie of each block of data \n"
                << " --idles     : number of idle charachters between chunks \n"
                << " --pattern   : type of data to write \n"
                << "               0 is incrimental \n"
                << "               1 sets all to 0xAA55AA55 \n"
                << "               2 sets all to 0xFFFFFFFF \n"
                << "               3 sets all to 0x00000000" << std::endl;
      exit(0);
    } else if (arg == "--filename") {
      // get input file name, then print it
      if (j >= cmdArgs.size() - 1) // check if the arguement is at the end of the line without an input
      {
        std::cout << "No file name was specified \n";
        break;
      }
      filename = cmdArgs[j + 1];
    } else if (arg == "--emuSize") {
      if (j >= cmdArgs.size() - 1) {
        std::cout << "No value was specified \n";
        break;
      }
      emusize = std::stoi(cmdArgs[j + 1]);

    } else if (arg == "--chunkSize") {
      if (j >= cmdArgs.size() - 1) {
        std::cout << "No value was specified \n";
        break;
      }
      req_chunksize = std::stoi(cmdArgs[j + 1]);

    } else if (arg == "--idles") {
      if (j >= cmdArgs.size() - 1) {
        std::cout << "No value was specified \n";
        break;
      }
      idle_chars = std::stoi(cmdArgs[j + 1]);

    } else if (arg == "--pattern") {
      if (j >= cmdArgs.size() - 1) {
        std::cout << "No value was specified \n";
        break;
      }
      pattern_id = std::stoi(cmdArgs[j + 1]);
    }
  }
  std::cout << "number of lines : " << emusize << std::endl;
  std::cout << "chunk size      : " << req_chunksize << std::endl;
  std::cout << "idle characters : " << idle_chars << std::endl;
  std::cout << "pattern type    : " << pattern_id << std::endl;

  filename += "_" + std::to_string(req_chunksize) + "_" + std::to_string(idle_chars) + "_" + std::to_string(pattern_id);
  std::cout << "output file     : " << filename << std::endl;

  uint64_t emudata[emusize];
  std::ofstream output;
  output.open(filename);

  generateFm(emudata,
             emusize,
             req_chunksize,
             pattern_id,
             idle_chars,
             random_sz,
             crc_new,
             use_streamid,
             add_busy,
             omit_one_soc,
             omit_one_eoc,
             add_crc_err);

  for (int i = 0; i < emusize; i++) {
    output << "FE_EMU_CONFIG_WRADDR=0x" << std::hex << i << std::endl;
    output << "FE_EMU_CONFIG_WRDATA=0x" << std::hex << emudata[i] << std::endl;
    output << "FE_EMU_CONFIG_WE=1" << std::endl;
    output << "FE_EMU_CONFIG_WE=0" << std::endl;
  }
  output.close();
}
