#include "naluprocess.h"
#include <iostream>
#include <QDebug>
#include <qthread.h>
#include <qfile.h>
#include <QMetaType>

static bool isstartcode(const char* buffer)
{
    if (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 0 && buffer[3] == 1) {
        return true;
    }
    return false;
}

Nalu::Nalu()
{
    init();
}

QByteArray &Nalu::append(QByteArray data)
{
    m_isDecoded = false;
    return m_naludata.append(data);
}

QByteArray &Nalu::data()
{
    return m_naludata;
}

QByteArray &Nalu::head()
{
    return m_naluhead;
}

QByteArray &Nalu::content()
{
    return m_nalucontent;
}

void Nalu::init()
{
    m_isNalu = false;
    m_isDecoded = false;
    m_unitType = NALU_TYPE_ILLEGAL;
    m_unitPriority = NALU_PRIORITY_DISPOSABLE;
    m_naludata.clear();
    m_naluhead.clear();
    m_nalucontent.clear();
}

void Nalu::clear()
{
     init();
}

bool Nalu::isNalu()
{
    if (!m_isDecoded) {
        decodeunit();
    }
    return m_isNalu;
}

Nalu::NaluType Nalu::unitType()
{
    if (!m_isDecoded) {
        decodeunit();
    }
    return m_unitType;
}

Nalu::NaluPriority Nalu::unitPriority()
{
    if (!m_isDecoded) {
        decodeunit();
    }
    return m_unitPriority;
}

int Nalu::size()
{
    return m_naludata.size();
}

bool Nalu::decodeunit()
{
    if (m_naludata.size() < 4)
        return false;
    char *buf = m_naludata.data();
    if (!isstartcode(buf))
        return false;

    m_isDecoded = true;

    char f_c = m_naludata[4];
    int forbidden_zero_bit = (f_c & 0x80) >> 7;
    int nal_ref_idc = (f_c & 0x60) >> 5;
    int nal_unit_type = (f_c & 0x1f);
    if (forbidden_zero_bit != 0)
        m_isNalu = false;
    if (nal_unit_type >= NALU_TYPE_MAX)
        m_isNalu = false;
    m_unitType = (NaluType)nal_unit_type;

    if (nal_ref_idc >= NALU_PRIORITY_MAX)
        m_isNalu = false;
    m_unitPriority = (NaluPriority)nal_ref_idc;

    m_naluhead = m_naludata.mid(0,4);
    m_nalucontent = m_naludata.mid(4);
    m_isNalu = true;
    return true;
}

NaluProcess::NaluProcess()
{
	codec_ = NULL;
	pFrame_ = NULL;
	avctx_ = NULL;
	codecInited = false;
	begin = 0;
    stream_rate.den = 1;
    stream_rate.num = 25;

	decodebegin = 0;
    m_feikong.clear();
}


NaluProcess::~NaluProcess()
{
    av_frame_free(&pFrame_);
	pFrame_ = NULL;

	av_packet_free(&avpkt_);
	avpkt_ = NULL;

    av_parser_close(avParserContext);
	avParserContext = NULL;

    avcodec_close(avctx_);
	avctx_ = NULL;

	codec_ = NULL;
	codecInited = false;
}

bool NaluProcess::ReInitDecoder()
{
	av_frame_free(&pFrame_);
	av_packet_free(&avpkt_);
	av_parser_close(avParserContext);
	avcodec_close(avctx_);
	codec_ = NULL;
	pFrame_ = NULL;
	avctx_ = NULL;
	codecInited = false;
	InitDecoder();
	return true;
}

bool NaluProcess::InitDecoder()
{
    if (codecInited)
        return true;

    avcodec_register_all();

    codec_ = avcodec_find_decoder(AVCodecID::AV_CODEC_ID_H264);
    if (!codec_)
        return false;

    avctx_ = avcodec_alloc_context3(codec_);
    if (!avctx_)
        return false;

    avParserContext = av_parser_init(AVCodecID::AV_CODEC_ID_H264);	//fjf_ques_1

    if (avcodec_open2(avctx_, codec_, NULL) < 0) {
        return false;
    }

    avpkt_ = av_packet_alloc();
    pFrame_ = av_frame_alloc();
    codecInited = true;
    return true;
}

void NaluProcess::appendData(const char* data, size_t len)
{
	mutex.lock();
	m_data.append(data, len);
	mutex.unlock();
}

