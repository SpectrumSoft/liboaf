/**
 * @file
 * @brief Интерфейс вспомогательных классов для работы с HTML-кодом
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __HTMLHELPERS_H
#define __HTMLHELPERS_H

#include <QtCore>

#include <idl/IPropertyBag.h>

#include <OAF/OafStdGlobal.h>

namespace OAF
{
	/**
	 * @brief Ячейка вставленной в HTML-код таблицы
	 */
	struct OAFSTD_EXPORT CHtmlTableCell
	{
		/**
		 * @brief Тег ячейки (т.е. подпись данных, например "Title");
		 * введен в том числе для возможности сделать весь текст ячейки зачеркнутым, кроме тегов
		 *
		 * @note Двоеточие в конце тега добавляется автоматически
		 */
		QString m_tag;

		QString m_tag_postfix;

		/**
		 * @brief Текст ячейки (возможно HTML)
		 */
		QString m_text;

		/**
		 * @brief Ширина ячейки в процентах относительно все таблицы
		 */
		int m_width;

		/**
		 * @brief Сколько столбцов занимает данная ячейка (при значении меньше 2 - игнорируется)
		 */
		int m_col_span;

		/**
		 * @brief Имя CSS-стиля, которой должен использоваться при отображении данной ячейки
		 */
		QString m_style_name;

		/**
		 * @brief Является ли ячейка заголовочной
		 */
		bool m_is_header;

		/**
		 * @brief Создает пустую ячейку HTML-таблицы
		 */
		CHtmlTableCell ();

		/**
		 * @brief Создает ячейку HTML-таблицы с заданным текстом
		 * @param _text Текст внутри ячейки (возможно форматированный HTML)
		 * @param _colSpan Сколько колонок занимает данная ячейка (по умолчанию - одну)
		 * @param _isHeader Является ли ячейка заголовком (выделяется более жирный шрифтом и т.д.)
		 */
		CHtmlTableCell (const QString& _tag, const QString& _text,
						int _colSpan = 0, int _width = 0,
						const QString& _tag_postfix = ": ",
						const QString& _style_name = QString ());

		CHtmlTableCell (const QString& _tag, int _col_span = 0, int _width = 0, const QString& _tag_postfix = ": ");

		~CHtmlTableCell ();
	};

	/**
	 * @brief Служит для генерации HTML-кода для различных данных
	 */
	class OAFSTD_EXPORT CHtmlGenerator
	{
		/**
		 * @brief HTML-текст, полученный в результате генерации
		 */
		QString m_html;

		/**
		 * @brief Генерировать ли заголовок HTML (теги html, head и т.д.)
		 */
		bool m_header;

	public:
		/**
		 * @brief Стили HTML-таблицы: добавленный, удаленный и обычный объект данных
		 */
		enum TableStyle { _TableInserted = 0, _TableRemoved, _TableNormal };

	private:
		QString m_css_source;
		QString m_table_style_name;
		TableStyle m_table_style;
		CHtmlTableCell m_last_cell;

	public:
		typedef QVector<CHtmlTableCell> Cells;

		/**
		 * @brief Инициализирует HTML-генератор, опционально с использованием CSS
		 */
		CHtmlGenerator (const QString& _css_source = QString (), bool _header = true);

		/**
		 * @brief Сбросить состояние генератора
		 */
		void clear ();

		/**
		 * @brief setCss Устанавливает CSS-стиль страницы
		 * @param _css_name Название стиля
		 * @param _css_source Исходный код стиля
		 */
		void setCss (const QString& _css_name, const QString& _css_source);

		/**
		 * @brief Добавляет текст, опционально проводя его дополнительную обработку
		 * на предмет соответствия HTML-канонам
		 */
		void insertText (const QString& _text, bool _htmlize = true);

		/**
		 * @brief Вставляет HTML-код безо всяких преобразований
		 */
		void insertHtml (const QString& _text);

		/**
		 * @brief Вставляет разрыв строки
		 */
		void insertLineBreak ();

		/**
		 * @brief Переводит генератор в режим предварительно отформатированного текста
		 *
		 * @note По умолчанию отступы и т.п. параметры HTML-текста формируются согласно логике браузера,
		 * которая запросто может не совпадать с желаемой
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

		void insertSpan (const QString& _text);

		/**
		 * @brief Вставляет заголовок указанного уровня (от 1 до 6 включая)
		 */
		void insertHeader (const QString& _header, int _level = 1);

		/**
		 * @brief Вставляет изменение текста (вставка/удаление)
		 * @param _diff_text
		 * @param _inserted
		 */
		void insertDiff (const QString& _diff_text, bool _inserted = true);

		void beginDiff (bool _inserted = true);
		void endDiff (bool _inserted = true);

		/**
		 * @name Методы для работы с таблицами
		 */
		/** @{*/
		void beginTable (const QString& _style_name = QString (), TableStyle _style = _TableNormal);
		void beginRow ();
		void beginCell (const CHtmlTableCell& _cell);
		void endCell ();
		void endRow ();
		void endTable ();
		/** @}*/

		/**
		 * @brief Формирует и возвращает итоговый HTML-код
		 */
		QString result () const;
	};
}

#endif /* __HTMLHELPERS_H */
