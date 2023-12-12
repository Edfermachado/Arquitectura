#ifndef _libsegy_h_
#define _libsegy_h_

// libsegy function return values
#define SEGY_NO_ERROR  0
#define SEGY_OPEN_ERR  -1
#define SEGY_CLOSE_ERR 2
#define SEGY_READ_ERR  3
#define SEGY_WRITE_ERR 4
#define SEGY_SEEK_ERR  5
#define SEGY_BADF      6
#define SEGY_BAD_INPUT 7
#define SEGY_UNKNOWN   8
#define SEGY_EOF       9

// libsegy misc. constants
#define SEGY_RDONLY 0
#define SEGY_RDWR   1

typedef struct _segy_file_header {
  char TextHeader[3200];            //
  int JobID;
  int LineNumber;
  int ReelNumber;
  short TracesPerRecord;            //
  short AuxTracesPerRecord;         //
  short ReelSampleInterval;         //
  short FieldSampleInterval;
  short ReelSamplesPerTrace;        //
  short FieldSamplesPerTrace;      
  short SampleFormatCode;           //
  short CDPFold;
  short TraceSortingCode;
  short VertSumCode;
  short StartSweepFreq;
  short EndSweepFreq;
  short SweepLength;
  short SweepCode;
  short SweepTraceNumber;
  short StartSweepTaperLength;
  short EndSweepTaperLength;
  short TaperType;
  short DataTracesCorrelated;
  short BinaryGainRecovered;
  short AmpRecoverMethod;
  short MeasurementSystem;          //
  short ImpulseSignalPolarity;
  short VibratoryPolarityCode;
  // <- End SEGY-0  Start SEGY1 ->
  char  unassigned[240];
  short RevisionNumber;             //
  short FixedLengthTraceFlag;       //
  short ExtendedTextHeaders;        //
  char  unassigned2[94];
} segy_file_header;


typedef struct _segy_trace_header {
  int LineSeqNumber;                //
  int ReelSeqNumber;
  int FieldRecordNumber;            //
  int TraceNumber;                  //
  int EnergySourceNumber;           
  int CDPEnsembleNumber;
  int CDPTraceNumber;
  short TraceCode;                  //
  short VertSumTraces;
  short HorizSumTraces;
  short DataUse;
  int SourceGroupDist;
  int GroupElevation;
  int SourceElevation;
  int SourceDepth;
  int GroupDatumElevation;
  int SourceDatumElevation;
  int SourceWaterDepth;
  int GroupWaterDepth;
  short ElevationScalar;
  short CoordinateScalar;
  int SourceX;
  int SourceY;
  int GroupX;
  int GroupY;
  short CoordinateUnit;
  short WeatheringVelocity;
  short SubweatheringVelocity;
  short SourceTime;
  short GroupTime;
  short SourceCorrection;
  short GroupCorrection;
  short TotalCorrection;
  short LagTimeA;
  short LagTimeB;
  short DelayAccordingTime;
  short BruteTime;
  short MuteTime;
  short SamplesPerTrace;            //
  short SampleInterval;             //
  short GainType;
  short GainConstant;
  short InitialGain;
  short Correlated;
  short StartSweepFreq;
  short EndSweepFreq;
  short SweepLength;
  short SweepType;
  short StartSweepTaperLength;
  short EndSweepTaperLength;
  short TaperType;
  short AliasFilterFreq;
  short AliasFilterSlope;
  short NotchFilterFreq;
  short NotchFilterSlope;
  short LowCutFreq;
  short HightCutFreq;
  short LowCutSlope;
  short HighCutSlope;
  short Year;
  short DayOfYear;
  short Hour;
  short Minute;
  short Second;
  short TimeCode;
  short TraceWeightFactor;
  short GroupRollNumber;
  short GroupTraceStart;
  short GroupTraceEnd;
  short GapSize;
  short Overtravel;
  // <- End SEGY-0  Start SEGY-1 ->
  int CDPEnsembleX;
  int CDPEnsembleY;
  int InLine;
  int CrossLine;
  int Shotpoint;
  short ShotpointScalar;
  short MeasurementUnit;
  char TransductionConstant[6];
  short TransductionUnit;
  short DeviceID;
  short TimeScalar;
  short SourceType;
  char SourceDirection[6];
  char SourceMeasurement[6];
  short SourceMeasurementUnit;
  char unassigned[8];
} segy_trace_header;



// label functions for export if building DLL
#ifdef BUILD_DLL
#define EXPORT __declspec(dllexport)

// label functions for import, and declare function pointers if linking to DLL
#elif defined(LINK_DLL)
#define EXPORT __declspec(dllimport)

