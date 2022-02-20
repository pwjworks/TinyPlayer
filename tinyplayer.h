#pragma once

#include <QtWidgets/QWidget>
#include "ui_tinyplayer.h"

class TinyPlayer : public QWidget
{
	Q_OBJECT

public:
	TinyPlayer(QWidget* parent = Q_NULLPTR);

public slots:
	void OpenFile();
	//private:
public:
	Ui::TinyPlayerClass ui;
};
