/* Copyright 2019 Tronlong Elec. Tech. Co. Ltd. All Rights Reserved. */

#include "mainwindow.h"

#include <QFile>
#include <QTextStream>
#include <stdio.h>

static QString g_led0_path = "/sys/class/leds/user-led0";
static QString g_btn_switch_style_sheet =
	"QPushButton{"
	"   background-color:rgba(0, 0, 0, 100); "
	"   border:8px groove gray; "
	"   border-style:outset; "
	"   border-radius:20px; "
	"   color: white;}"
	"QPushButton:pressed{"
	"   background-color:rgba(80, 160, 255, 160); "
	"   color:black;}"
	"QPushButton:hover{"
	"   background-color:rgba(80, 160, 255, 88); "
	"   color:black;}";
static QString g_btn_exit_style_sheet =
	"QPushButton{"
	"   background-color:rgba(0, 0, 0, 88);"
	"   border-radius:12px; "
	"   color:white;}"
	"QPushButton:pressed{"
	"   background-color:rgba(80, 160, 255, 160);"
	"   color:black;}"
	"QPushButton:hover{"
	"   background-color:rgba(80, 160, 255, 88);"
	"   color:black;}";

static bool writeSysfs(const QString &path, const QString &value)
{
	QFile f(path);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Unbuffered)) {
		printf("open %s failed\n", path.toUtf8().constData());
		return false;
	}
	QByteArray data = value.toUtf8();
	qint64 n = f.write(data);
	f.close();
	if (n != data.size()) {
		printf("write %s failed\n", path.toUtf8().constData());
		return false;
	}
	return true;
}

static QString readSysfs(const QString &path)
{
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Unbuffered))
		return QString();
	QByteArray data = f.readAll();
	f.close();
	return QString::fromUtf8(data).trimmed();
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_led_on(false)
{
	/* gpio-leds: stop any kernel trigger before manual brightness control */
	writeSysfs(g_led0_path + "/trigger", "none");

	m_label[0].setParent(this);
	m_label[0].setAlignment(Qt::AlignCenter);

	m_btn_switch[0].setParent(this);
	m_btn_switch[0].setText("Switch LED");
	m_btn_switch[0].setStyleSheet(g_btn_switch_style_sheet);
	m_btn_switch[0].setFocusPolicy(Qt::NoFocus);

	m_btn_exit.setParent(this);
	m_btn_exit.setText("Exit");
	m_btn_exit.setGeometry(20, 20, 80, 25);
	m_btn_exit.setStyleSheet(g_btn_exit_style_sheet);
	m_btn_exit.setFocusPolicy(Qt::NoFocus);
	m_btn_exit.raise();

	connect(&m_btn_exit, SIGNAL(clicked()), this, SLOT(close()));
	connect(&m_btn_switch[0], SIGNAL(clicked()), this, SLOT(handle_btn_clicked()));

	int st = GetLedStatus(g_led0_path);
	m_led_on = (st >= 1);
	UpdateLabelStatus(&m_label[0], m_led_on ? 1 : 0);
}

void MainWindow::resizeEvent(QResizeEvent *)
{
	m_label[0].setGeometry(5, 5, this->width() - 10, this->height() / 2 - 10);
	m_btn_switch[0].setGeometry(5, this->height() / 2 + 5,
				     this->width() - 10, this->height() / 2 - 10);
	m_btn_exit.raise();
}

int MainWindow::GetLedStatus(const QString &led_path)
{
	QString s = readSysfs(led_path + "/brightness");
	if (s.isEmpty())
		return -1;
	return s.toInt();
}

bool MainWindow::SetLedStatus(const QString &led_path, int status)
{
	/* ensure manual mode every time (heartbeat/mmc triggers would fight us) */
	writeSysfs(led_path + "/trigger", "none");
	return writeSysfs(led_path + "/brightness", QString::number(status ? 1 : 0));
}

void MainWindow::UpdateLabelStatus(QLabel *label, int led_status)
{
	QString led_name = "LED";

	if (led_status == 0) {
		label->setText(led_name + " is OFF");
		label->setStyleSheet("border-width:1px;"
				     "border-style:solid; "
				     "border-color:white;"
				     "background-color:rgba(88, 88, 88, 200)");
	} else if (led_status >= 1) {
		label->setText(led_name + " is ON");
		label->setStyleSheet("border-width:1px;"
				     "border-style:solid; "
				     "border-color:white;"
				     "background-color:rgba(00, 205, 00, 255)");
	} else {
		label->setText(led_name + " Abnormal state");
		label->setStyleSheet("border-width:1px;"
				     "border-style:solid;"
				     "border-color:white;"
				     "background-color:rgba(88, 88, 88, 200)");
	}
}

void MainWindow::handle_btn_clicked()
{
	m_led_on = !m_led_on;
	if (!SetLedStatus(g_led0_path, m_led_on ? 1 : 0)) {
		/* roll back UI state if sysfs write failed */
		m_led_on = !m_led_on;
		printf("SetLedStatus failed\n");
	}
	/* prefer software state for UI; re-read as sanity check */
	int st = GetLedStatus(g_led0_path);
	if (st == 0 || st >= 1)
		m_led_on = (st >= 1);
	UpdateLabelStatus(&m_label[0], m_led_on ? 1 : 0);
}

MainWindow::~MainWindow()
{
}
