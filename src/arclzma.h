#include "arcfile.h"
#include "lzma.h"
#include "tar32api.h"

class CTarArcFile_Lzma : public ITarArcFile{
public:
	CTarArcFile_Lzma();
	~CTarArcFile_Lzma();
	bool open(const char *arcfile, const char *mode, int compress_level);
	size64 read(void *buf, size64 size);
	size64 write(void *buf, size64 size);
	void close();
	int get_archive_type();
	virtual std::string get_orig_filename();

	bool open_not_file(int compress_level);

	typedef enum demand_t{
		DEMAND_NOTHING,
		DEMAND_READ,
		DEMAND_WRITE,
		DEMAND_CLOSE,
	} demand_t;
	typedef enum error_status_t{
		ES_NO_ERROR,
		ES_WARNING,
		ES_FATAL_ERROR,
	} error_status_t;
	typedef enum coder_mode_t{
		CM_NOTHING,
		CM_ENCODE,
		CM_DECODE,
	} coder_mode_t;
	// encoding -> give pre-encoded data,  decoding -> give pre-decoded data
	// arg: 
	typedef size64 (*demand_callback_t)(class CTarArcFile_Lzma *instance,
		void *buff, size64 size, demand_t demand, void *arg);


	void set_format(int format_type);
	void set_demand_callback(demand_callback_t result_callback, void *arg = NULL);

	static int check_head_format(unsigned char *buf, size_t buf_size, int format_type = -1);

private:
	bool coder_initialize(void);
	size64 coder(void *buf, size64 size);

	static size64 file_demand_callback(class CTarArcFile_Lzma *instance,
		void *buff, size64 size, demand_t demand, void *arg=NULL);
	static size64 void_demand_callback(class CTarArcFile_Lzma *instance,
		void *buff, size64 size, demand_t demand, void *arg=NULL);

	unsigned char m_io_buff[8192];

	FILE *m_fp;

	demand_callback_t m_demand_callback;
	void *m_callback_arg;

	int m_compress_level;
	lzma_stream m_strm;
	lzma_options_lzma m_opt_lzma;
	uint64_t m_memlimit;
	lzma_filter m_filters[2];
	lzma_action m_action;
	bool m_opened;

	int m_format_type;
	coder_mode_t m_mode;
	error_status_t m_error_status;
};
