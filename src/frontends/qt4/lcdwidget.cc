#include <QPainter>
#include "frontends/qt4/lcdwidget.h"
#include <QDebug>

LCDWidget::LCDWidget(QWidget *parent) :
	QWidget(parent)
{
	setFixedSize(160, 144);
}

void LCDWidget::paintEvent(QPaintEvent *event)
{
	QPainter p;

	if(!p.begin(this))
	{
		return;
	}

	p.eraseRect(0, 0, 160, 144);
	p.drawImage(0, 0, currentFrame);

	p.end();
}
