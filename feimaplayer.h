#pragma once
#include <QObject>
#include "nalu.h"
#include <QImage>
#include <qthread.h>

class feimaplayer:
	public QThread
	{
    Q_OBJECT
public:
	feimaplayer();
	void ThreadMain();
	bool load(QString );
	bool set_max_buffer(int max) { max_buffer = max; }
	~feimaplayer();

signals:
    void sig_sendoneimg(QImage);

private slots:
    void slt_onimageget(QImage);

private:
	size_t readonenalu(FILE*, char *, size_t, size_t *);
	bool isstartcode(const char*);
	bool resetcalloc();
	int max_buffer;
	char *data_buffer;
	nalu *m_nalu;
	QString filename;
	QThread *pthread;
};

