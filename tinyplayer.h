#pragma once

#include <QtWidgets/QWidget>
#include "ui_tinyplayer.h"

class TinyPlayer : public QWidget
{
	Q_OBJECT

public:
	TinyPlayer(QWidget* parent = Q_NULLPTR);
	~TinyPlayer();
	// 定时器 滑动条显示
	void timerEvent(QTimerEvent* e);

	// 窗口尺寸变化
	void resizeEvent(QResizeEvent* e);

	// 双击全屏
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
