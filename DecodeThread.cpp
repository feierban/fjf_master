#include "DecodeThread.h"
#include <qfile.h>


DecodeThread::DecodeThread()
{
	m_ip.setAddress("127.0.0.1");
	m_port = 10001;
	m_nalu = NULL;
	m_socket = NULL;
	m_updateconfig = false;
	m_thread = NULL;
}

void DecodeThread::run()
{
	if (!m_socket)
		m_socket = new QUdpSocket();
	if (!m_nalu)
		m_nalu = new nalu();

	if (!m_nalu || !m_socket)
		return;

	if (!m_nalu->InitDecoder()) {
		return;
	}

	m_thread = new QThread();
	m_nalu->moveToThread(m_thread);
	connect(m_thread, &QThread::started, m_nalu, &nalu::DecodeThreadMain);
	m_thread->start();

	connect(m_nalu, &nalu::sig_GetOneFrame, this, &DecodeThread::Slt_onImageGet);
	connect(m_nalu, &nalu::Sig_GetFenkong, this, &DecodeThread::Slt_onFeiKongDataGet);
	connect(m_socket, &QUdpSocket::readyRead, this, &DecodeThread::Slt_getVideoStream);
	QString str = "this is test\n";
	m_socket->writeDatagram(str.toStdString().c_str(), str.length(), m_ip, m_port);

	return;
}

void DecodeThread::stop()
{
	
	if (m_socket) {
		delete m_socket;
		m_socket = NULL;
	}

	if (m_nalu) {
		m_nalu->flush();
		delete m_nalu;
		m_nalu = NULL;
	}
}

void DecodeThread::Slt_getVideoStream()
{
	while (m_socket->hasPendingDatagrams()) {
		if (!m_nalu)
			return;

		QByteArray datagram;
		datagram.resize(m_socket->pendingDatagramSize());
		m_socket->readDatagram(datagram.data(), datagram.size());
		m_nalu->appendData(datagram.data(), datagram.size());

#if fjf_dug
		QFile file("1.data");
		file.open(QIODevice::Append);
		QDataStream out(&file);
		out << datagram;
		file.close();
#endif

	}
	return;
}

void DecodeThread::SetUdpConfig(QString ip, int port)
{
	m_ip = QHostAddress(ip);
	m_port = port;
}

void DecodeThread::Slt_onFeiKongDataGet(QByteArray fk_data)
{
	Sig_SendFeiKongData(fk_data);
}

void DecodeThread::Slt_onImageGet(QImage img)
{
	static int a = 0;
	qDebug() << a++ << "pics";
	if (a%3 == 0)
		Sig_Sendimg(img);
}

DecodeThread::~DecodeThread()
{
	m_socket = NULL;
}