// don't label anything if building static or *nix library
#else
#define EXPORT
#endif



// segy_ebcdic2ascii(buf, buf_len)
//   buf    : char * = pointer to ebcdic character data
//   buf_len: int    = length of buffer
// converts buf from ebcdic (codepage 00037) to ascii code
EXPORT void segy_ebcdic2ascii(char *buf, int buf_len);


// segy_ascii2ebcdic(buf, buf_len)
//   buf    : char * = pointer to ebcdic character data
//   buf_len: int    = length of buffer
// converts buf from ascii code to ebcdic (codepage 00037)
EXPORT void segy_ascii2ebcdic(char *buf, int buf_len);


// The following functions return 0 on success and non-zero on failure

// convert2ieee(format, data, data_len, buf, buf_len)
//   format  : short   = data format code, see SampleFormatCode in file header
//   data    : char *  = trace data bytes
//   data_len: int     = size of data in bytes
//   buf     : float * = buffer for output ieee data
//   buf_len : short   = size of buf (number of floats)
// converts segy rev0 data formats to ieee float format
EXPORT int convert2ieee(short format, const char *data, int data_len, 
			float *buf, short buf_len);


// convert2segy(format, data, data_len, buf, buf_len)
//   format  : short   = data format code, see SampleFormatCode in file header
//   data    : char *  = IEEE input float array, as bytes
//   data_len: int     = size of data in bytes
//   buf     : float * = buffer for output segy data
//   buf_len : short   = size of buf (number of floats)
// converts ieee float format to segy rev0 format 
EXPORT int convert2segy(short format, const char *data, int data_len, 
			float *buf, short buf_len);


// segy_error_message(errnum)
//   errnum: int = error number
// returns string message corresponding to error number
EXPORT const char * segy_error_message(int errnum);


// segy_open_file(filename, mode)
//   filename: char * = name of .segy file to open
//   mode    : int    = file access mode, either SEGY_RDONLY or SEGY_RDWR
// returns open file descriptor of .segy file, or -1 on error
EXPORT int segy_open_file(const char *filename, int mode);


// segy_close_file(fd)
//   fd: int = open file descriptor
// returns 0 on successfully closing fd, nonzero on error
EXPORT int segy_close_file(int fd);


// segy_read_file_header(fd, header)
//   fd    : int = open file descriptor of .segy file
//   header: segy_file_header * = pointer to file header
// reads file header information from .segy file
EXPORT int segy_read_file_header(int fd, segy_file_header *header);


// segy_write_file_header(fd, header)
//   fd   : int = open file descriptor of .segy file
//   header: segy_file_header * = pointer to file header
// writes file header information to .segy file
EXPORT int segy_write_file_header(int fd, const segy_file_header *header);


// segy_seek_trace(fd, tracenum)
//   fd      : int = open file descriptor of .segy file
//   tracenum: int = trace number
// seeks to beginning of given trace
EXPORT int segy_seek_trace(int fd, int tracenum);


// segy_read_trace_header(fd, format, header)
//   fd    : int = open file descriptor for .segy file
//   format: int = sample format code
//   header: segy_trace_header * = pointer to trace header
// reads next trace header into buffers
EXPORT int segy_read_trace_header(int fd, int format, 
				  segy_trace_header *header);


// segy_read_trace(fd, format, header, buf, buf_len)
//   fd      : int = open file descriptor of .segy file
//   format  : int = sample format code
//   header  : segy_trace_header * = pointer to trace header
//   buf     : float * = pointer to trace data buffer
//   buf_len : short   = size of trace data buffer (number of floats)
// reads next trace header and data into buffers
EXPORT int segy_read_trace(int fd, int format, segy_trace_header *header, 
			   float *buf, short buf_len);


// segy_read_data(fd, format, samples, buf, buf_len)
//   fd      : int = open file descriptor of .segy file
//   format  : int = sample format code
//   samples : int = number of samples per each trace in .segy file
//   buf     : float * = pointer to trace data buffer
//   buf_len : short   = size of trace data buffer (number of floats)
// reads next trace data into buffers
EXPORT int segy_read_data(int fd, int format, int samples, 
			  float *buf, short buf_len);


// segy_write_trace(fd, format, header, buf, buf_len)
//   fd      : int   = open file descriptor of .segy file
//   format  : int   = data format, see SampleFormatCode in file header
//   header  : segy_trace_header * = pointer to trace header
//   buf     : float * = pointer to trace data buffer
//   buf_len : short   = size of trace data buffer (number of floats)
// writes trace header and data from buffers to file
EXPORT int segy_write_trace(int fd, int format, 
			    const segy_trace_header *header,
			    const float *buf, short buf_len);


#endif
