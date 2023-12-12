#include "libsegy.h"
#include <stdlib.h>    // malloc, free
#include <string.h>    // memcpy
#include <math.h>      // pow
#include <unistd.h>    // read, write, lseek, close
#include <sys/types.h> // open, lseek
#include <sys/stat.h>  // open
#include <fcntl.h>     // open
#include <stdio.h>     // sprintf


#ifdef BUILD_DLL
#include <windows.h>
// DLL Init function
BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
  return TRUE;
}
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

static char _segy_ebcdic_table[16][16] = {
  {0,0,0,0, ' ', '&', '-', 248, 216, 176, 181, '^', '{', '}','\\', '0'},
  {0,0,0,0, 127, 233, '/', 201, 'a', 'j', '~', 163, 'A', 'J', 247, '1'},
  {0,0,0,0, 226, 234, 194, 202, 'b', 'k', 's', 165, 'B', 'K', 'S', '2'},
  {0,0,0,0, 228, 235, 196, 203, 'c', 'l', 't', 183, 'C', 'L', 'T', '3'},
  {0,0,0,0, 224, 232, 192, 200, 'd', 'm', 'u', 169, 'D', 'M', 'U', '4'},
  {0,0,0,0, 225, 237, 193, 205, 'e', 'n', 'v', 167, 'E', 'N', 'V', '5'},
  {0,0,0,0, 227, 238, 195, 206, 'f', 'o', 'w', 182, 'F', 'O', 'W', '6'},
  {0,0,0,0, 229, 239, 197, 207, 'g', 'p', 'x', 188, 'G', 'P', 'X', '7'},
  {0,0,0,0, 231, 236, 199, 204, 'h', 'q', 'y', 189, 'H', 'Q', 'Y', '8'},
  {0,0,0,0, 241, 223, 209, '`', 'i', 'r', 'z', 190, 'I', 'R', 'Z', '9'},
  {0,0,0,0, 162, '!', 166, ':', 171, 170, 161, '[', 150, 185, 178, 179},
  {0,0,0,0, '.', '$', ',', '#', 187, 186, 191, ']', 244, 251, 212, 219},
  {0,0,0,0, '<', '*', '%', '@', 240, 230, 208, 175, 246, 252, 214, 220},
  {0,0,0,0, '(', ')', '_','\'', 253, 184, 221, 168, 242, 249, 210, 217},
  {0,0,0,0, '+', ';', '>', '=', 254, 198, 222, 180, 243, 250, 211, 218},
  {0,0,0,0, '|', 172, '?', '"', 177, 164, 174, 215, 245, 255, 213,0xff}};

static char _segy_err_msg[9][80] = {
  {"libsegy: Error opening file"},
  {"libsegy: Error closing file"},
  {"libsegy: Error reading from file"},
  {"libsegy: Error writing to file"},
  {"libsegy: Error seeking within file"},
  {"libsegy: Invalid file descriptor"},
  {"libsegy: Input argument(s) invalid"},
  {"libsegy: Unknown (errno) error"},
  {"libsegy: End of file reached"}
};


// try_read(fd, buf, len)
//   fd : int    = file descriptor
//   buf: char * = read buffer 
//   len: int    = bytes to read
// reads len bytes of data from fd into buf, returns bytes read or -1 on fail
static int try_read(int fd, char *buf, int len)
{
  int bytes_read = 0;
  int tries = 0;
  while (bytes_read < len && tries < 10) {
    off_t prev_pos = lseek(fd, 0, SEEK_CUR);
    int err = read(fd, buf + bytes_read, len - bytes_read);
    if (err == -1) return -1;
    if (err == 0) break;
    off_t cur_pos = lseek(fd, 0, SEEK_CUR);    
    err = cur_pos - prev_pos;
    bytes_read += err;
    tries++;
  }
  return bytes_read;
}


// try_write(fd, buf, len)
//   fd : int    = file descriptor
//   buf: char * = write buffer 
//   len: int    = bytes to read
// writes len bytes of data from buf to fd, returns bytes wrote or -1 on fail
static int try_write(int fd, char *buf, int len)
{
  int bytes_wrote = 0;
  int tries = 0;
  while (bytes_wrote < len && tries < 10) {
    off_t prev_pos = lseek(fd, 0, SEEK_CUR);
    int err = write(fd, buf + bytes_wrote, len - bytes_wrote);
    if (err == -1) return -1;
    off_t cur_pos = lseek(fd, 0, SEEK_CUR);    
    err = cur_pos - prev_pos;
    bytes_wrote += err;
    tries++;
  }
  return bytes_wrote;
}


