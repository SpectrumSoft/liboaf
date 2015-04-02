/**
 * @file
 * @brief Интерфейс метки с автоматическим сокращением текста
 * @author Novikov Dmitriy <novikovdimka@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __CELIDED_LABEL_H
#define __CELIDED_LABEL_H

#include <QtCore>
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK (5, 0, 0))
#include <QtWidgets>
#endif

#include <OAF/OafGuiGlobal.h>

namespace OAF
{
	/**
	 * @brief Метка, дополняющяяся многоточием, если на экране не хватает места для
	 *        отображения помещённого в неё текста
	 */
	class OAFGUI_EXPORT CElidedLabel : public QLabel
	{
		Q_OBJECT

		/**
		 * @brief Метод вывода многоточия
		 */
		Qt::TextElideMode m_elide_mode;

		/**
		 * @brief Кэшированный сокращённый текст
		 */
		QString m_cached_elided_text;

	protected:
		/**
		 * @brief Кэшируем сокращённый текст, чтобы не пересчитывать его при каждой прорисовке
		 */
		void cacheElidedText (int _width);

	protected:
		void paintEvent (QPaintEvent* _event);
		void resizeEvent (QResizeEvent* _event);

	public:
		/**
		 * @brief Определим несколько конструкторов для удобства использования
		 */
		/** @{ */
		explicit CElidedLabel (QWidget* _parent = 0, Qt::WindowFlags _flags = 0);
		explicit CElidedLabel (const QString& _text, QWidget* _parent = 0, Qt::WindowFlags _flags = 0);
		explicit CElidedLabel (const QString& _text, Qt::TextElideMode _elide_mode = Qt::ElideRight,
							   QWidget* _parent = 0, Qt::WindowFlags _flags = 0);
		/** @} */

		/**
		 * @brief Настроить режим сокращения текста
		 */
		void setElideMode (Qt::TextElideMode _elide_mode);

		/**
		 * @brief Получить текущий режим сокращения текста
		 */
		Qt::TextElideMode elideMode () const;

		/**
		 * @brief Установить текст метки
		 *
		 * NOTE: используется перекрытие метода QLabel::setText, т.к. он не виртуальный
		 *		 в связи с чем будет некорректно работать при обращение, как к QLabel
		 */
		void setText (const QString& _text);
	};
}

#endif /* __CELIDED_LABEL_H */
