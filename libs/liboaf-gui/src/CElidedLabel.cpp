/**
 * @file
 * @brief Реализация метки с автоматическим сокращением текста
 * @author Novikov Dmitriy <novikovdimka@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/CElidedLabel.h>

void
OAF::CElidedLabel::cacheElidedText (int _width)
{
	//
	// Кэшируем доступный для вывода текст
	//
	m_cached_elided_text =
			fontMetrics ().elidedText (text (), m_elide_mode, _width, Qt::TextShowMnemonic);
}

void
OAF::CElidedLabel::paintEvent (QPaintEvent* _event)
{
	//
	// Если не используются сокращения, выводим стандартным образом
	//
	if (m_elide_mode == Qt::ElideNone)
	{
		QLabel::paintEvent (_event);
	}
	//
	// В противном случае рисуем текст вручную
	//
	else
	{
		QPainter p (this);
		p.drawText (0, 0,
					geometry ().width (),
					geometry ().height (),
					alignment (),
					m_cached_elided_text);
	}
}

void
OAF::CElidedLabel::resizeEvent (QResizeEvent* _event)
{
	QLabel::resizeEvent (_event);

	//
	// При изменении размеров виджета закэшируем текст заново
	//
	cacheElidedText (_event->size ().width ());
}


OAF::CElidedLabel::CElidedLabel (QWidget* _parent, Qt::WindowFlags _flags)
	: QLabel (_parent, _flags), m_elide_mode (Qt::ElideRight)
{
	setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Preferred);
}

OAF::CElidedLabel::CElidedLabel (const QString& _text, QWidget* _parent, Qt::WindowFlags _flags)
	: QLabel (_text, _parent, _flags), m_elide_mode (Qt::ElideRight)
{
	setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Preferred);
}

OAF::CElidedLabel::CElidedLabel (const QString& _text, Qt::TextElideMode _elide_mode, QWidget*_parent,
							Qt::WindowFlags _flags)
	: QLabel (_text, _parent, _flags), m_elide_mode (_elide_mode)
{
	setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Preferred);
}

void
OAF::CElidedLabel::setElideMode (Qt::TextElideMode _elide_mode)
{
	m_elide_mode = _elide_mode;
	updateGeometry ();
}

Qt::TextElideMode
OAF::CElidedLabel::elideMode () const
{
	 return m_elide_mode;
}

void
OAF::CElidedLabel::setText (const QString &_text)
{
	QLabel::setText (_text);
	cacheElidedText (geometry ().width ());
	setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Preferred);
}
