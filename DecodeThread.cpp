#include "DecodeThread.h"
#include <qfile.h>
#include <qdebug.h>

DecodeThread::DecodeThread()
{
	m_ip.setAddress("127.0.0.1");
	m_port = 10001;
    m_naluproc = NULL;
	m_socket = NULL;
	m_updateconfig = false;
	m_thread = NULL;
}

void DecodeThread::run()
{
	if (!m_socket)
		m_socket = new QUdpSocket();
    if (!m_naluproc)
        m_naluproc = new NaluProcess();

    if (!m_naluproc || !m_socket)
		return;

    if (!m_naluproc->InitDecoder()) {
		return;
	}

	m_thread = new QThread();
    m_naluproc->moveToThread(m_thread);
    connect(m_thread, &QThread::started, m_naluproc, &NaluProcess::DecodeThreadMain);
	m_thread->start();

    qRegisterMetaType<Nalu> ("Nalu");
    connect(m_naluproc, &NaluProcess::sig_GetOneFrame, this, &DecodeThread::Slt_onImageGet);
    connect(m_naluproc, &NaluProcess::Sig_GetOneNalu, this, &DecodeThread::Slt_onOneNaluGet);
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

    if (m_naluproc) {
        m_naluproc->flush();
        delete m_naluproc;
        m_naluproc = NULL;
	}
}

void DecodeThread::Slt_getVideoStream()
{
	while (m_socket->hasPendingDatagrams()) {
        if (!m_naluproc)
			return;

        QByteArray datagram;
		datagram.resize(m_socket->pendingDatagramSize());
		m_socket->readDatagram(datagram.data(), datagram.size());

#if 1
        QFile file("1.data");
        file.open(QIODevice::Append);
        QDataStream out(&file);
        out << datagram;
        file.close();
#endif

        m_naluproc->appendData(datagram.data(), datagram.size());
	}
	return;
}

void DecodeThread::SetUdpConfig(QString ip, int port)
{
	m_ip = QHostAddress(ip);
	m_port = port;
}

void DecodeThread::Slt_onOneNaluGet(Nalu _nalu)
{
    static int i = 0;
    static int j = 0;
    static int k = 0;
    if (_nalu.unitType() == Nalu::NALU_TYPE_SPS) {
        i++;
    } else if (_nalu.unitType() == Nalu::NALU_TYPE_PPS) {
        j++;
    } else if (_nalu.unitType() == Nalu::NALU_TYPE_IDR) {
        k++;
    }
    qDebug() << "i = " << i << " j = " << j << " k = " << k;
    //Sig_SendH264Data();
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
