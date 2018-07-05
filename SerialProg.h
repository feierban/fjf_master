#pragma once
#include <FmLog/QsLog.h>
#include <FmLinkInstance/fmlinkmanager.h>
#include <FmLinkInstance/fmuavconnectobj.h>
#include <fmLinkCore/fmuasprovide.h>

class SerialProg :public QObject
{
	Q_OBJECT

public:
	static QStringList GetSerialComs();

	SerialProg();
	void SerialMain();
	~SerialProg();
signals:
	void startConnectUAV();

	//to mainthread
signals:
	void Sig_GetUasProvide(CFmUasProvide *);
	void Sig_UasDisconnect();
	void Sig_GetMultirotorPlaneInfo(QStringList);

private:
	CFmLinkManager *linkManager;
	CFmUAVConnectObj *pUAVConnectObj;
	QThread m_UavConnectThread;

private slots:
	void slotUasConnected(CFmUasProvide*);
	void slotRecivePlaneInfo(QStringList);
	void slotReciveMultirotorPlaneInfo(QStringList);
	void slotKeyStatusString(QString);
	void slotUasDisConnected(CFmUasProvide*);
	void slotNeedNotifyParachute(int);
	void slotReadD200FmVersion(QString);
	void slotMarked(bool);

};