// host_little_endian()
// returns 1 if (executing) host is little endian, 0 otherwise
static int host_little_endian()
{
  int a = 256;
  char *bytes = (char*)&a;
  if ((bytes[1] & 0x01) == 1) return 1;
  return 0;
}


// swap_int(i)
//   i: int * = pointer to int
// swaps byte-order of int between little/big endian
static void swap_int(int *i) {
  char *bytes = (char*)i;
  char buf = bytes[0];
  bytes[0] = bytes[3];
  bytes[3] = buf;
  buf = bytes[1];
  bytes[1] = bytes[2];
  bytes[2] = buf;
}


// swap_float(f)
//   f: float * = pointer to float
// swaps byte-order of float between little/big endian
static void swap_float(float *f) {
  char *bytes = (char*)f;
  char buf = bytes[0];
  bytes[0] = bytes[3];
  bytes[3] = buf;
  buf = bytes[1];
  bytes[1] = bytes[2];
  bytes[2] = buf;
}


// swap_short(s)
//   s: short * = pointer to short
// swaps byte-order of short between little/big endian
static void swap_short(short *s) {
  char *bytes = (char*)s;
  char buf = bytes[0];
  bytes[0] = bytes[1];
  bytes[1] = buf;
}


// convert2ieee(format, data, data_len, buf, buf_len)
//   format  : short   = data format code, see SampleFormatCode in file header
//   data    : char *  = trace data bytes
//   data_len: int     = size of data in bytes
//   buf     : float * = buffer for output ieee data
//   buf_len : short   = size of buf (number of floats)
// converts segy rev0 data formats to ieee float format
int convert2ieee(short format, const char *data, int data_len, 
			float *buf, short buf_len)
{
  if (format != 1) return SEGY_BAD_INPUT;

  // convert each IBM float to IEEE float
  const char *ibm = data;
  char *ieee = (char*)buf;
  
  int num = buf_len;
  if (num > (data_len / 4)) num = data_len / 4;
  
  int i;
  for (i = 0; i < num; i++, ibm += 4, ieee += 4) {
    // get sign bit
    ieee[0] = ibm[0] & 0x80;
    ieee[1] = ibm[1]; 
    ieee[2] = ibm[2]; 
    ieee[3] = ibm[3];

    // get excess-64 exponent
    char cexp[4] = { '\0', '\0', '\0', (ibm[0] & 0x7f) };
    int exp = ((int*)cexp)[0];
    if (host_little_endian()) swap_int(&exp);
    exp = (exp - 64) * 4;

    // find first IBM mantissa bit that is 1 (if exists)
    int bit;
    char mask = 0x01;
    for (bit = 0; bit < 24; bit++) {
      char data = ibm[(bit/8) + 1];
      if ( data & (mask << (7-(bit%8))) ) break;
    }
    if (bit == 24) bit = -1;               // case: all mantissa bits are zero

    // do left shift on IBM mantissa to normalize (large) numbers
    int norm_shift = (bit >= 0) ? bit : 0;
    if (norm_shift) {
      int byte_shift = bit % 8;
      if (byte_shift == 0) mask = 0x00;
      else mask = 0x7f >> (7 - byte_shift);  // mask for >> sign extention

      int ieee_byte = 1;
      int ibm_byte = (bit / 8) + 1;
      while (ibm_byte < 4) {
	ieee[ieee_byte] = ibm[ibm_byte] << byte_shift;
	if (ibm_byte < 3)
	  ieee[ieee_byte] |= (ibm[ibm_byte+1] >> (8 - byte_shift)) & mask;
	ieee_byte++;
	ibm_byte++;
      }

      exp -= norm_shift;                // find normalized mantissa exponent
    }

    // do right shift on IBM mantissa to denormalize (small) numbers
    int denorm_shift = (exp >= -126) ? 0 : (-1 * exp) - 126;
    if (denorm_shift > 0 && denorm_shift < 23) {
      int byte_shift = denorm_shift % 8;
      if (byte_shift == 0) mask = 0xff;
      else mask = 0x7f >> (byte_shift - 1);

      char norm[4] = { ieee[0], ieee[1], ieee[2], ieee[3] };
      char *denorm = ieee;
      denorm[1] = 0x00; denorm[2] = 0x00; denorm[3] = 0x00;

      int norm_byte = 1;
      int denorm_start = (denorm_shift / 8) + 1;
      int denorm_byte = denorm_start;
      while (denorm_byte < 4) {
	denorm[denorm_byte] = (norm[norm_byte] >> byte_shift) & mask;
	if (denorm_byte != denorm_start)
	  denorm[denorm_byte] |= norm[norm_byte-1] << (8 - byte_shift);
	norm_byte++;
	denorm_byte++;
      }

      exp += denorm_shift;              // find denormalized mantissa exponent
    }
        
    // IEEE +/- inf
    if (exp > 128) {
      ieee[0] = ieee[0] | 0x7f;
      ieee[1] = 0x80;
      ieee[2] = 0x00;
      ieee[3] = 0x00;
    }

    // IEEE 0
    else if (exp < -126) { 
      ieee[0] = ieee[0] & 0x80;
      ieee[1] = 0x00;
      ieee[2] = 0x00;
      ieee[3] = 0x00;
    }

    // IEEE normalized numbers
    else if (exp > -126) {
      char cexp = (exp - 1) + 127;
      ieee[0] = (ieee[0] & 0x80) | ((cexp >> 1) & 0x7f);
      ieee[1] = (ieee[1] & 0x7f) | cexp << 7;
    }

    // IEEE denormalized numbers
    else {
      ieee[0] = ieee[0] & 0x80;
      ieee[1] = ((ieee[1] >> 1) & 0x7f);
      ieee[2] = ((ieee[2] >> 1) & 0x7f) | (ieee[1] << 7);
      ieee[3] = ((ieee[3] >> 1) & 0x7f) | (ieee[2] << 7);
    }    

    // convert to little endian
    float *ieee_f = (float*)ieee;
    if (host_little_endian()) swap_float(ieee_f);
  }

  return 0;
}


