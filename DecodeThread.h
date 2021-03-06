#pragma once
#include <qthread.h>
#include <QtNetwork\qudpsocket.h>
#include <naluprocess.h>
#include <qimage.h>

class DecodeThread :
	public QObject
{
	Q_OBJECT
public:
	DecodeThread();
	void SetRun(bool r) {
		m_decode = r;
	}

	void run();
	void stop();
	void SetUdpConfig(QString, int);
	~DecodeThread();

signals:
	void Sig_Sendimg(QImage);
	void Sig_SendFeiKongData(QByteArray);
    void Sig_SendH264Data(QByteArray);

private slots:
	void Slt_onImageGet(QImage);
	void Slt_getVideoStream();
    void Slt_onOneNaluGet(Nalu);

private:
	bool m_decode;
	bool m_updateconfig;
	QHostAddress m_ip;
	quint64 m_port;
	QUdpSocket *m_socket;

    NaluProcess *m_naluproc;
	QThread *m_thread;
};

