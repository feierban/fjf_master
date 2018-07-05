#pragma once
#include <qbytearray.h>
#include <qimage.h>
#include <qmutex.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class nalu : public QObject
{
    Q_OBJECT
public:
	enum NaluType {
		NALU_TYPE_ILLEGAL = 0,
		NALU_TYPE_SLICE = 1,
		NALU_TYPE_DPA = 2,
		NALU_TYPE_DPB = 3,
		NALU_TYPE_DPC = 4,
		NALU_TYPE_IDR = 5,
		NALU_TYPE_SEI = 6,
		NALU_TYPE_SPS = 7,
		NALU_TYPE_PPS = 8,
		NALU_TYPE_AUD = 9,
		NALU_TYPE_EOSEQ = 10,
		NALU_TYPE_EOSTREAM = 11,
		NALU_TYPE_FILL = 12,
		NALU_TYPE_FAIKONG = 24,		//FEIMA DATA
	};
	enum NaluPriority {
		NALU_PRIORITY_DISPOSABLE = 0,
		NALU_PRIRITY_LOW = 1,
		NALU_PRIORITY_HIGH = 2,
		NALU_PRIORITY_HIGHEST = 3
	};

public:
	nalu();
	~nalu();
	bool InitDecoder();
	void appendData(const char* data, size_t len);
	bool directDecode();
	bool DecodeThreadMain();
	int findstartcode(QByteArray in);
	int findfeikongstartcode(QByteArray in);
	bool flush();

signals:
    void sig_GetOneFrame(QImage img);
	void Sig_GetFenkong(QByteArray);

private:
    bool saveTorgb24(AVCodecContext *,AVFrame *);
	bool DecodeNalu(QByteArray);
	bool ReInitDecoder();

public:
	int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
	int forbidden_bit;            //! should be always FALSE
	int nal_reference_idc;        //! NALU_PRIORITY_xxxx

	QByteArray m_data;			  //data of receive
	QByteArray m_datatodecode;	  

private:
	AVCodecParserContext *avParserContext;
	AVCodec *codec_;
	AVFrame *pFrame_;
	AVCodecContext *avctx_;
	AVPacket *avpkt_;
	bool codecInited;
	bool decodebegin;
    uint8_t *out_buffer;

	QMutex mutex;
	int begin;
	QByteArray m_feikong;
};