// convert2segy(format, data, data_len, buf, buf_len)
//   format  : short   = data format code, see SampleFormatCode in file header
//   data    : char *  = trace data bytes
//   data_len: int     = size of data in bytes
//   buf     : float * = buffer for output ieee data
//   buf_len : short   = size of buf (number of floats)
// converts ieee float format to segy rev0 format 
int convert2segy(short format, const char *data, int data_len, 
			float *buf, short buf_len)
{
  if (! (format == 1 || format == 5) ) return SEGY_BAD_INPUT;

  // find the smaller of: input buffer (to floats), output buffer
  int num = buf_len;
  if (num > data_len / 4) num = data_len / 4;

  // for IEEE format, write out big-endian IEEE data
  if (format == 5) {
    float *data_floats = (float*)data;
    int i;
    for (i = 0; i < num; i++) {
      buf[i] = data[i];
      if (host_little_endian()) swap_float(&buf[i]);
    }
    return 0;
  }

  // copy input array
  char *ibm = (char*)buf;
  char *input = (char*)malloc(sizeof(float) * num);
  char *ieee = input;
  memcpy(input, data, (sizeof(float) * num)); 
  if (host_little_endian()) {
    int i;
    float *f = (float*)input;
    for (i = 0; i < num; i++) swap_float(&f[i]);
  }

  // convert each IEEE float to IBM float
  int i;
  for (i = 0; i < num; i++, ibm += 4, ieee += 4) {
    char cexp[4] = {'\0', '\0', '\0', 
		    ((ieee[0] << 1) | ((ieee[1] >> 7) & 0x01)) };
    int exp = ((int*)cexp)[0];
    if (host_little_endian()) swap_int(&exp);
    exp -= 127;
    
    // ieee +/- Inf or NaN
    if (exp == 128) {
      ibm[0] = (ieee[0] & 0x80) | 0x7f;
      ibm[1] = 0xff; ibm[2] = 0xff; ibm[3] = 0xff;

      // check for zero mantissa
      if (ibm[1] == 0 && ibm[2] == 0 && ibm[3] == 0) ibm[0] = 0;

      continue;
    }

    // ieee normalized numbers (can destroy up to 4 least sig. mantissa bits)
    else if (exp > -127) {
      int new_exp = exp + 1;                // denormalize mantissa     
      while (new_exp % 4) new_exp++;        // find next greater power of 16
      int shift = (new_exp - exp) - 1;         
      char mask = 0xff;                     // right shifts cause sign ext.
      if (shift == 1) mask = 0x7f;
      if (shift == 2) mask = 0x3f;
      if (shift == 3) mask = 0x1f;
      if (shift == 4) mask = 0x0f;

      char cexp = (new_exp / 4) + 64;
      ibm[0] = (0x80 & ieee[0]) | cexp;
      ibm[1] = ((ieee[1] | 0x80) >> shift) & mask;
      ibm[2] = ((ieee[2] >> shift) & mask) | (ieee[1] << (8 - shift));
      ibm[3] = ((ieee[3] >> shift) & mask) | (ieee[2] << (8 - shift));

      // check for zero mantissa
      if (ibm[1] == 0 && ibm[2] == 0 && ibm[3] == 0) ibm[0] = 0;

      continue;
    }
    
    // ieee denormalized numbers (can destroy up to 2 least sig. mantissa bits)
    else {
      
      // shift left 2 with no precision loss
      if ((ieee[1] & 0xC0) == 0) {
	int new_exp = -128;
	char mask = 0x03;
	ibm[0] = (0x80 & ieee[0]) | ((char)((new_exp / 4) + 64));
	ibm[1] = (ieee[1] << 2) | ((ieee[2] >> 6) & mask);
	ibm[2] = (ieee[2] << 2) | ((ieee[3] >> 6) & mask);
	ibm[3] = ieee[3] << 2;

	// check for zero mantissa
	if (ibm[1] == 0 && ibm[2] == 0 && ibm[3] == 0) ibm[0] = 0;

	continue;
      }

      // shift right 2 with up to 2 least sig. bit loss
      else {
	int new_exp = -124;
	char mask = 0x3f;
	ibm[0] = (0x80 & ieee[0]) | ((char)((new_exp / 4) + 64));
	ibm[1] = (ieee[1] >> 2) & mask;
	ibm[2] = (ieee[1] << 6) | ((ieee[2] >> 2) & mask);
	ibm[3] = (ieee[2] << 6) | ((ieee[3] >> 2) & mask);

	// check for zero mantissa
	if (ibm[1] == 0 && ibm[2] == 0 && ibm[3] == 0) ibm[0] = 0;

	continue;
      }
    }

  }
  free(input);
  return 0;
}


