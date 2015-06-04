#include <QPainter>
#include "frontends/qt/lcdwidget.h"
#include <QDebug>
#include <stdint.h>

LCDWidget::LCDWidget(QWidget *parent) :
	QWidget(parent), image(NULL)
{
	setFixedSize(160, 144);
}

void LCDWidget::setData(uchar *data)
{
	if(image != NULL)
	{
		delete image;
		image = NULL;
	}

	if(data != NULL)
	{
		image = new QImage(data, 160, 144, QImage::Format_RGB32);
	}
}

LCDWidget::~LCDWidget()
{
	if(image != NULL)
	{
		delete image;
	}
}

void LCDWidget::paintEvent(QPaintEvent *event)
{
	QPainter p;

	if(image == NULL || !p.begin(this))
	{
		return;
	}

	p.drawImage(QPoint(0, 0), *image);

	p.end();
}
