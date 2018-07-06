#pragma once
#include <qbytearray.h>
#include <qimage.h>
#include <qmutex.h>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

class Nalu
{
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
        NALU_TYPE_FEIKONG = 24,		//FEIMA DATA
        NALU_TYPE_MAX,
    };
    enum NaluPriority {
        NALU_PRIORITY_DISPOSABLE = 0,
        NALU_PRIRITY_LOW = 1,
        NALU_PRIORITY_HIGH = 2,
        NALU_PRIORITY_HIGHEST = 3,
        NALU_PRIORITY_MAX,
    };
public:
    Nalu();
    //function
    //must include nalu head
    QByteArray &append(QByteArray);

    QByteArray &data();
    QByteArray &head();
    QByteArray &content();
    void clear();
    bool isNalu();
    NaluType unitType();
    NaluPriority unitPriority();
    int size();
    //variable


private:
    //function
    void init();
    bool decodeunit();

    //variable
    QByteArray m_naludata;
    QByteArray m_naluhead;
    QByteArray m_nalucontent;
    bool m_isNalu;
    NaluType m_unitType;
    NaluPriority m_unitPriority;
    bool m_isDecoded;

};

class NaluProcess : public QObject
{
    Q_OBJECT

public:
    NaluProcess();
    ~NaluProcess();
	bool InitDecoder();
	void appendData(const char* data, size_t len);
	bool directDecode();
	bool DecodeThreadMain();
	int findstartcode(QByteArray in);
	int findfeikongstartcode(QByteArray in);
	bool flush();

signals:
    void sig_GetOneFrame(QImage img);
    void Sig_GetOneNalu(Nalu);

private:
    bool saveTorgb24(AVCodecContext *,AVFrame *);
	bool DecodeNalu(QByteArray);
	bool ReInitDecoder();
    void Rtmp_Init();
    void Rtmp_Write();


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
    AVFormatContext *p_ofmt_ctx;
    int64_t n_start_time;
    AVRational stream_rate;    //h264 rate

    bool inited = false;


	QMutex mutex;
	int begin;
	QByteArray m_feikong;
};