// swap_file_header_bytes(header)
//   header: segy_file_header = .segy file header
// converts header bytes from big endian to little endian 
static void swap_file_header_bytes(segy_file_header *header) 
{
  int i;
  int *hi = (int*)((char*)header + 3200);

  // swap JobID to ReelNumber
  for (i = 0; i < 3; i++) swap_int(&hi[i]);

  // swap TracesPerRecord to VibratoryPolarityCode
  short *hs = (short*)((char*)header + 3212);
  for (i = 0; i < 24; i++) swap_short(&hs[i]);

  // swap RevisionNumber to ExtendedTextHeaders
  hs = (short*)((char*)header + 3500);
  for (i = 0; i < 3; i++) swap_short(&hs[i]);
}


// swap_trace_header_bytes(header)
//   header: segy_trace_header = .segy file header
// converts header bytes from big endian to little endian 
static void swap_trace_header_bytes(segy_trace_header *header) 
{
  int i;
  int *hi = (int*)((char*)header);

  // swap LineSeqNumber through CDPTraceNumber
  for (i = 0; i < 7; i++) swap_int(&hi[i]);

  // swap TraceCode to DataUse
  short *hs = (short*)((char*)header + 28);
  for (i = 0; i < 4; i++) swap_short(&hs[i]);

  // swap SourceGroupDist to GroupWaterDepth
  hi = (int*)((char*)header + 36);
  for (i = 0; i < 8; i++) swap_int(&hi[i]);

  // swap ElevationScalar and CoordinateScalar
  hs = (short*)((char*)header + 68);
  swap_short(&hs[0]); swap_short(&hs[1]);

  // swap SourceX to GroupY
  hi = (int*)((char*)header + 72);
  for (i = 0; i < 4; i++) swap_int(&hi[i]);

  // swap CoordinateUnit to Overtravel
  hs = (short*)((char*)header + 88);
  for (i = 0; i < 46; i++) swap_short(&hs[i]);

  // swap CDPEnsembleX to Shotpoint
  hi = (int*)((char*)header + 180);
  for (i = 0; i < 5; i++) swap_int(&hi[i]);

  // swap ShotpointScalar and MeasurementUnit
  hs = (short*)((char*)header + 200);
  swap_short(&hs[0]); swap_short(&hs[1]);

  // swap TransductionUnit to SourceType
  hs = (short*)((char*)header + 210);
  for (i = 0; i < 4; i++) swap_short(&hs[i]);  

  // swap SourceMeasurementUnit
  hs = (short*)((char*)header + 230);
  swap_short(&hs[0]);
}


