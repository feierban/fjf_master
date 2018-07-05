#pragma once
#include <qthread.h>
#include <qtimer.h>
#include <fmLinkCore/fmuasprovide.h>
class UasProvideProcess :public QObject
{
	Q_OBJECT
public:
	UasProvideProcess(CFmUasProvide*);
	~UasProvideProcess();
	void UasMain();

signals:
	void Sig_StatusChange(QStringList);
	void Sig_LoseHeartBeat();

private slots:
	void Slt_GetHeartBeat();
	void Slt_ParamterInfo(int nUasID, int component, QString parameterName, QVariant value);

private:
	void TimerFired();

	int m_heartbeat;
	CFmUasProvide* m_UasP;
	QThread m_UavProcessThread;
	QTimer m_timer;
};

