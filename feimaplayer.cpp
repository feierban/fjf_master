#include "feimaplayer.h"
#include <iostream>
#include "qdebug.h"

feimaplayer::feimaplayer()
{
	max_buffer = 10000;
	data_buffer = (char*)calloc(1, max_buffer);
}

void feimaplayer::slt_onimageget(QImage img)
{
	qDebug() << "error";
    sig_sendoneimg(img);
}

feimaplayer::~feimaplayer()
{
	free(data_buffer);
	data_buffer = NULL;
}

bool feimaplayer::isstartcode(const char* buffer)
{
    if (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 0 && buffer[3] == 1) {
			return true;
	}
	return false;
}

bool feimaplayer::resetcalloc()
{
	if (data_buffer == NULL)
		return false;

	free(data_buffer);
	data_buffer = (char *)calloc(1, max_buffer);
	if (data_buffer == NULL)
		return false;
}

size_t feimaplayer::readonenalu(FILE* file, char *buff, size_t maxsize, size_t *startpos)
{
	if (maxsize < 4)
		return 0;

	bool getstart = false;
	size_t start_pos = 0;
	size_t pos = 0;
	while (!feof(file) && pos < maxsize) {
		pos += fread(buff + pos, 1, 1, file);
		if (pos >= 4) {
			size_t tmp_pos = pos - 4;
			if (isstartcode(buff + tmp_pos)) {
				getstart = true;
				start_pos = tmp_pos;
				break;
			}
		}
	}
	
	while (!feof(file) && pos < maxsize) {
		pos += fread(buff + pos, 1, 1, file);
		if (pos - start_pos >= 8) {
			size_t tmp_pos = pos - 4;
			if (isstartcode(buff + tmp_pos)) {
				break;
			}
		}
	}

	if (getstart) {
		if (!feof(file)) {
			fseek(file, -4, SEEK_CUR);
		}
		*startpos = start_pos;
		return (pos - 4 - start_pos);
	}
	return 0;
}

void feimaplayer::ThreadMain()
{
	if (data_buffer == NULL) {
		std::cout << "ERROR: no buffer memory\n";
		return;
	}

	FILE *file = fopen(filename.toStdString().c_str(), "rb+");
	if (file == NULL) {
		std::cout << "ERROR: file can not open\n";
		return;
	}

	pthread = new QThread();
	m_nalu = new nalu();
	m_nalu->moveToThread(pthread);
	connect(pthread, &QThread::started, m_nalu, &nalu::DecodeThreadMain);
	connect(m_nalu, &nalu::sig_GetOneFrame, this, &feimaplayer::slt_onimageget, Qt::QueuedConnection);

	m_nalu->InitDecoder();
	pthread->start();
	while (!feof(file)) {
		memset(data_buffer, 0, max_buffer);
		size_t start_pos = 0;
		size_t size = fread(data_buffer, 1, max_buffer, file);
		m_nalu->appendData(data_buffer, size);
		QThread::msleep(50);
	}
	m_nalu->flush();
	fclose(file);
}

bool feimaplayer::load(QString name)
{
	filename = name;
	return true;
}
