/**
 * @file
 * @brief Реализация вспомогательных классов для работы с HTML-кодом
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/HtmlHelpers.h>

using namespace OAF;

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

CHtmlTableCell::CHtmlTableCell () : m_tag_postfix (": "), m_width (0), m_col_span (0), m_is_header (false)
{}

CHtmlTableCell::CHtmlTableCell (const QString & _tag, const QString& _text,
								int _colSpan, int _width,
								const QString& _tag_postfix,
								const QString& _style_name):
	m_tag (_tag),
	m_tag_postfix (_tag_postfix),
	m_width (_width),
	m_col_span (_colSpan),
	m_style_name (_style_name),
	m_is_header (false), m_text (_text)
{}

CHtmlTableCell::CHtmlTableCell (const QString& _tag, int _col_span, int _width, const QString& _tag_postfix):
	m_tag (_tag),
	m_tag_postfix (_tag_postfix),
	m_width (_width),
	m_col_span (_col_span),
	m_is_header (false)
{}

CHtmlTableCell::~CHtmlTableCell ()
{}

CHtmlGenerator::CHtmlGenerator (QTextStream& _stream) : m_stream (_stream)
{}

void
CHtmlGenerator::beginHTML (const QString& _css)
{
	m_stream << HTML4_HEADER.arg (_css);
}

void
CHtmlGenerator::endHTML ()
{
	m_stream << "</body></html>";
}

void
CHtmlGenerator::beginDiff (bool _inserted)
{
	if (_inserted)
		m_stream << "<ins>";
	else
		m_stream << "<del>";
}

void
CHtmlGenerator::endDiff (bool _inserted)
{
	if (_inserted)
		m_stream << "</ins>";
	else
		m_stream << "</del>";
}

void
CHtmlGenerator::beginTable (const QString& _class, TableStyle _style)
{
	m_stream << "<table";
	if (!_class.isEmpty ())
		m_stream << " class=\"" << _class << "\"";
	if (_style == _TableInserted)
		m_stream << " style=\"background:#e6ffe6;\"";
	else if (_style == _TableRemoved)
		m_stream << " style=\"background:#ffe6e6;\"";
	m_stream << ">";
}

void
CHtmlGenerator::endTable ()
{
	m_stream << "</table>";
}

void
CHtmlGenerator::beginRow ()
{
	m_stream << "<tr>";
}

void
CHtmlGenerator::endRow ()
{
	m_stream << "</tr>";
}

void
CHtmlGenerator::beginCell (const OAF::CHtmlTableCell& _cell)
{
	QString html_cell;

	//
	// Ячейка может быть рядовой, а может - заголовочной
	//
	if (_cell.m_is_header)
		m_stream << "<th";
	else
		m_stream << "<td";

	//
	// Задаем стиль ячейки, если он указан - иначе используем табличный
	//
	if (!_cell.m_style_name.isEmpty ())
		m_stream << " class=\"" << _cell.m_style_name << "\" ";
	else
		m_stream << " class=\"" << m_table_style_name << "\" ";

	//
	// Ширину ячейки можно задавать явно (высота будет регулироваться подписями вроде "ID:")
	//
	if (_cell.m_width > 0)
		m_stream << QString (" width=\"%1%\"").arg (_cell.m_width);

	//
	// Несколько столбцов таблицы можно объединять в один
	//
	if (_cell.m_col_span >= 2)
		m_stream << QString (" colspan=\"%1\"").arg (_cell.m_col_span);
	m_stream << ">";

	//
	// Добавляем "заголовок" ячейки (часть ее текста перед разделителем)
	//
	m_stream << _cell.m_tag << _cell.m_tag_postfix;
	//
	// Добавляем разделитель и текст, если они указаны
	//
	if (!_cell.m_text.isEmpty ())
		m_stream << _cell.m_text;

	//
	// Сохраняем данную ячейку для использования при закрытии тега
	//
	m_last_cell = _cell;
}

void
CHtmlGenerator::endCell ()
{
	if (m_last_cell.m_is_header)
		m_stream << " </th>";
	else
		m_stream << " </td>";
}

void
CHtmlGenerator::insertText (const QString& _text, bool _htmlize)
{
	//
	// Пробельные символы заменяем в любом случае - без этого текст корректно отображаться точно не будет
	//
	QString htmled_text = _text;
	htmled_text.replace ("\t"  , "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
	htmled_text.replace ("\r\n", "<br>");
	htmled_text.replace ("\n"  , "<br>");

	//
	// Заменяем запрещенные в HTML спец-символы на их аналоги (см. http://www.w3.org/TR/html4/charset.html#h-5.4)
	//
	if (_htmlize)
	{
		htmled_text.replace ("<" , "&lt");
		htmled_text.replace (">" , "&gt");
		htmled_text.replace ("&" , "&amp");
		htmled_text.replace ("\"", "&quot");
	}

	m_stream << htmled_text;
}

void
CHtmlGenerator::insertHtml (const QString& _text)
{
	m_stream << _text;
}

void
CHtmlGenerator::insertLineBreak ()
{
	m_stream << "<br/>";
}

void
CHtmlGenerator::beginPreformatted ()
{
	m_stream << "<pre>";
}

void
CHtmlGenerator::endPreformatted ()
{
	m_stream << "</pre>";
}

void
CHtmlGenerator::insertBold (const QString& _text)
{
	m_stream << "<b>";
	insertText (_text, false);
	m_stream << "</b>";
}

void
CHtmlGenerator::insertItalic (const QString& _text)
{
	m_stream << "<i>";
	insertText (_text, false);
	m_stream << "</i>";
}

void
CHtmlGenerator::insertSpan (const QString& _text)
{
	m_stream << "<span>";
	insertText (_text);
	m_stream << "</span>";
}

void
CHtmlGenerator::insertHeader (const QString& _header, int _level)
{
	Q_ASSERT ((_level >= 1) && (_level <= 6));

	//
	// Формируем тег заголовка в зависимости от заданного уровня
	//
	QString header_open_tag  = "<h"  + QString::number (_level) + ">";
	QString header_close_tag = "</h" + QString::number (_level) + ">";

	//
	// Добавляем заголовок в HTML
	//
	m_stream << header_open_tag;
	insertText (_header);
	m_stream << header_close_tag;
}

void
CHtmlGenerator::insertDiff (const QString& _diff_text, bool _inserted)
{
	if (_inserted)
		m_stream << "<ins>" << _diff_text << "</ins>";
	else
		m_stream << "<del>" << _diff_text << "</del>";
}
