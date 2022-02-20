#include "tinyplayer.h"
#include <QFileDialog>
#include "XDemuxThread.h"
#include <QMessageBox>
static XDemuxThread dt;
TinyPlayer::TinyPlayer(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	dt.Start();

}


void TinyPlayer::OpenFile() {
	QString name = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择视频文件"));
	if (name.isEmpty()) return;
	this->setWindowTitle(name);
	if (!dt.Open(name.toLocal8Bit(), ui.openGLWidget)) {
		QMessageBox::information(0, "error", "open file failed!");
		return;
	}
}