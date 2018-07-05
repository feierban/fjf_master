#include "SerialProg.h"
#include <qmetatype.h>
#include <FmLinkInstance\fmmultirotorpilot.h>

SerialProg::SerialProg()
{
}

void SerialProg::SerialMain()
{
	linkManager = new CFmLinkManager(this);
	//linkManager->AddSerialConnectionLink();
	//linkManager->AddTcpClientLink();
	//linkManager->AddRTKTcpClientLink();
	qRegisterMetaType<mavlink_message_t>("mavlink_message_t");

	pUAVConnectObj = new CFmUAVConnectObj(linkManager, NULL);
	pUAVConnectObj->moveToThread(&m_UavConnectThread);

	connect(pUAVConnectObj, &CFmUAVConnectObj::UAV_connected, this, &SerialProg::slotUasConnected);
	connect(pUAVConnectObj, &CFmUAVConnectObj::planeInfo, this, &SerialProg::slotRecivePlaneInfo);
	connect(pUAVConnectObj, &CFmUAVConnectObj::multirotor_planeInfo, this, &SerialProg::slotReciveMultirotorPlaneInfo);
	connect(pUAVConnectObj, &CFmUAVConnectObj::KeyStatusString, this, &SerialProg::slotKeyStatusString);
	connect(pUAVConnectObj, &CFmUAVConnectObj::UAV_disconnected, this, &SerialProg::slotUasDisConnected);
	connect(pUAVConnectObj, &CFmUAVConnectObj::NeedNotifyParachute, this, &SerialProg::slotNeedNotifyParachute);
	connect(pUAVConnectObj, &CFmUAVConnectObj::multirotor_BootState, this, &SerialProg::slotReadD200FmVersion);
	connect(pUAVConnectObj, &CFmUAVConnectObj::parachuteMark, this, &SerialProg::slotMarked);
	connect(pUAVConnectObj, &CFmUAVConnectObj::multirotor_BootState, this, &SerialProg::slotKeyStatusString);
	connect(this, SIGNAL(signalSendParachuteMsg()), pUAVConnectObj, SLOT(slotSendParachuteMsg()));

	//Free Serial Port
	connect(this, SIGNAL(freeSerialPort()), pUAVConnectObj, SLOT(slotFreeSerialPort()));
	connect(this, SIGNAL(connectSerialPort()), pUAVConnectObj, SLOT(slotConnectSerialPort()));

	connect(&m_UavConnectThread, SIGNAL(started()), pUAVConnectObj, SLOT(slotConnectUAVStart()));
	connect(&m_UavConnectThread, SIGNAL(finished()), pUAVConnectObj, SLOT(deleteLater()));
	m_UavConnectThread.start();

}

SerialProg::~SerialProg()
{
}

void SerialProg::slotUasConnected(CFmUasProvide* pUas)
{
	qDebug() <<"fjf_dug" << __FUNCTION__ << __LINE__ << "get CFmUasProvide";
	Sig_GetUasProvide(pUas);
}

void SerialProg::slotRecivePlaneInfo(QStringList)
{
	qDebug() << "fjf_dug" << __FUNCTION__ << __LINE__ << "get slot";
}

void SerialProg::slotReciveMultirotorPlaneInfo(QStringList strlist)
{
	Sig_GetMultirotorPlaneInfo(strlist);
}

void SerialProg::slotKeyStatusString(QString str)
{
	qDebug() << "fjf_dug" << __FUNCTION__ << __LINE__ << "get slot" << str;
}

void SerialProg::slotUasDisConnected(CFmUasProvide *)
{
	Sig_UasDisconnect();
}

void SerialProg::slotNeedNotifyParachute(int)
{
	qDebug() << "fjf_dug" << __FUNCTION__ << __LINE__ << __FUNCTION__ << "get slot";
}

void SerialProg::slotReadD200FmVersion(QString)
{
	qDebug() << "fjf_dug" << __FUNCTION__ << __LINE__ << __FUNCTION__ << "get slot";
}

void SerialProg::slotMarked(bool)
{
	qDebug() << "fjf_dug" << __FUNCTION__ << __LINE__ << __FUNCTION__ << "get slot";
}

