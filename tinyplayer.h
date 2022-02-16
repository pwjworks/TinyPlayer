#pragma once

#include <QtWidgets/QWidget>
#include "ui_tinyplayer.h"

class TinyPlayer : public QWidget
{
    Q_OBJECT

public:
    TinyPlayer(QWidget *parent = Q_NULLPTR);

private:
    Ui::TinyPlayerClass ui;
};
