#include "UasProvideProcess.h"
#include "qdebug.h"



UasProvideProcess::UasProvideProcess(CFmUasProvide* fmUasP)
{
	m_UasP = NULL;
	m_heartbeat = 0;

	if (fmUasP != NULL && m_UasP != fmUasP) {
		m_UasP = fmUasP;
	}
}

void UasProvideProcess::UasMain()
{
	if (m_UasP) {
		connect(m_UasP, SIGNAL(parameterChanged(int, int, QString, QVariant)), this, SLOT(Slt_ParamterInfo(int, int, QString, QVariant)));
		connect(m_UasP, &CFmUasProvide::heartbeat, this, &UasProvideProcess::Slt_GetHeartBeat);
		connect(&m_timer, &QTimer::timeout, this, &UasProvideProcess::TimerFired);
		m_timer.start(1000);
	}
}

void UasProvideProcess::Slt_GetHeartBeat()
{
	m_heartbeat++;
}

void UasProvideProcess::TimerFired()
{
	if (m_heartbeat < -5) {
		m_timer.stop();
		Sig_LoseHeartBeat();
		return;	//lost connect
	}
	m_heartbeat--;

	QStringList status;
	status << m_UasP->GetUASName();
	status << QString::number(m_UasP->GetUasID());
	status << QString::number(m_UasP->GetSystemId());
	status << QString::number(m_UasP->GetComponentId());
	status << QString::number(m_UasP->GetUasSysTime() / 1000);
	status << QString::number(m_UasP->GetLoadType());
	if (m_UasP->IsGpsReady()) {
		int satellite = 0;
		double dGpsHDOP = 0.0;
		int gpsfix = 0;
		m_UasP->GetGpsState(satellite, dGpsHDOP, gpsfix);
		QString gpsinfo = QString::number(gpsfix);
		gpsinfo += "-";
		gpsinfo += QString::number(dGpsHDOP);
		gpsinfo += "-";
		gpsinfo += QString::number(satellite);
		status << gpsinfo;
	} else {
		status << "false";
	}
	status << m_UasP->GetPlaneType();

	Sig_StatusChange(status);
}

void UasProvideProcess::Slt_ParamterInfo(int nUasID, int component, QString parameterName, QVariant value)
{
	qDebug() << "get Slt_ParamterInfo" << nUasID << component << parameterName << value;
}

UasProvideProcess::~UasProvideProcess()
{
}