int NaluProcess::findstartcode(QByteArray in)
{
	int buf_size = in.size();
	if (buf_size < 4)
		return false;

	char * buf = in.data();
	int start = 0;
	while (start < buf_size - 4) {
		if (isstartcode(buf + start)) {
			return start;
		}
		start++;
	}
	return -1;
}

int NaluProcess::findfeikongstartcode(QByteArray in)
{
	int start = findstartcode(in);
	if (start != -1) {
        if (Nalu::NALU_TYPE_FEIKONG == (in[start + 4] & 0x1f)) {
			return start;
		}
		else {
			int feima = findfeikongstartcode(in.mid(start + 4));
			if (feima != -1)
				return (4 + feima);
			else
				return -1;
		}
	}
	return -1;
}


bool NaluProcess::DecodeNalu(QByteArray na)
{
	const uint8_t* data = NULL;
	int buf_size = na.size();
	data = (uint8_t*)calloc(buf_size, 1);
	const uint8_t *p_data = data;
	memcpy((void*)data, na.data(), buf_size);
    avpkt_ = av_packet_alloc();
	av_parser_parse2(avParserContext, avctx_, &(avpkt_->data), &(avpkt_->size), data, buf_size, 0, 0, 0);

#if 0
	if (avpkt_->size < 0) {
		return false;
	}

	int gotPic = 0;
	int len = avcodec_decode_video2(avctx_, pFrame_, &gotPic, avpkt_);
	if (len < 0) {
		return false;
	}
	if (gotPic) {
		// succ to get some pictures
	}
#else
	if (avpkt_->size > 0) {
		if (avcodec_send_packet(avctx_, avpkt_)) {
			// !=0: mean send false;
			// todo
		}
		else {
			// 0: mead send succ;
			// todo
		}
	} else {
		qDebug() << "error" << " pkt";
	}
	while (0 == (avcodec_receive_frame(avctx_, pFrame_))) {
		saveTorgb24(avctx_, pFrame_);
	}
#endif		
	if (p_data) {
		free((void*)p_data);
	}
	return true;
}

bool NaluProcess::DecodeThreadMain()
{
	QByteArray receivedata;
	bool streamstart = false;

	int findpos = -1;

	while (true) {
		mutex.lock();
		receivedata.append(m_data);
		m_data.clear();
		mutex.unlock();

		if (!streamstart) {
			int index = findstartcode(receivedata);
			if (index == -1) {
				continue;
			} else {
				receivedata = receivedata.mid(index);
				streamstart = true;
			}
		}

		while (receivedata.size()) {
			findpos = findstartcode(receivedata.mid(4));

			if (findpos == -1) {
				break;
			} else {
                //for nalu unit send
                {
                    Nalu naulint;
                    naulint.append(receivedata.mid(0, 4 + findpos));
                    emit Sig_GetOneNalu(naulint);
                }

				m_datatodecode.append(receivedata.mid(0, 4 + findpos));
				receivedata = receivedata.mid(4 + findpos);
				directDecode();
			}
		}	
		QThread::msleep(10);
	}
}

bool NaluProcess::directDecode()
{
	if (!m_datatodecode.size())
		return false;

	const uint8_t* data = NULL;
	data = (uint8_t*)calloc(m_datatodecode.size() + 1, 1);
	const uint8_t *p_data = data;
	memcpy((void*)data, m_datatodecode.data(), m_datatodecode.size());
	int len = m_datatodecode.size();
	while (len > 0) {
        int length = av_parser_parse2(avParserContext, avctx_, &(avpkt_->data), &(avpkt_->size), data, len, 0, 0, 0);
		data += length;
		len -= length;

#if 0
		if (avpkt_->size < 0) {
			return false;
		}

		int gotPic = 0;
		int len = avcodec_decode_video2(avctx_, pFrame_, &gotPic, avpkt_);
		if (len < 0) {
			return false;
		}
		if (gotPic) {
			// succ to get some pictures
		}
#else
		if (avpkt_->size > 0) {
            if (avcodec_send_packet(avctx_, avpkt_)) {
				// !=0: mean send false;
				// todo
			} else {
				// 0: mead send succ;
				// todo
                Rtmp_Init();
                Rtmp_Write();            }
		} else {
			qDebug() << "error" << " pkt";
		}
		while (0 == (avcodec_receive_frame(avctx_, pFrame_))) {
            saveTorgb24(avctx_, pFrame_);
		}
#endif		
	}
	if (p_data) {
		free((void*)p_data);
	}
	m_datatodecode.clear();
    return true;
}