// segy_ebcdic2ascii(buf, buf_len)
//   buf    : char * = pointer to ebcdic character data
//   buf_len: int    = length of buffer
// converts buf from ebcdic (codepage 00037) to ascii code
void segy_ebcdic2ascii(char *buf, int buf_len)
{
  int i;
  for (i = 0; i < buf_len; i++) {
    char c = buf[i];
    int hi = ((c & 0xf0) >> 4) & 0x0f;
    int lo = c & 0x0f;
    buf[i] = _segy_ebcdic_table[lo][hi];
    if (buf[i] == 0x00) buf[i] = 32;
  }
}


// segy_ascii2ebcdic(buf, buf_len)
//   buf    : char * = pointer to ebcdic character data
//   buf_len: int    = length of buffer
// converts buf from ascii code to ebcdic (codepage 00037)
void segy_ascii2ebcdic(char *buf, int buf_len)
{
  int i;
  for (i = 0; i < buf_len; i++) {
    char c = buf[i];

    int row; int col; int stop = 0;
    for (row = 0; row < 16; row++) { 
      for (col = 0; col < 16; col++) { 
	if (_segy_ebcdic_table[row][col] == c) stop = 1;
	if (stop) break;
      }
      if (stop) break;
    }

    buf[i] = ((char)col << 4) | (char)row;
  }
}


// segy_error_message(errnum)
//   errnum: int = error number
// returns string message corresponding to error number
const char * segy_error_message(int errnum)
{
  if (errnum == -1) return _segy_err_msg[0];
  if (errnum < 1 || errnum > 9) return NULL;
  return _segy_err_msg[errnum-1];
}


// segy_open_file(filename, mode)
//   filename: char * = name of .segy file to open
//   mode    : int    = file access mode, either SEGY_RDONLY or SEGY_RDWR
// returns open file descriptor of .segy file, or -1 on error
int segy_open_file(const char *filename, int mode)
{
  if (mode != SEGY_RDONLY && mode != SEGY_RDWR)
    return SEGY_OPEN_ERR;

  if (mode == SEGY_RDWR)
    return open(filename, O_RDWR | O_CREAT | O_BINARY, S_IRWXU);
  else
    return open(filename, O_RDONLY | O_BINARY);
}


// segy_close_file(fd)
//   fd: int = open file descriptor
// returns 0 on successfully closing fd, nonzero on error
int segy_close_file(int fd)
{
  if (close(fd)) return SEGY_CLOSE_ERR;
  return 0;
}


// segy_read_file_header(fd, header)
//   fd    : int = open file descriptor of .segy file
//   header: segy_file_header * = pointer to file header
// reads file header information from .segy file
int segy_read_file_header(int fd, segy_file_header *header)
{
  if (lseek(fd, 0, SEEK_SET) == (off_t)-1) return SEGY_BADF;
			  
  if (try_read(fd, (void*)header, 3600) < 3600) return SEGY_READ_ERR;

  // convert text header to ascii
  segy_ebcdic2ascii(header->TextHeader, 3200);

  // convert byteorder to little-endian if necessary
  if (host_little_endian()) swap_file_header_bytes(header);
  return 0;
}


