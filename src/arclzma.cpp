#include "stdafx.h"
#include "util.h"
#include "lzma.h"
#include "arclzma.h"
#include "tar32dll.h"

CTarArcFile_Lzma::CTarArcFile_Lzma()
{
	lzma_stream initial_strm = LZMA_STREAM_INIT;
	memcpy(&m_strm,&initial_strm,sizeof(lzma_stream));
	set_demand_callback(void_demand_callback,NULL);
	m_format_type = -1;
	m_fp = NULL;
	m_mode = CM_NOTHING;
	m_compress_level = 6;
	m_memlimit = 1; m_memlimit <<= 30;  // fixed memory limit: 1G. need to calculate it.
	m_error_status = ES_NO_ERROR;
	m_opened = false;
}
CTarArcFile_Lzma::~CTarArcFile_Lzma()
{
	close();
}
bool CTarArcFile_Lzma::open(const char *arcfile, const char *mode, int compress_level)
{
	close();
	set_demand_callback(file_demand_callback,NULL);
	m_arcfile = arcfile;
	m_fp = fopen(arcfile,mode);
	if(m_fp)setvbuf(m_fp,NULL,_IOFBF,1024*1024);
	m_mode = CM_NOTHING;
	m_compress_level = compress_level;
	m_error_status = ES_NO_ERROR;
	m_opened = true;
	return m_fp!=NULL;
}
size64 CTarArcFile_Lzma::read(void *buf, size64 size)
{
	if(m_error_status != ES_NO_ERROR){
		return 0;
	}
	if(m_mode==CM_NOTHING){
		m_mode=CM_DECODE;
		if(!coder_initialize()){
			m_error_status = ES_FATAL_ERROR;
			return 0;
		}
	} else if(m_mode!=CM_DECODE){
		return 0;
	}
	return coder(buf,size);
}
size64 CTarArcFile_Lzma::write(void *buf, size64 size)
{
	if(m_error_status != ES_NO_ERROR){
		return 0;
	}
	if(m_mode==CM_NOTHING){
		m_mode=CM_ENCODE;
		if(!coder_initialize()){
			m_error_status = ES_FATAL_ERROR;
			return 0;
		}
	} else if(m_mode!=CM_ENCODE){
		return 0;
	}
	return coder(buf,size);
}
void CTarArcFile_Lzma::close()
{
	if(m_opened){
		// 残ったバッファの分を処理する
		m_action = LZMA_FINISH;
		coder(NULL,0);
		lzma_end(&m_strm);
		m_demand_callback(this,NULL,0,DEMAND_CLOSE,m_callback_arg);
		m_opened=false;
	}
}

int CTarArcFile_Lzma::get_archive_type()
{
	return m_format_type;
}

std::string CTarArcFile_Lzma::get_orig_filename(){
	if(! m_orig_filename.empty()){return m_orig_filename;}
	std::string fname = get_filename(m_arcfile.c_str());
	char *ext = NULL;
	switch(m_format_type) {
	case ARCHIVETYPE_LZMA:
		ext = ".lzma";
		break;
	case ARCHIVETYPE_XZ:
		ext = ".xz";
		break;
	default:
		break;
	};
	if(ext!=NULL){
		size_t ext_len = strlen(ext);
		if(fname.length()>ext_len && stricmp(fname.substr(fname.length()-ext_len).c_str(),ext) == 0){
			return fname.substr(0, fname.length()-ext_len);
		}
	}
	return fname + "_extracted";
}

// open as not-file mode.
// require set_demand_callback()
bool CTarArcFile_Lzma::open_not_file(int compress_level)
{
	close();
	set_demand_callback(void_demand_callback,NULL);
	m_arcfile = "not file";
	m_mode = CM_NOTHING;
	m_compress_level = compress_level;
	m_error_status = ES_NO_ERROR;
	m_opened = true;
	return true;
}

void CTarArcFile_Lzma::set_format(int format_type){
	m_format_type = format_type;
}

void CTarArcFile_Lzma::set_demand_callback(demand_callback_t result_callback, void *arg)
{
	m_demand_callback = result_callback;
	m_callback_arg = arg;
}

