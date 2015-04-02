/**
 * @file
 * @brief Реализация вспомогательных классов для работы с HTML-кодом
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/HtmlHelpers.h>

static const QString HTML4_HEADER = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
		//
		// FIXME: валидный HTML требует определения данных атрибутов, но QWebView его не понимает;
		// при раскомментировании не забыть убрать ">" в конце верхней строки!
		//
//	"[\n"
//	"<!ATTLIST del user_title CDATA #IMPLIED>\n"
//	"<!ATTLIST del user_title_2 CDATA #IMPLIED>\n"
//	"<!ATTLIST ins user_title CDATA #IMPLIED>\n"
//	"<!ATTLIST ins user_title_2 CDATA #IMPLIED>\n"
//	"]>\n"
		"<html>\n"
		"<head>\n"
		"<title>" + QTranslator::tr ("Final report") + "</title>\n"
		"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n"
		"%1"
		"</head>\n"
		"<body>\n";

OAF::CHtmlTableCell::CHtmlTableCell ():
	m_tag_postfix (": "),
	m_width (0),
	m_col_span (0),
	m_is_header (false)
{
}

OAF::CHtmlTableCell::CHtmlTableCell (const QString & _tag, const QString& _text,
								int _colSpan, int _width,
								const QString& _tag_postfix,
								const QString& _style_name):
	m_tag (_tag),
	m_tag_postfix (_tag_postfix),
	m_width (_width),
	m_col_span (_colSpan),
	m_style_name (_style_name),
	m_is_header (false)
{
	m_text = _text;
}

OAF::CHtmlTableCell::CHtmlTableCell (const QString& _tag, int _col_span, int _width, const QString& _tag_postfix):
	m_tag (_tag),
	m_tag_postfix (_tag_postfix),
	m_width (_width),
	m_col_span (_col_span),
	m_is_header (false)
{
}

OAF::CHtmlTableCell::~CHtmlTableCell ()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

OAF::CHtmlGenerator::CHtmlGenerator (const QString& _css_source, bool _header)
{
	m_header = _header;
	m_css_source = _css_source;

	//
	// Инициализируем HTML-текст, указав его тип документа, CSS и основные теги
	//
	if (_header)
		m_html = HTML4_HEADER.arg (_css_source);
}

void
OAF::CHtmlGenerator::clear ()
{
	//
	// Заново формируем заголовок HTML (если было указано при создании)
	//
	m_html.clear ();
	if (m_header)
		m_html = HTML4_HEADER.arg (m_css_source);

	m_table_style = _TableNormal;
//	m_table_style_name.clear ();
	m_last_cell = OAF::CHtmlTableCell ();
}

void
OAF::CHtmlGenerator::setCss (const QString& _css_name, const QString& _css_source)
{
	m_header = true;
	m_css_source = _css_source;
	m_table_style_name = _css_name;

	m_html = HTML4_HEADER.arg (_css_source);
}

void
OAF::CHtmlGenerator::beginDiff (bool _inserted)
{
	if (_inserted)
		m_html += "<ins>";
	else
		m_html += "<del>";
}

void
OAF::CHtmlGenerator::endDiff (bool _inserted)
{
	if (_inserted)
		m_html += "</ins>";
	else
		m_html += "</del>";
}

void
OAF::CHtmlGenerator::beginTable (const QString& _style_name, TableStyle _style)
{
	//
	// Сохраняем название стиля таблицы для возможного последуюшего использования
	// в ее ячейках
	//
	m_table_style_name = _style_name;

	QString html_table = "<table";
	//
	// Задаем CSS-стиль таблицы, если он указан
	//
	if (! _style_name.isEmpty ())
		html_table += " class=\"" + _style_name + "\"";

	//
	// Задаем вывод таблицы в качестве добавленного/удаленного объекта, если задано
	//
	m_table_style = _style;
	if (_style == _TableInserted)
		html_table += " style=\"background:#e6ffe6;\"";
	else if (_style == _TableRemoved)
		html_table += " style=\"background:#ffe6e6;\"";
	html_table += ">\n";

	//
	// Вставляем таблицу в HTML-документ
	//
	m_html += html_table;
}

void
OAF::CHtmlGenerator::endTable ()
{
	//
	// Закрываем тег таблицы
	//
	m_html += "</table>\n";
}

void
OAF::CHtmlGenerator::beginRow ()
{
	m_html += "<tr>\n";
}

void
OAF::CHtmlGenerator::endRow ()
{
	m_html += "</tr>\n";
}

void
OAF::CHtmlGenerator::beginCell (const OAF::CHtmlTableCell& _cell)
{
	QString html_cell;

	//
	// Ячейка может быть рядовой, а может - заголовочной
	//
	if (_cell.m_is_header)
		html_cell += "<th";
	else
		html_cell += "<td";

	//
	// Задаем стиль ячейки, если он указан - иначе используем табличный
	//
	if (! _cell.m_style_name.isEmpty ())
		html_cell += " class=\"" + _cell.m_style_name + "\" ";
	else
		html_cell += " class=\"" + m_table_style_name + "\" ";

	//
	// Ширину ячейки можно задавать явно (высота будет регулироваться подписями вроде "ID:")
	//
	if (_cell.m_width > 0)
		html_cell += QString (" width=\"%1%\"").arg (_cell.m_width);

	//
	// Несколько столбцов таблицы можно объединять в один
	//
	if (_cell.m_col_span >= 2)
		html_cell += QString (" colspan=\"%1\"").arg (_cell.m_col_span);
	html_cell += ">\n";

	//
	// Добавляем "заголовок" ячейки (часть ее текста перед разделителем)
	//
	html_cell += (_cell.m_tag + _cell.m_tag_postfix);
	//
	// Добавляем разделитель и текст, если они указаны
	//
	if (!_cell.m_text.isEmpty ())
		html_cell += _cell.m_text;

	//
	// Сохраняем данную ячейку для использования при закрытии тега
	//
	m_last_cell = _cell;

	//
	// Добавляем открывающие теги ячейки в HTML-документ
	//
	m_html += html_cell;
}

void
OAF::CHtmlGenerator::endCell ()
{
	QString html_cell;

	if (m_last_cell.m_is_header)
		html_cell += " </th>\n";
	else
		html_cell += " </td>\n";

	//
	// Добавляем закрывающие теги ячейки в HTML-документ
	//
	m_html += html_cell;
}

void
OAF::CHtmlGenerator::insertText (const QString& _text, bool _htmlize)
{
	//
	// Пробельные символы заменяем в любом случае - без этого текст корректно отображаться точно не будет
	//
	QString htmled_text = _text;
	htmled_text.replace ("\t",   "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
	htmled_text.replace ("\r\n", "<br>");
	htmled_text.replace ("\n",   "<br>");

	//
	// Заменяем запрещенные в HTML спец-символы на их аналоги (см. http://www.w3.org/TR/html4/charset.html#h-5.4)
	//
	if (_htmlize)
	{
		htmled_text.replace ("<",    "&lt");
		htmled_text.replace (">",    "&gt");
		htmled_text.replace ("&",    "&amp");
		htmled_text.replace ("\"",   "&quot");
	}
	m_html += htmled_text;
}

void
OAF::CHtmlGenerator::insertHtml (const QString& _text)
{
	m_html += _text;
}

void
OAF::CHtmlGenerator::insertLineBreak ()
{
	m_html += "<br>";
}

void
OAF::CHtmlGenerator::beginPreformatted ()
{
	m_html += "<pre>";
}

void
OAF::CHtmlGenerator::endPreformatted ()
{
	m_html += "</pre>";
}

void
OAF::CHtmlGenerator::insertBold (const QString& _text)
{
	m_html += "<b>";
	insertText (_text, false);
	m_html += "</b>";
}

void
OAF::CHtmlGenerator::insertItalic (const QString& _text)
{
	m_html += "<i>";
	insertText (_text, false);
	m_html += "</i>";
}

void
OAF::CHtmlGenerator::insertSpan (const QString& _text)
{
	m_html += "<span>";
	insertText (_text);
	m_html += "</span>";
}

void
OAF::CHtmlGenerator::insertHeader (const QString& _header, int _level)
{
	Q_ASSERT ((_level >= 1) && (_level <= 6));
	//
	// Формируем тег заголовка в зависимости от заданного уровня
	//
	QString header_open_tag = "<h" + QString::number (_level) + ">";
	QString header_close_tag = "</h" + QString::number (_level) + ">";
	//
	// Добавляем заголовок в HTML
	//
	m_html += header_open_tag;
	insertText (_header);
	m_html += header_close_tag;
	m_html += "\n";
}

void
OAF::CHtmlGenerator::insertDiff (const QString& _diff_text, bool _inserted)
{
	if (_inserted)
		m_html += "<ins>" + _diff_text + "</ins>";
	else
		m_html += "<del>" + _diff_text + "</del>";
}

QString
OAF::CHtmlGenerator::result () const
{
	if (m_header)
	{
		static const QString end_tags = "\n"
										"</body>\n"
										"</html>\n";
		return (m_html + end_tags);
	}
	return m_html;
}
