/**
 * @file
 * @brief Реализация вспомогательных классов для работы с HTML-кодом
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/HelpersHTML.h>

static const QString HTML4_HEADER =
		"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
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

OAF::CHTMLTableCell::CHTMLTableCell (const QString& _text, int _span, int _width,
									 const QString& _prefix, const QString& _delim,
									 const QString& _style) :
	m_is_header (false),
	m_span (_span),
	m_width (_width),
	m_prefix (_prefix),
	m_delim (_delim),
	m_style (_style)
{
	m_text = _text;
}

OAF::CHTMLTableCell::CHTMLTableCell (int _span, int _width,
									 const QString& _prefix, const QString& _delim,
									 const QString& _style) :
	m_is_header (false),
	m_span (_span),
	m_width (_width),
	m_prefix (_prefix),
	m_delim (_delim),
	m_style (_style)
{}

OAF::CHTMLTableCell::~CHTMLTableCell ()
{}

OAF::CHTMLGenerator::CHTMLGenerator (const QString& _css, bool _header)
{
	setCSS (_css, QString::null, _header);
}

void
OAF::CHTMLGenerator::setCSS (const QString& _css, const QString& _style, bool _header)
{
	m_header        = _header;
	m_css           = _css;
	m_current_style = _style;

	clear ();
}

void
OAF::CHTMLGenerator::beginDiff (bool _inserted)
{
	if (_inserted)
		m_html += "<ins>";
	else
		m_html += "<del>";
}

void
OAF::CHTMLGenerator::endDiff (bool _inserted)
{
	if (_inserted)
		m_html += "</ins>";
	else
		m_html += "</del>";
}

void
OAF::CHTMLGenerator::beginTable (const QString& _style, int _mode)
{
	//
	// Сохряняем параметры форматирования таблицы для последующего использования
	//
	m_current_style = _style;
	m_current_mode  = _mode;

	//
	// Открываем таблицу
	//
	m_html += "<table";

	//
	// Задаем CSS-стиль таблицы, если он указан
	//
	if (!_style.isEmpty ())
		m_html += " class=\"" + m_current_style + "\"";

	//
	// Задаем вывод таблицы в качестве добавленного/удаленного объекта, если задано
	//
	switch (m_current_mode)
	{
		case MODE_INSERTED:
			m_html += " style=\"background:#e6ffe6;\"";
			break;

		case MODE_REMOVED:
			m_html += " style=\"background:#ffe6e6;\"";
			break;
	}

	m_html += ">\n";
}

void
OAF::CHTMLGenerator::endTable ()
{
	m_html += "</table>\n";
}

void
OAF::CHTMLGenerator::beginRow ()
{
	m_html += "<tr>\n";
}

void
OAF::CHTMLGenerator::endRow ()
{
	m_html += "</tr>\n";
}

void
OAF::CHTMLGenerator::beginCell (const OAF::CHTMLTableCell& _cell)
{
	QString html_cell;

	//
	// Ячейка может быть рядовой, а может - заголовочной
	//
	if (_cell.isHeader ())
		html_cell += "<th";
	else
		html_cell += "<td";

	//
	// Задаем стиль ячейки, если он указан - иначе используем табличный
	//
	if (!_cell.style ().isEmpty ())
		html_cell += " class=\"" + _cell.style () + "\" ";
	else
		html_cell += " class=\"" + m_current_style + "\" ";

	//
	// Ширину ячейки можно задавать явно (высота будет регулироваться подписями вроде "ID:")
	//
	if (_cell.width () > 0)
		html_cell += QString (" width=\"%1%\"").arg (_cell.width ());

	//
	// Несколько столбцов таблицы можно объединять в один
	//
	if (_cell.span () >= 2)
		html_cell += QString (" colspan=\"%1\"").arg (_cell.span ());
	html_cell += ">\n";

	//
	// Добавляем "заголовок" ячейки (часть ее текста перед разделителем)
	//
	html_cell += (_cell.prefix () + _cell.delim ());
	//
	// Добавляем разделитель и текст, если они указаны
	//
	if (!_cell.text ().isEmpty ())
		html_cell += _cell.text ();

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
OAF::CHTMLGenerator::endCell ()
{
	QString html_cell;

	if (m_last_cell.isHeader ())
		html_cell += " </th>\n";
	else
		html_cell += " </td>\n";

	//
	// Добавляем закрывающие теги ячейки в HTML-документ
	//
	m_html += html_cell;
}

void
OAF::CHTMLGenerator::insertText (const QString& _text, bool _htmlize)
{
	//
	// Пробельные символы заменяем в любом случае - без этого текст корректно отображаться точно не будет
	//
	QString text = _text;
	text.replace ("\t"  , "&nbsp;&nbsp;&nbsp;&nbsp;");
	text.replace ("\r\n", "<br>");
	text.replace ("\n"  , "<br>");

	//
	// Заменяем запрещенные в HTML спец-символы на их аналоги (см. http://www.w3.org/TR/html4/charset.html#h-5.4)
	//
	if (_htmlize)
	{
		text.replace ("<" , "&lt");
		text.replace (">" , "&gt");
		text.replace ("&" , "&amp");
		text.replace ("\"", "&quot");
	}

	m_html += text;
}

void
OAF::CHTMLGenerator::insertHTML (const QString& _text)
{
	m_html += _text;
}

void
OAF::CHTMLGenerator::insertBR ()
{
	m_html += "<br>";
}

void
OAF::CHTMLGenerator::beginPreformatted ()
{
	m_html += "<pre>";
}

void
OAF::CHTMLGenerator::endPreformatted ()
{
	m_html += "</pre>";
}

void
OAF::CHTMLGenerator::insertBold (const QString& _text)
{
	m_html += "<b>";
	insertText (_text, false);
	m_html += "</b>";
}

void
OAF::CHTMLGenerator::insertItalic (const QString& _text)
{
	m_html += "<i>";
	insertText (_text, false);
	m_html += "</i>";
}

void
OAF::CHTMLGenerator::insertSpan (const QString& _text)
{
	m_html += "<span>";
	insertText (_text);
	m_html += "</span>";
}

void
OAF::CHTMLGenerator::insertHeader (const QString& _text, int _level)
{
	Q_ASSERT ((_level >= 1) && (_level <= 6));

	m_html += QString ("<h%1>").arg (_level);
	insertText (_text);
	m_html += QString ("</h%1>\n").arg (_level);
}

void
OAF::CHTMLGenerator::insertDiff (const QString& _text, bool _inserted)
{
	if (_inserted)
		m_html += "<ins>" + _text + "</ins>";
	else
		m_html += "<del>" + _text + "</del>";
}

QString
OAF::CHTMLGenerator::result () const
{
	if (m_header)
		return m_html + "\n</body>\n</html>\n";
	return m_html;
}

void
OAF::CHTMLGenerator::clear ()
{
	m_html.clear ();
	if (m_header)
		m_html = HTML4_HEADER.arg (m_css);

	m_current_style = QString::null;
	m_current_mode  = MODE_DEFAULT;
	m_last_cell     = OAF::CHTMLTableCell ();
}