bool NaluProcess::flush()
{
	while (true) {
		AVPacket *tmp_avpkt_ = NULL;
		if (avcodec_send_packet(avctx_, tmp_avpkt_)) {
			return true;
		}

		while (0 == (avcodec_receive_frame(avctx_, pFrame_))) {
            saveTorgb24(avctx_, pFrame_);
		}
	}

	return true;
}

bool NaluProcess::saveTorgb24(AVCodecContext *avcc, AVFrame *avf)
{
    qDebug() << "width:" << avcc->width << " height:" << avcc->height;

    uint8_t *rgbBuffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, avcc->width, avcc->height, 1));

    SwsContext *sws;
    sws = sws_getContext(avcc->width, avcc->height, AV_PIX_FMT_YUV420P, \
                         avcc->width, avcc->height, AV_PIX_FMT_RGB24, \
                         SWS_BICUBIC, NULL, NULL, NULL);
    int stride = avcc->width * 3;
    sws_scale(sws, avf->data, avf->linesize, 0, avcc->height,
              &rgbBuffer, &stride);

    QImage tmpImg((uchar *)rgbBuffer, avcc->width, avcc->height,QImage::Format_RGB888);
    QImage image = tmpImg.copy();
    emit sig_GetOneFrame(image);
    av_free(rgbBuffer);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
void NaluProcess::Rtmp_Init()
{
    if (inited)
        return;
    //QString outurl = "rtmp://111.231.56.227:1935/live/123";
    QString outurl = "2.mp4";
    //avformat_alloc_output_context2( &p_ofmt_ctx, NULL, "flv", outurl.toLatin1().data() );
    avformat_alloc_output_context2( &p_ofmt_ctx, NULL, "mp4", outurl.toLatin1().data() );
    AVStream *out_stream = avformat_new_stream( p_ofmt_ctx, NULL );
    {
        AVCodecContext *c;
        c = out_stream->codec;
        c->bit_rate = 400000;
        c->codec_id = AV_CODEC_ID_H264;
        c->codec_type = AVMEDIA_TYPE_VIDEO;
        c->time_base.num = 1000000;
        c->time_base.den = 1;
        c->width = 1920;
        c->height = 1080;
        //c->pix_fmt = AV_PIX_FMT_YUVJ420P;
        c->max_qdiff = 3;
        c->qmax = 31;
        c->qmin = 2;
        c->qcompress = 0.5;

    }
    {
        //avcodec_copy_context(out_stream->codec, avctx_);
    }

    int n_ret = avio_open( &p_ofmt_ctx->pb, outurl.toLatin1().data(), AVIO_FLAG_WRITE );
    n_ret = avformat_write_header( p_ofmt_ctx, NULL );
    if (n_ret)
        inited = true;
}

void NaluProcess::Rtmp_Write()
{
    static int a = 0;
    AVPacket *pket = av_packet_alloc();
    av_copy_packet(pket, avpkt_);
    pket->flags |= AV_PKT_FLAG_KEY;

    if( pket->pts == AV_NOPTS_VALUE ) {
        AVRational time_base1 = p_ofmt_ctx->streams[0]->time_base;
        int64_t n_calc_duration = ( double )AV_TIME_BASE/av_q2d(stream_rate);
        pket->pts = ( double )( p_ofmt_ctx->streams[0]->nb_frames  * n_calc_duration )/( double )( av_q2d(time_base1)*AV_TIME_BASE );
        pket->dts = pket->pts;
        pket->duration = ( double )n_calc_duration/( double )( av_q2d( time_base1 )*AV_TIME_BASE );
    }

    int n_ret = 0;
    n_ret = av_write_frame( p_ofmt_ctx, pket );
    qDebug() << "fjf_dug ------------ log" << n_ret;
    if (a++ > 300) {
        a = 0;
        av_write_trailer(p_ofmt_ctx);
        avio_close(p_ofmt_ctx->pb);
        p_ofmt_ctx->pb = NULL;
        av_free(p_ofmt_ctx);
        inited = false;
    }
}


