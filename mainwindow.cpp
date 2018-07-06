#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "qdebug.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //ui->Input_File->setText("E:/resource/VADemo.h264");
    ui->Input_ip->setText("192.168.2.100");
	ui->Input_port->setText("19502");

    player = new feimaplayer();
	m_thread = new DecodeThread();

	//qthread
	qthread = new QThread();
	m_thread->moveToThread(qthread);
	connect(qthread, &QThread::started, m_thread, &DecodeThread::run);
	fthread = new QThread();
	player->moveToThread(fthread);
	connect(fthread, &QThread::started, player, &feimaplayer::ThreadMain);

	//slt of other class
    connect(player, &feimaplayer::sig_sendoneimg, this, &MainWindow::slt_getoneimg);
	connect(m_thread, &DecodeThread::Sig_Sendimg, this, &MainWindow::slt_getoneimg);

	//slt of other ui
	connect(ui->Button_UDP, &QPushButton::clicked, this, &MainWindow::on_Button_Udp_clicked);
//	connect(ui->Button_stop, &QPushButton::clicked, this, &MainWindow::on_Button_Stop_clicked);
	connect(ui->Button_serial, &QPushButton::clicked, this, &MainWindow::on_Button_Serial_clicked);
	connect(ui->Button_action, &QPushButton::clicked, this, &MainWindow::on_Button_Action_clicked);
	connect(ui->ComboBox_other, SIGNAL(activated(int)), this, SLOT(Slt_PlaneSelInfoChange(int)));

	m_pUA = NULL;
	m_uasp = NULL;
	QStringList items;
	items << u8"GPS固件" << u8"电池管理系统";
	items << u8"磁力计1固件" << u8"磁力计2固件" << u8"载荷固件";
	items << u8"电调1固件" << u8"电调2固件" << u8"电调3固件" << u8"电调4固件";
	items << u8"超声波固件" << u8"视觉固件" << u8"CPLD" << u8"FPGA";
	items << u8"电池版本" << u8"LiDir";

	ui->ComboBox_other->addItems(items);
	ui->ComboBox_other->setDisabled(true);
    label=new QLabel("",0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Button_Sel_clicked()
{
    QString file = QFileDialog::getOpenFileName();
    //ui->Input_File->setText(file);
}

void MainWindow::slt_getoneimg(QImage img)
{
    QPixmap pix = QPixmap::fromImage(img);
	pix = pix.scaled(ui->Label_pic->size(), Qt::KeepAspectRatio);
	qDebug() << "there is " << img.size().height() << img.size().width();
	ui->Label_pic->setScaledContents(true);
    ui->Label_pic->setPixmap(pix);
    ui->Label_pic->show();
    ui->Label_pic->repaint();
}

void MainWindow::on_Button_Decode_clicked()
{
	/*
    QString text = ui->Input_File->text();
    player->load(text);
	fthread->start();
	*/
}

void MainWindow::on_Button_Serial_clicked()
{
	serial = new SerialProg();
	connect(serial, &SerialProg::Sig_GetUasProvide, this, &MainWindow::Slt_GetUasProvide);
	connect(serial, &SerialProg::Sig_GetMultirotorPlaneInfo, this, &MainWindow::Slt_GetMultirotorPlaneInfo);
	ui->Label_status->setText("connecting...");
	serial->SerialMain();
}

void MainWindow::on_Button_Udp_clicked()
{
	QString text_ip = ui->Input_ip->text();
	QString text_port = ui->Input_port->text();
	int port = text_port.toInt();
	m_thread->SetUdpConfig(text_ip, port);
	qthread->start();
}

void MainWindow::on_Button_Stop_clicked()
{
	qthread->exit();
}

void MainWindow::Update_GPSInfo(int satellite, double dGpsHDOP, int gpsfix)
{
	if (m_pUA->IsGpsReady()) {
		QString tex = QString::number(gpsfix);
		tex += "-";
		tex += QString::number(dGpsHDOP);
		tex += "-";
		tex += QString::number(satellite);
		ui->Input_gpsstatus->setText(tex);
	}
	return;
}

void MainWindow::Slt_GetUasProvide(CFmUasProvide * pUasProd)
{
	if (m_pUA == pUasProd)
		return;
	m_pUA = pUasProd;
	ui->Label_status->setText("connected");
	ui->Button_serial->setEnabled(false);
	ui->ComboBox_other->setDisabled(false);

	if (m_uasp) {
		disconnect(m_uasp, &UasProvideProcess::Sig_StatusChange, this, &MainWindow::Slt_StatusChange);
		disconnect(serial, &SerialProg::Sig_UasDisconnect, this, &MainWindow::Slt_UasDisconnect);
		disconnect(this, &MainWindow::Sig_GetUasProvide, m_uasp, &UasProvideProcess::UasMain);
		delete m_uasp;
		m_uasp = NULL;
	}
	m_uasp = new UasProvideProcess(m_pUA);

	connect(m_uasp, &UasProvideProcess::Sig_LoseHeartBeat, this, &MainWindow::Slt_LoseHeartBeat);
	connect(m_uasp, &UasProvideProcess::Sig_StatusChange, this, &MainWindow::Slt_StatusChange);
	connect(serial, &SerialProg::Sig_UasDisconnect, this, &MainWindow::Slt_UasDisconnect);
	connect(this, &MainWindow::Sig_GetUasProvide, m_uasp, &UasProvideProcess::UasMain);
	emit Sig_GetUasProvide();

}

void MainWindow::Slt_LoseHeartBeat()
{
	ui->Label_status->setText("lose heart beat");
}

void MainWindow::Slt_StatusChange(QStringList status)
{
	if (status.size() < 8)
		return;

	ui->Input_uasname->setText(status[0]);
	ui->Input_uasid->setText(status[1]);
	ui->Input_sysid->setText(status[2]);
	ui->Input_compoid->setText(status[3]);
	ui->Input_uastime->setText(status[4]);
	ui->Input_loadtype->setText(status[5]);
	ui->Input_gpsstatus->setText(status[6]);
	ui->Input_planetype->setText(status[7]);
}

void MainWindow::Slt_GetMultirotorPlaneInfo(QStringList info)
{
	m_baseinfo = info;
	Fresh_MutirInfo();
}

void MainWindow::Slt_PlaneSelInfoChange(int index)
{
	Fresh_MutirInfo(index);
}
void MainWindow::Fresh_MutirInfo(int index)
{
	ui->Label_info1->setText(m_baseinfo[0]);
	ui->Label_info2->setText(m_baseinfo[1]);
	ui->Label_info3->setText(m_baseinfo[2]);
	ui->Label_info4->setText(m_baseinfo[3]);
	ui->Label_info5->setText(m_baseinfo[4]);
	ui->Label_info6->setText(m_baseinfo[5]);

	ui->Label_info7->setText(m_baseinfo[6 + index]);
}

void MainWindow::Slt_UasDisconnect()
{
	if (m_pUA == NULL)
		return;

	m_pUA = NULL;
	ui->Label_status->setText("disconnected");
	ui->Button_serial->setEnabled(true);
	ui->ComboBox_other->setDisabled(true);

}

void MainWindow::on_Button_Action_clicked()
{
	if (m_pUA) {
		//qDebug() << "SEND COMMAND SendParachuteReplaceMsg";
		//m_pUA->SendParachuteReplaceMsg(); //no return;
		//qDebug() << "SEND COMMAND RequestFlightMissionID";
		//m_pUA->RequestFlightMissionID();	//no return;
		//qDebug() << "SEND COMMAND RequestPosFlightMissionID";
		//m_pUA->RequestPosFlightMissionID(); //no return;
		//qDebug() << "SEND COMMAND SendMileAge";
		//m_pUA->SendMileAge();	//useful
		//qDebug() << "SEND COMMAND LogEraseAll";
		//m_pUA->LogEraseAll(); //no return;
		//qDebug() << "SEND COMMAND LogRequestEnd";
		//m_pUA->LogRequestEnd(); //no return;
		//qDebug() << "SEND COMMAND RequestLoadType";
		//m_pUA->RequestLoadType();
		//qDebug() << "SEND COMMAND RequestPitotSn";
		//m_pUA->RequestPitotSn();
		//qDebug() << "SEND COMMAND Shutdown";
		//m_pUA->Shutdown(); //may dangerous
		//qDebug() << "SEND COMMAND SendPatachuteChanged";
		//m_pUA->SendPatachuteChanged(); //no return
		//qDebug() << "SEND COMMAND ArmSystem";
		//m_pUA->ArmSystem(); //no return
		//qDebug() << "SEND COMMAND DisarmSystem";
		//m_pUA->DisarmSystem(); //no return
		//qDebug() << "SEND COMMAND DisarmSystem";
		//m_pUA->DisarmSystem(); //no return
		//qDebug() << "SEND COMMAND batteryInfo";
		//m_pUA->batteryInfo(); //no return
		

		//qDebug() << "SEND COMMAND EnableHeartBeat";
		//m_pUA->EnableHeartBeat(false); 

		//qDebug() << "SEND COMMAND IsFixwingPlane";
		//qDebug() << m_pUA->IsFixwingPlane(); 
		//qDebug() << "SEND COMMAND GetPlaneType";
		//qDebug() << m_pUA->GetPlaneType();
		//qDebug() << "SEND COMMAND IsArmed";
		//qDebug() << m_pUA->IsArmed();

	}
}
