#include <QPainter>
#include "frontends/qt4/lcdwidget.h"
#include <QDebug>

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
		image = new QImage(data, 160, 144, QImage::Format_ARGB32);
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

	p.eraseRect(0, 0, 160, 144);
	p.drawImage(0, 0, *image);

	p.end();
}
