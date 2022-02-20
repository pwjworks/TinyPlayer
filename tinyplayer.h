#pragma once

#include <QtWidgets/QWidget>
#include "ui_tinyplayer.h"

class TinyPlayer : public QWidget
{
	Q_OBJECT

public:
	TinyPlayer(QWidget* parent = Q_NULLPTR);
	~TinyPlayer();
	// ��ʱ�� ��������ʾ
	void timerEvent(QTimerEvent* e);

	// ���ڳߴ�仯
	void resizeEvent(QResizeEvent* e);

	// ˫��ȫ��
	void mouseDubleClickEvent(QMouseEvent* e);

	void SetPause(bool isPause);


public slots:
	void OpenFile();
	void PlayOrPause();
	void SliderPress();
	void SliderRelease();
	//private:
public:
	Ui::TinyPlayerClass ui;
private:
	bool isSliderPress = false;
};