// refer to src/xz/coder.c in XZ Utils.
int CTarArcFile_Lzma::check_head_format(unsigned char *buf, size_t buf_size, int format_type)
{
	if(buf_size>=6 && memcmp(buf, "\3757zXZ", 6) == 0){
		if(format_type>=0) {
			return format_type==ARCHIVETYPE_XZ;
		} else {
			return ARCHIVETYPE_XZ;
		}
	}
	while(buf_size>=13){
		lzma_filter filter;
		filter.id = LZMA_FILTER_LZMA1;
		if(lzma_properties_decode(&filter, NULL, buf, 5) != LZMA_OK){
			break;
		}
		lzma_options_lzma *opt = (lzma_options_lzma *)(filter.options);
		const uint32_t dict_size = opt->dict_size;
//		free(opt);

		if (dict_size != UINT32_MAX) {
			uint32_t d = dict_size - 1;
			d |= d >> 2;
			d |= d >> 3;
			d |= d >> 4;
			d |= d >> 8;
			d |= d >> 16;
			++d;
			if (d != dict_size || dict_size == 0)
				break;
		}

		uint64_t uncompressed_size = 0;
		for (size_t i = 0; i < 8; ++i)
			uncompressed_size |= (uint64_t)(buf[5 + i]) << (i * 8);

		if (uncompressed_size != UINT64_MAX && uncompressed_size > (UINT64_C(1) << 38)){
			break;
		}
		if(format_type>=0) {
			return format_type==ARCHIVETYPE_LZMA;
		} else {
			return ARCHIVETYPE_LZMA;
		}
	}
	if(format_type>=0) {
		return 0;
	} else {
		return ARCHIVETYPE_NORMAL;
	}
}

size64 CTarArcFile_Lzma::file_demand_callback(class CTarArcFile_Lzma *instance, void *buff, size64 size, demand_t demand, void *arg)
{
	if(instance->m_fp==NULL) return false;
	size64 ressize = 0;
	switch(demand){
	case DEMAND_READ:
		ressize = fread(buff,1,(size_t)size,instance->m_fp);
		if(ressize != size && !feof(instance->m_fp)){
			instance->m_error_status = ES_FATAL_ERROR;
			throw CTar32Exception("can't read file", ERROR_CANNOT_READ);
		}
		break;
	case DEMAND_WRITE:
		ressize = fwrite(buff,1,(size_t)size,instance->m_fp);
		if(ressize != size){
			instance->m_error_status = ES_FATAL_ERROR;
			throw CTar32Exception("can't write file", ERROR_CANNOT_WRITE);
		}
		break;
	case DEMAND_CLOSE:
		if(instance->m_fp!=NULL){
			fclose(instance->m_fp);
			instance->m_fp = NULL;
		}
		break;
	}
	return ressize;
}
size64 CTarArcFile_Lzma::void_demand_callback(class CTarArcFile_Lzma *instance, void *buff, size64 size, demand_t demand, void *arg)
{
	return size;
}
bool CTarArcFile_Lzma::coder_initialize(void)
{
	lzma_ret ret = LZMA_PROG_ERROR;

	if(lzma_lzma_preset(&m_opt_lzma, m_compress_level)){
		; // バグ.
	}

	// 現状、以下の処理を端折ってる.
	// エンコード時、物理メモリから辞書サイズ(opt_lzma.dict_size)を算出.
	//   辞書サイズはcompress_levelから自動で計算されるが、物理メモリから.
	//   調整しないといけない.
	// デコード時、物理メモリから memlimit を算出.
	switch(m_format_type){
	case ARCHIVETYPE_LZMA:
		m_filters[0].id = LZMA_FILTER_LZMA1;
		m_filters[0].options = &m_opt_lzma;
		m_filters[1].id = LZMA_VLI_UNKNOWN;
		break;
	case ARCHIVETYPE_XZ:
		m_filters[0].id = LZMA_FILTER_LZMA2;
		m_filters[0].options = &m_opt_lzma;
		m_filters[1].id = LZMA_VLI_UNKNOWN;
		break;
	default:
		m_filters[0].id = LZMA_VLI_UNKNOWN;
		break;
	}

	if(m_mode==CM_ENCODE) {
		switch(m_format_type){
		case ARCHIVETYPE_LZMA:
			ret = lzma_alone_encoder(&m_strm, (lzma_options_lzma *)(m_filters[0].options));
			break;
		case ARCHIVETYPE_XZ:
			ret = lzma_stream_encoder(&m_strm, m_filters, LZMA_CHECK_CRC32);
			break;
		default:
			break;
		}
	} else {
		switch(m_format_type){
		case ARCHIVETYPE_LZMA:
			ret = lzma_alone_decoder(&m_strm, m_memlimit);
			break;
		case ARCHIVETYPE_XZ:
			ret = lzma_stream_decoder(&m_strm, m_memlimit, LZMA_TELL_UNSUPPORTED_CHECK |  LZMA_CONCATENATED);
//			ret = lzma_stream_decoder(&m_strm, m_memlimit, 0);
			break;
		default:
			break;
		}
	}
	if(ret!=LZMA_OK) return false;

	m_strm.avail_in = 0;
	m_strm.avail_out = 0;
	m_action = LZMA_RUN;

	return true;
}

