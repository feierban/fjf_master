#include "nalu.h"
#include <iostream>
#include <QDebug>
#include <qthread.h>
#include <qfile.h>

static bool isstartcode(const char* buffer)
{
	if (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 0 && buffer[3] == 1) {
		return true;
	}
	return false;
}

nalu::nalu()
{
	startcodeprefix_len = 4;
	forbidden_bit = 0;
	nal_reference_idc = NALU_PRIORITY_DISPOSABLE;

	codec_ = NULL;
	pFrame_ = NULL;
	avctx_ = NULL;
	codecInited = false;
	begin = 0;

	decodebegin = 0;
	m_feikong.clear();
}


nalu::~nalu()
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

bool nalu::ReInitDecoder()
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

bool nalu::InitDecoder()
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

void nalu::appendData(const char* data, size_t len)
{
	mutex.lock();
	m_data.append(data, len);
	mutex.unlock();
}

int nalu::findstartcode(QByteArray in)
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

int nalu::findfeikongstartcode(QByteArray in)
{
	int start = findstartcode(in);
	if (start != -1) {
		if (NALU_TYPE_FAIKONG == (in[start + 4] & 0x1f)) {
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


bool nalu::DecodeNalu(QByteArray na)
{
	const uint8_t* data = NULL;
	int buf_size = na.size();
	data = (uint8_t*)calloc(buf_size, 1);
	const uint8_t *p_data = data;
	memcpy((void*)data, na.data(), buf_size);

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

bool nalu::DecodeThreadMain()
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
				m_datatodecode.append(receivedata.mid(0, 4 + findpos));
				receivedata = receivedata.mid(4 + findpos);
				directDecode();
			}
		}	
		QThread::msleep(10);
	}
}

bool nalu::directDecode()
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
			}
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

bool nalu::flush()
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

bool nalu::saveTorgb24(AVCodecContext *avcc, AVFrame *avf)
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