// segy_write_file_header(fd, header)
//   fid   : int = open file descriptor of .segy file
//   header: segy_file_header * = pointer to file header
// writes file header information to .segy file
int segy_write_file_header(int fd, const segy_file_header *header)
{
  if (lseek(fd, 0, SEEK_SET) == (off_t)-1) return SEGY_BADF;

  // convert text header to ebcdic
  segy_file_header header_cpy = *header;
  segy_ascii2ebcdic(header_cpy.TextHeader, 3200);

  // convert byteorder if necessary
  if (host_little_endian()) swap_file_header_bytes(&header_cpy);

  if (try_write(fd, (char*)&header_cpy, 3600) < 3600) return SEGY_WRITE_ERR;
  return 0;
}


// segy_seek_trace(fd, tracenum)
//   fd      : int = open file descriptor of .segy file
//   tracenum: int = trace number
// seeks to beginning of given trace
int segy_seek_trace(int fd, int tracenum)
{
  off_t file_size = lseek(fd, 0, SEEK_END);
  if (file_size == (off_t)-1) return SEGY_BADF;
  off_t prev_pos = 0;

  // calculate number of bytes per trace (including trace header)
  segy_file_header fhead;
  int err;
  if ( (err = segy_read_file_header(fd, &fhead)) ) return err;

  short num_samples = fhead.ReelSamplesPerTrace;
  int bytes_per_sample = 4;
  if (fhead.SampleFormatCode == 3) bytes_per_sample = 2;
  int bytes_per_trace = 240 + (num_samples * bytes_per_sample);
  
  // fast seek if constant trace length flag is set
  if (fhead.FixedLengthTraceFlag == 1) {
    int traces_to_skip = tracenum - 1;
    off_t curr_pos = lseek(fd, bytes_per_trace * traces_to_skip, SEEK_CUR);
    if (curr_pos > file_size) {
      lseek(fd, prev_pos, SEEK_SET);
      return SEGY_SEEK_ERR;
    }
  }

  // slower seek if trace length is variable for each trace
  else {
    int curr_trace = 1;
    while (curr_trace < tracenum) {

      // calculate this trace size
      off_t curr_pos = lseek(fd, 114, SEEK_CUR);
      if (curr_pos+2 > file_size) {
	lseek(fd, prev_pos, SEEK_SET);
	return SEGY_SEEK_ERR;
      }
      read(fd, (void*)&num_samples, 2);
      if (host_little_endian()) swap_short(&num_samples);
      int skip_bytes = (240-116) + (num_samples * bytes_per_sample);

      // skip past rest of trace
      curr_pos = lseek(fd, skip_bytes, SEEK_CUR);
      if (curr_pos > file_size) {
	lseek(fd, prev_pos, SEEK_SET);
	return SEGY_SEEK_ERR;
      }
      prev_pos = curr_pos;
      curr_trace++;
    }
  }
  return 0;
}


// segy_read_trace_header(fd, format, header)
//   fd    : int = open file descriptor for .segy file
//   format: int = sample format code
//   header: segy_trace_header * = pointer to trace header
// reads next trace header into buffers
int segy_read_trace_header(int fd, int format, segy_trace_header *header)
{
  off_t start_pos = lseek(fd, 0, SEEK_CUR);
  if (start_pos == (off_t)-1) return SEGY_BADF;

  // read and convert trace headers
  int err = try_read(fd, (char*)header, 240);
  if (err < 240) {
    lseek(fd, start_pos, SEEK_SET);
    if (err == 0) return SEGY_EOF;
    return SEGY_READ_ERR;
  }
  if (host_little_endian()) swap_trace_header_bytes(header);

  // calculate trace data size
  short num_samples = header->SamplesPerTrace;
  int bytes_per_sample = 4;
  if (format == 3) bytes_per_sample = 2;
  int trace_bytes = num_samples * bytes_per_sample;

  // seek past trace data
  lseek(fd, trace_bytes, SEEK_CUR);

  return 0;
}


