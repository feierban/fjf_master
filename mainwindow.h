#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <Feimaplayer.h>
#include <qlabel.h>
#include <DecodeThread.h>
#include <SerialProg.h>
#include <UasProvideProcess.h>

#define FMSERIALPORT_BUILD_SHARED

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
	void Sig_SetConnectConf(QString ip, int port);
	void Sig_GetUasProvide();

private slots:
    void on_Button_Sel_clicked();
    void on_Button_Decode_clicked();
	void on_Button_Udp_clicked();
	void on_Button_Stop_clicked();
	void on_Button_Serial_clicked();
	void on_Button_Action_clicked();

    void slt_getoneimg(QImage img);

private slots:
	void Slt_GetUasProvide(CFmUasProvide *);
	void Slt_UasDisconnect();
	void Slt_LoseHeartBeat();
	void Slt_GetMultirotorPlaneInfo(QStringList);
	void Slt_PlaneSelInfoChange(int index);
	void Slt_StatusChange(QStringList status);

private:
	void Update_GPSInfo(int satellite, double dGpsHDOP, int gpsfix);
	void Fresh_MutirInfo(int index=0);

    Ui::MainWindow *ui;
	QStringList m_baseinfo;

	DecodeThread *m_thread;
    feimaplayer *player;
    QLabel* label;
    QPixmap mp;

	QThread *qthread;
	QThread *fthread;

	SerialProg *serial;
	CFmUasProvide *m_pUA;
	UasProvideProcess *m_uasp;
};

#endif // MAINWINDOW_H