// 実処理.
// 処理が分かりにくいのでコメントを足す.
size64 CTarArcFile_Lzma::coder(void *buf, size64 size)
{
	unsigned char *buff = (unsigned char *)buf;
	size64 io_buff_size = sizeof(m_io_buff)/sizeof(char);
	size64 total_size = 0;
	size64 size_orig = size;
	size64 prev = 0;	// 処理されたサイズを知るための補助変数.
	bool do_break = false;
	bool do_flush = false;

	if(buff==NULL||size==0){
		if(m_mode==CM_DECODE) return 0;
		buff = NULL;
		size = 0;
	}
	while (m_error_status != ES_FATAL_ERROR){
		lzma_ret ret;
		// エンコード時 入力: 引数バッファ、出力:一時バッファ、コールバック.
		// デコード時 入力: 一時バッファ、コールバック、出力:引数バッファ.
		// 入力バッファが空になったら.
		if(m_strm.avail_in==0){
			if(m_mode==CM_ENCODE){
				if(m_action==LZMA_RUN){
					m_strm.next_in = buff;
					if(size<=io_buff_size){
						m_strm.avail_in = (size_t)size;
					}else{
						m_strm.avail_in = (size_t)io_buff_size;
					}
					buff += m_strm.avail_in;
					size -= m_strm.avail_in;
				}
				prev = m_strm.avail_in;
//				必要ないっぽいので除外.
//				if(m_strm.avail_in==0){
//					m_action = LZMA_FINISH;
//				}
			}else if(m_mode==CM_DECODE){
				m_strm.avail_in = (size_t)m_demand_callback(this,m_io_buff,io_buff_size,DEMAND_READ,m_callback_arg);
				m_strm.next_in = m_io_buff;
//				0 にしてからじゃないと駄目っぽい？.
				if(m_strm.avail_in==0){
//				if(m_strm.avail_in<io_buff_size){
					m_action = LZMA_FINISH;
				}
			}
		}
		// 出力バッファが空になったら.
		if(m_strm.avail_out==0){
			if(m_mode==CM_ENCODE){
				m_strm.next_out = m_io_buff;
				m_strm.avail_out = (size_t)io_buff_size;
			}else if(m_mode==CM_DECODE){
				m_strm.next_out = (uint8_t *)buff;
				if(size<=io_buff_size){
					m_strm.avail_out = (size_t)size;
				}else{
					m_strm.avail_out = (size_t)io_buff_size;
				}
				buff += m_strm.avail_out;
				size -= m_strm.avail_out;
				prev = m_strm.avail_out;
			}
		}

//printf("<ac%d: in:%d out:%d ",m_action,m_strm.avail_in,m_strm.avail_out);
		ret = lzma_code(&m_strm, m_action);
//printf("ret:%d in:%d out:%d>\n",ret,m_strm.avail_in,m_strm.avail_out);
		// 処理サイズの計算.
		if(m_mode==CM_ENCODE){
			total_size += prev - m_strm.avail_in;
			prev = m_strm.avail_in;
		}else if(m_mode==CM_DECODE){
			total_size += prev - m_strm.avail_out;
			prev = m_strm.avail_out;
		}

		// 出力バッファが一杯になったら(エンコード時用).
		if(m_mode==CM_ENCODE && m_strm.avail_out==0) do_flush = true;

		if (ret != LZMA_OK) {
			if (ret == LZMA_STREAM_END) {
				if(m_mode==CM_ENCODE) do_flush = true;
				m_action = LZMA_FINISH; // 別にいらなそうだが.
			}
			do_break = true;
		}
		// 出力バッファを書き出す(エンコード時用).
		if(do_flush){
			m_demand_callback(this,m_io_buff,io_buff_size-m_strm.avail_out,DEMAND_WRITE,m_callback_arg);
			do_flush = false;
		}
		
		if(do_break) break;

		// 終了処理時は継続.
		if (ret == LZMA_OK && m_action == LZMA_FINISH) continue;

		// デコード時、予定分のサイズ処理したら強制で抜ける.
		if(m_mode==CM_DECODE && total_size>=size_orig) break;
	}

	return total_size;
}