// segy_read_trace(fd, format, header, buf, buf_len)
//   fd      : int = open file descriptor of .segy file
//   format  : int = sample format code
//   header  : segy_trace_header * = pointer to trace header
//   buf     : float * = pointer to trace data buffer
//   buf_len : short   = size of trace data buffer (number of floats)
// reads next trace header and data into buffers
int segy_read_trace(int fd, int format, segy_trace_header *header, 
		    float *buf, short buf_len)
{
  off_t start_pos = lseek(fd, 0, SEEK_CUR);
  if (start_pos == (off_t)-1) return SEGY_BADF;
  if (format != 1) return SEGY_BAD_INPUT;

  // read and convert trace headers
  int err = try_read(fd, (char*)header, 240);
  if (err < 240) {
    lseek(fd, start_pos, SEEK_SET);
    if (err <= 0) return SEGY_EOF;
    return SEGY_READ_ERR;
  }

  if (host_little_endian()) swap_trace_header_bytes(header);

  // calculate trace data size
  short num_samples = header->SamplesPerTrace;
  int bytes_per_sample = 4;
  if (format == 3) bytes_per_sample = 2;
  int trace_bytes = num_samples * bytes_per_sample;

  // read and convert trace data
  char *trace_data = (char*)malloc(trace_bytes);
  err = try_read(fd, trace_data, trace_bytes);
  if (err < trace_bytes) {
    lseek(fd, start_pos, SEEK_SET);
    if (err == 0) return SEGY_EOF;
    return SEGY_READ_ERR;
  }

  convert2ieee(format, trace_data, trace_bytes, buf, buf_len);
  free(trace_data); 

  return 0;
}


// segy_read_data(fd, format, samples, buf, buf_len)
//   fd      : int = open file descriptor of .segy file
//   format  : int = sample format code
//   samples : int = number of samples per each trace in .segy file
//   buf     : float * = pointer to trace data buffer
//   buf_len : short   = size of trace data buffer (number of floats)
// reads next trace data into buffers
int segy_read_data(int fd, int format, int samples, float *buf, short buf_len)
{
  off_t start_pos = lseek(fd, 0, SEEK_CUR);
  if (start_pos == (off_t)-1) return SEGY_BADF;
  if (format != 1) return SEGY_BAD_INPUT;

  // calculate trace data size
  int bytes_per_sample = 4;
  if (format == 3) bytes_per_sample = 2;
  int trace_bytes = samples * bytes_per_sample;

  // skip headers
  lseek(fd, 240, SEEK_CUR);
  
  // read and convert trace data
  char *trace_data = (char*)malloc(trace_bytes);
  int err = try_read(fd, trace_data, trace_bytes);
  if (err < trace_bytes) {
    lseek(fd, start_pos, SEEK_SET);
    if (err == 0) return SEGY_EOF;
    return SEGY_READ_ERR;
  }

  convert2ieee(format, trace_data, trace_bytes, buf, buf_len);
  free(trace_data); 

  return 0;
}


// segy_write_trace(fd, format, header, buf, buf_len)
//   fd      : int   = open file descriptor of .segy file
//   format  : short = data format, see SampleFormatCode in file header
//   header  : segy_trace_header * = pointer to trace header
//   buf     : float * = pointer to trace data buffer
//   buf_len : short   = size of trace data buffer (number of floats)
// writes trace header and data from buffers to file
int segy_write_trace(int fd, int format, const segy_trace_header *header,
		     const float *buf, short buf_len)
{
  if (fd < 0) return SEGY_BADF;
  if (lseek(fd, 0, SEEK_CUR) == (off_t)-1) return SEGY_BADF;
  if (! (format == 1 || format == 5) ) return SEGY_BAD_INPUT;

  // copy header and data, perform conversions
  segy_trace_header header_cpy = *header;
  int buf_bytes = sizeof(float) * buf_len;
  float *buf_cpy = (float*)malloc(buf_bytes);
  memcpy((void*)buf_cpy, (void*)buf, buf_bytes);

  if (host_little_endian()) swap_trace_header_bytes(&header_cpy);
  int err =convert2segy(format, (const char*)buf, buf_bytes, buf_cpy, buf_len);
  if (err != 0) return err;

  if (try_write(fd, (char*)&header_cpy, 240) < 240) return SEGY_WRITE_ERR;
  if (try_write(fd, (char*)buf_cpy, buf_bytes) < buf_bytes) return SEGY_WRITE_ERR;

  free(buf_cpy);
  return 0;
}

