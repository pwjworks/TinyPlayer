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

	startTimer(40);
}

void TinyPlayer::timerEvent(QTimerEvent* e) {
	if (isSliderPress)return;
	long long total = dt.totalMs;
	if (dt.totalMs > 0) {
		double pos = dt.pts / (double)total;
		int v = ui.playPos->maximum() * pos;
		ui.playPos->setValue(v);
	}
}


void TinyPlayer::OpenFile() {
	QString name = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择视频文件"));
	if (name.isEmpty()) return;
	this->setWindowTitle(name);
	if (!dt.Open(name.toLocal8Bit(), ui.openGLWidget)) {
		QMessageBox::information(0, "error", "open file failed!");
		return;
	}
	SetPause(dt.isPause);
}

void TinyPlayer::mouseDubleClickEvent(QMouseEvent* e) {
	if (isFullScreen())
		this->showNormal();
	else
		this->showFullScreen();
}

void TinyPlayer::resizeEvent(QResizeEvent* e) {
	ui.playPos->move(50, this->height() - 100);
	ui.playPos->resize(this->width() - 100, ui.playPos->height());
	ui.pushButton->move(100, this->height() - 150);
	ui.isPlay->move(ui.pushButton->x() + ui.pushButton->width() + 10, ui.pushButton->y());
	ui.openGLWidget->resize(this->size());
}

void TinyPlayer::PlayOrPause() {
	bool isPause = !dt.isPause;
	SetPause(isPause);
	dt.SetPause(isPause);
}

void TinyPlayer::SetPause(bool isPause) {
	if (isPause)
		ui.isPlay->setText(QString::fromLocal8Bit("播 放"));
	else
		ui.isPlay->setText(QString::fromLocal8Bit("暂 停"));
}

void TinyPlayer::SliderPress() {
	isSliderPress = true;
}
void TinyPlayer::SliderRelease() {
	isSliderPress = false;
	double pos = 0.0;
	pos = (double)ui.playPos->value() / (double)ui.playPos->maximum();
	dt.Seek(pos);
}
TinyPlayer::~TinyPlayer() {
	dt.Close();
}