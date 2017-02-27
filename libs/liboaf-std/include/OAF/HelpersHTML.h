/**
 * @file
 * @brief Интерфейс вспомогательных классов для работы с HTML-кодом
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_HELPERS_HTML_H
#define __OAF_HELPERS_HTML_H

#include <QtCore>

#include <idl/IPropertyBag.h>

#include <OAF/OafStdGlobal.h>

namespace OAF
{
	/**
	 * @brief Ячейка вставленной в HTML-код таблицы
	 */
	class OAFSTD_EXPORT CHTMLTableCell
	{
		/**
		 * @brief Является ли ячейка частью заголовка
		 */
		bool m_is_header;

		/**
		 * @brief Текст ячейки (возможно HTML)
		 */
		QString m_text;

		/**
		 * @brief Сколько столбцов занимает данная ячейка
		 *
		 * При значении меньше 2 - игнорируется
		 */
		int m_span;

		/**
		 * @brief Ширина ячейки в процентах относительно ширины всей таблицы
		 */
		int m_width;

		/**
		 * @brief Тег ячейки (т.е. подпись данных, например "Title")
		 *
		 * Введен в том числе для возможности сделать весь текст ячейки зачеркнутым, кроме тегов
		 */
		QString m_prefix;

		/**
		 * @brief Разделитель префикса и текста
		 */
		QString m_delim;

		/**
		 * @brief Имя CSS-стиля, которой должен использоваться при отображении данной ячейки
		 */
		QString m_style;

	public:
		/**
		 * @brief Создает ячейку HTML-таблицы с заданным текстом
		 *
		 * @param _text Текст внутри ячейки (возможно форматированный HTML)
		 * @param _span Сколько колонок занимает данная ячейка (по умолчанию - одну)
		 */
		CHTMLTableCell (const QString& _text, int _span = 0, int _width = 0,
						const QString& _prefix = QString::null, const QString& _delim = QString::null,
						const QString& _style = QString::null);

		CHTMLTableCell (int _span = 0, int _width = 0,
						const QString& _prefix = QString::null, const QString& _delim = QString::null,
						const QString& _style = QString::null);

		~CHTMLTableCell ();

		void setHeader (bool _h)
		{
			m_is_header = _h;
		}

		bool isHeader () const
		{
			return m_is_header;
		}

		const QString& text () const
		{
			return m_text;
		}

		int span () const
		{
			return m_span;
		}

		int width () const
		{
			return m_width;
		}

		const QString& prefix () const
		{
			return m_prefix;
		}

		const QString& delim () const
		{
			return m_delim;
		}

		const QString& style () const
		{
			return m_style;
		}
	};

	/**
	 * @brief Генератор HTML-кода для различных данных
	 */
	class OAFSTD_EXPORT CHTMLGenerator
	{
		/**
		 * @brief HTML-текст, полученный в результате генерации
		 */
		QString m_html;

		/**
		 * @brief Генерировать ли заголовок HTML (теги html, head и т.д.)
		 */
		bool m_header;

		/**
		 * @brief Таблица стилей
		 */
		QString m_css;

		/**
		 * @brief Текущий стиль отображения таблицы (влияет на все добавляемые ячейки)
		 */
		QString m_current_style;

		/**
		 * @brief Текущий режим отображения данных
		 */
		int m_current_mode;

		/**
		 * @brief Последняя ячейка таблицы
		 */
		CHTMLTableCell m_last_cell;

	public:
		/**
		 * @brief Режим отображения данных по умолчанию
		 */
		static const int MODE_DEFAULT  = 0;

		/**
		 * @brief Режим отображения добавленных данных
		 */
		static const int MODE_INSERTED = 1;

		/**
		 * @brief Режим отображения удалённых данных
		 */
		static const int MODE_REMOVED  = 2;

		/**
		 * @brief Инициализирует HTML-генератор, опционально с использованием CSS
		 */
		CHTMLGenerator (const QString& _css = QString::null, bool _header = true);

		/**
		 * @brief Устанавливает CSS-стиль страницы
		 *
		 * @param _css   исходный код стиля
		 * @param _style название стиля
		 */
		void setCSS (const QString& _css, const QString& _style, bool _header = true);

		/**
		 * @brief Добавляет текст, опционально проводя его дополнительную обработку
		 *        на предмет соответствия HTML-канонам
		 */
		void insertText (const QString& _text, bool _htmlize = true);

		/**
		 * @brief Вставляет HTML-код безо всяких преобразований
		 */
		void insertHTML (const QString& _text);

		/**
		 * @brief Вставляет разрыв строки
		 */
		void insertBR ();

		/**
		 * @brief Переводит генератор в режим предварительно отформатированного текста
		 *
		 * @note По умолчанию отступы и т.п. параметры HTML-текста формируются согласно
		 *       логике браузера, которая запросто может не совпадать с желаемой
		 */
		void beginPreformatted ();

		/**
		 * @brief Отключает режим предварительно отформатированного текста
		 */
		void endPreformatted ();

		/**
		 * @brief Вставляет полужирный текст
		 */
		void insertBold (const QString& _text);

		/**
		 * @brief Вставляет курсив
		 */
		void insertItalic (const QString& _text);

		/**
		 * @brief Вставляет выделенный текст
		 */
		void insertSpan (const QString& _text);

		/**
		 * @brief Вставляет заголовок указанного уровня (от 1 до 6 включительно)
		 */
		void insertHeader (const QString& _text, int _level = 1);

		/**
		 * @brief Вставляет изменение текста (вставка/удаление)
		 */
		void insertDiff (const QString& _text, bool _inserted = true);
		void beginDiff (bool _inserted = true);
		void endDiff (bool _inserted = true);

		/**
		 * @name Методы для работы с таблицами
		 */
		/** @{*/
		void beginTable (const QString& _style = QString::null, int _mode = MODE_DEFAULT);
		void beginRow ();
		void beginCell (const CHTMLTableCell& _cell);
		void endCell ();
		void endRow ();
		void endTable ();
		/** @}*/

		/**
		 * @brief Формирует и возвращает итоговый HTML-код
		 */
		QString result () const;

		/**
		 * @brief Сбросить состояние генератора
		 */
		void clear ();
	};
}

#endif /* __OAF_HELPERS_HTML_H */
