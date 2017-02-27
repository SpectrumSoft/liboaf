/**
 * @file
 * @brief Реализация вспомогательных функций для работы с MIME-типами
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QtXml>

#include <OAF/OAF.h>
#include <OAF/Utils.h>
#include <OAF/HelpersMIME.h>

static bool
tokenIsOperator (const QString& _token)
{
	return (_token == "&") || (_token == "|");
}

static bool
tokenIsVariable (const QString& _token)
{
	bool ok = false;
	_token.toInt (&ok, 0);
	return ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

OAF::MimeTypeInfo::MagicMatch::MagicMatch (): type (DATA_UNKNOWN), offset (QPair <int, int> (0, 0))
{}

bool
OAF::MimeTypeInfo::MagicMatch::isValid () const
{
	return (type != DATA_UNKNOWN) && !value.pattern ().isEmpty ();
}

bool
OAF::MimeTypeInfo::MagicMatch::dataIsMatching (const QByteArray& _header_data) const
{
	Q_ASSERT (isValid ());
	bool magic_matched = false;

	//
	// Процедура сравнения данных зависит от их типа
	//
	switch (type)
	{
		case OAF::MimeTypeInfo::MagicMatch::DATA_STRING:
		{
			//
			// Сначала применяем маску к значению из файла
			//
			if (!mask.isEmpty ())
			{
				for (int offset_value = offset.first; offset_value <= offset.second; ++offset_value)
				{
					//
					// Вырезаем кусочек из заголовка файла по текущему смещению
					//
					QByteArray header_value = QByteArray (_header_data).mid (offset_value, mask.length ());
					//
					// Накладываем маску на него
					//
					for (int i = 0; i < mask.length (); ++i)
						header_value[i] = (uchar)header_value[i] & (uchar)mask[i];
					//
					// Сравниваем полученное magic-значение с указанными в MIME-типе
					//
					if (value.indexIn (header_value) != -1)
						magic_matched = true;
				}
			}
			else
			{
				//
				// Если же маска не задана, то проверяем все смещения указанного диапазона
				// (границы включаются) на присутствие магического значения
				//
				int value_offset = value.indexIn (_header_data, offset.first);
				if ((value_offset >= offset.first) && (value_offset <= offset.second))
					magic_matched = true;
			}

			break;
		}
		case OAF::MimeTypeInfo::MagicMatch::DATA_BYTE:
		{
			//
			// Получаем числовое значение "магического" байта и проверяем его на корректность
			//
			bool ok = false;
			int byte_value = value.pattern ().toInt (&ok, 0);
			if (ok && ((byte_value >= 0) && (byte_value <= UCHAR_MAX)))
			{
				if (mask.isEmpty ())
				{
					//
					// Проверяем значение найденного выше байта по всем указанным смещениям
					//
					for (int offset_value = offset.first; offset_value <= offset.second; ++offset_value)
					{
						//
						// Проверяем, чтобы смещение было в пределах заголовка
						//
						if ((offset_value >= 0) && (offset_value < _header_data.size ()))
						{
							if ((uchar)_header_data.at (offset_value) == (uchar)byte_value)
								magic_matched = true;
						}
					}
				}
				else
					// TODO: implement mask support this case; however it looks like unneeded
					qWarning ("Mask support is not implemented yet for BYTE magic data type");
			}
			else
				qWarning ("Magic value of type byte must be one byte long");

			break;
		}
		// FIXME: add support for the rest data types
		case OAF::MimeTypeInfo::MagicMatch::DATA_HOST16:
			//break;
		case OAF::MimeTypeInfo::MagicMatch::DATA_HOST32:
			//break;
		case OAF::MimeTypeInfo::MagicMatch::DATA_BIG16:
			//break;
		case OAF::MimeTypeInfo::MagicMatch::DATA_BIG32:
			//break;
		case OAF::MimeTypeInfo::MagicMatch::DATA_LITTLE16:
			//break;
		case OAF::MimeTypeInfo::MagicMatch::DATA_LITTLE32:
			//break;
		default:
			break;
	}

	return magic_matched;
}

OAF::MimeTypeInfo::MagicMatchExpr::MagicMatchExpr ()
{}

OAF::MimeTypeInfo::MagicMatchExpr::MagicMatchExpr (const QString& _polish_expr, const QList<MagicMatch>& _matches):
	polish_expr (_polish_expr),
	matches (_matches)
{}

bool
OAF::MimeTypeInfo::MagicMatchExpr::isValid () const
{
	return (!polish_expr.isEmpty () && !matches.isEmpty ());
}

bool
OAF::MimeTypeInfo::MagicMatchExpr::dataIsMatching (const QByteArray& _header_data) const
{
	QStack<QString> var_stack;
	//
	// Интерпретируем обратную польскую запись (состоящую из переменных/констант и операторов):
	// 1) идём по токенам выражения слева направо;
	// - если попалась переменная/константа - добавляем в стек;
	// - если попался оператор, то берем N нужных операндов из стека и помещаем туда же результат;
	// 2) забираем результат вычисления с вершины стека
	//
	QStringList expr_tokens = polish_expr.split (QRegExp ("\\s+"), QString::SkipEmptyParts);
	foreach (const QString& token, expr_tokens)
	{
		if (tokenIsVariable (token))
		{
			bool idx_ok = false;
			int var_idx = token.toInt (&idx_ok);
			Q_ASSERT (idx_ok);
			bool result = matches[--var_idx].dataIsMatching (_header_data);

			var_stack.push (result ? "true" : "false");
		}
		else if (tokenIsOperator (token))
		{
			//
			// TODO: у нас пока используются только логические операторы AND/OR;
			// вообще бы надо сделать статическое хранилище с описанием допустимых операторов
			//

			//
			// Добываем два операнда из стека: там могут быть либо числа (индексы match'ей),
			// либо константы true/false для уже вычисленных magic-проверок
			//
			// NOTE: для более простой проверки глазками "польского" выражения
			// индексы переменных-match'ей там начинаются с единицы
			//
			Q_ASSERT_X (var_stack.size () >= 2, Q_FUNC_INFO, "Reverse polish notation expression is incorrect");
			QString first_op = var_stack.pop ();
			QString second_op = var_stack.pop ();
			bool first_bool = (first_op == "true");
			bool second_bool = (second_op == "true");

			bool result = false;
			//
			// Применяем к ним соответствующий оператор
			//
			if (token == "&")
				result = first_bool && second_bool;
			else if (token == "|")
				result = first_bool || second_bool;
			else
				Q_ASSERT_X (0, Q_FUNC_INFO, "Unsupported operator type was detected");

			//
			// Кладём результат обратно в стек
			//
			var_stack.push (result ? "true" : "false");
		}
		else
			Q_ASSERT_X (0, Q_FUNC_INFO, "Unknown token type was detected");
	}

	Q_ASSERT (var_stack.size () == 1);
	return (var_stack.pop () == "true");
}

bool
OAF::MimeTypeInfo::isValid () const
{
	return (!mime_type.isEmpty ());
}

bool
OAF::MimeTypeInfo::hasMagicData () const
{
	return (!magic_values.empty ());
}

bool
OAF::MimeTypeInfo::hasMagicXmlData () const
{
	return (!root_xml.isEmpty () || !namespace_uri.isEmpty ());
}

QString
OAF::MimeTypeInfo::sysLocaleDesc () const
{
	QString sys_lang = OAF::CMimeDatabase::instance ().currentLang ();

	return descriptions.value (sys_lang);
}

QString
OAF::MimeTypeInfo::filterString () const
{
	QString filter_str;

	foreach (const Extension& ext, extensions)
	{
		filter_str += (ext.first + " ");
	}
	filter_str = filter_str.trimmed ();

	return filter_str;
}

/**
 * @brief Проверяет, что указанный массив байтов является числом в десятичной/шестнадцатеричной системе счисления
 * @param _as_hex Изначально рассматривать массив байтов только как шестнадцатеричное число
 */
static bool
checkMaskCorrectness (const QByteArray& _ba, bool _as_hex)
{
	if (_ba.isEmpty ())
		return false;

	int start_idx = 0;
	if ((_ba[0] == '0') && (_ba[1] == 'x'))	// hexadecimal
	{
		_as_hex = true;
		start_idx = 2;
	}
	else if (_ba[0] != '0')	// octal
	{
		qWarning ("Warning: decimal mask value is not currently supported");
		return false;
	}

	//
	// TODO: конвертировать в int блоками по qulonglong длиной для улучшения производительности
	//
	for (int i = start_idx; i < _ba.length (); ++i)
	{
		QChar c (_ba [i]);

		if (!(_as_hex && c.isLetterOrNumber ()) && !c.isDigit ())
			return false;
	}

	return true;
}

static int
convertToInt (const QString& _str, bool& _ok)
{
	int dec_num = _str.toInt (&_ok, 8);
	if (!_ok)
	{
		dec_num = _str.toInt (&_ok, 16);
		if (!_ok)
			dec_num = _str.toInt (&_ok, 10);
	}

	return dec_num;
}

/**
 * @brief Заменяет восьмеричные и шестнадцатеричные escape-последовательности на их десятичные значения
 */
static void
normalizeMagicValue (const QString& _pattern, QByteArray& _magic_value)
{
	QString magic_value_str (_magic_value);

	QRegExp rx (_pattern, Qt::CaseInsensitive);
	int pos = 0;
	while ((pos = rx.indexIn (magic_value_str)) != -1)
	{
		QString num_str = rx.cap (1);
		int matched_length = rx.matchedLength ();

		//
		// Сначала пробуем восьмеричную систему счисления, затем шестнадцатеричную и наконец десятичную
		//
		bool ok = false;
		int dec_num = convertToInt (num_str, ok);
		Q_ASSERT ((dec_num >= 0) && (dec_num <= UCHAR_MAX));
		if (ok)
		{
			magic_value_str.remove (pos, matched_length - 1);
			//
			// Записываем байт вместо обратного слэша и убираем оставшиеся символы
			//
			_magic_value[pos] = dec_num;
			_magic_value.remove (pos + 1, matched_length - 1);
		}
		else
			qWarning ("Invalid number was detected: %s", qPrintable (num_str));
	}
}

static OAF::MimeTypeInfo::MagicMatch::DataType
detectMagicValueType (const QString& _type_attr)
{
	OAF::MimeTypeInfo::MagicMatch::DataType data_type = OAF::MimeTypeInfo::MagicMatch::DATA_UNKNOWN;

	if (_type_attr.compare ("string", Qt::CaseInsensitive) == 0)
		data_type = OAF::MimeTypeInfo::MagicMatch::DATA_STRING;
	else if (_type_attr.compare ("byte", Qt::CaseInsensitive) == 0)
		data_type = OAF::MimeTypeInfo::MagicMatch::DATA_BYTE;
	else if (_type_attr.compare ("host16", Qt::CaseInsensitive) == 0)
		data_type = OAF::MimeTypeInfo::MagicMatch::DATA_HOST16;
	else if (_type_attr.compare ("host32", Qt::CaseInsensitive) == 0)
		data_type = OAF::MimeTypeInfo::MagicMatch::DATA_HOST32;
	else if (_type_attr.compare ("big16", Qt::CaseInsensitive) == 0)
		data_type = OAF::MimeTypeInfo::MagicMatch::DATA_BIG16;
	else if (_type_attr.compare ("big32", Qt::CaseInsensitive) == 0)
		data_type = OAF::MimeTypeInfo::MagicMatch::DATA_BIG32;
	else if (_type_attr.compare ("little16", Qt::CaseInsensitive) == 0)
		data_type = OAF::MimeTypeInfo::MagicMatch::DATA_LITTLE16;
	else if (_type_attr.compare ("little32", Qt::CaseInsensitive) == 0)
		data_type = OAF::MimeTypeInfo::MagicMatch::DATA_LITTLE32;
	else if (!_type_attr.isEmpty ())
		qWarning ("mime-type/magic/match/[type] has unsupported value: %s", qPrintable (_type_attr));
	else
		qWarning ("Required field mime-type/magic/match/[type] is empty");

	return data_type;
}

static QPair<int,int>
readOffsetValue (const QString& _offset_attr)
{
	QPair<int,int> offset (0, 0);
	bool ok = false;

	if (_offset_attr.contains (':'))
	{
		uint start_match = 0, end_match = 0;
		QStringList offset_list = _offset_attr.split (':', QString::SkipEmptyParts);
		if (offset_list.size () == 2)
		{
			start_match = offset_list[0].toUInt (&ok);
			if (!ok)
				qWarning ("mime-type/magic/match/offset value is NaN");
			end_match = offset_list[1].toUInt (&ok);
			if (!ok)
				qWarning ("mime-type/magic/match/[offset] value is NaN");

			offset.first = start_match;
			offset.second = end_match;
		}
	}
	else
	{
		uint offset_value = _offset_attr.toUInt (&ok);
		if (!ok)
			qWarning ("mime-type/magic/match/[offset] value is NaN");

		offset.first = offset_value;
		offset.second = offset_value;
	}

	return offset;
}

static uint
readPriorityValue (const QString& _priority_attr)
{
	uint priority = 50;

	if (!_priority_attr.isEmpty ())
	{
		bool ok = false;
		uint priority_attr_uint = _priority_attr.toUInt (&ok);
		if (ok && (priority_attr_uint <= 100))
			priority = priority_attr_uint;
	}

	return priority;
}

static void
readMagicMatch (QDomElement _match_element, QString& _expression, QList<OAF::MimeTypeInfo::MagicMatch>& _matches)
{
	OAF::MimeTypeInfo::MagicMatch match;

	if (_match_element.isNull ())
		return;

	//
	// Считываем тип данных в заголовке файла
	//
	match.type = detectMagicValueType (_match_element.attribute ("type"));

	//
	// Считываем смещение относительно начала файла, где находятся данные для проверки
	//
	match.offset = readOffsetValue (_match_element.attribute ("offset"));

	//
	// Считываем собственно магическое значение,
	// наличие которого в заголовке файла будет проверяться
	//
	// NOTE: здесь и далее предполагаем, что кодировка XML-файла - UTF-8
	//
	QByteArray magic_value = _match_element.attribute ("value").toUtf8 ();
	if (!magic_value.isEmpty ())
	{
		normalizeMagicValue ("\\\\x([\\dabcdef]+)", magic_value);
		normalizeMagicValue ("\\\\(\\d+)", magic_value);

		match.value.setPattern (magic_value);
	}
	else
		qWarning ("mime-type/magic/match/[value] is empty");

	//
	// Считываем маску, которая предварительно должна применяться
	// к считанному из файла значению перед сравнением с указанным магическим
	//
	// NOTE: Маска должна быть числом в десятичной или шестнадцатеричной
	// системе счисления (для типа string), возможно большого размера (больше 8 байт)
	//
	QByteArray magic_mask = _match_element.attribute ("mask").toUtf8 ();
	if (checkMaskCorrectness (magic_mask, match.type == OAF::MimeTypeInfo::MagicMatch::DATA_STRING))
		match.mask = magic_mask;

	//
	// Добавляем очередное магическое значение в список
	//
	Q_ASSERT (match.isValid ());
	_matches << match;
	_expression += QString::number (_matches.size ());

	//
	// NOTE: они могут быть любого уровня вложенности!
	// отношение родитель-дочерний считаем операцией AND, элементы на одном уровне связаны операцией OR
	//
	QDomElement submatch_element = _match_element.firstChildElement ("match");
	if (!submatch_element.isNull ())
	{
		_expression += " & ( ";

		while (!submatch_element.isNull ())
		{
			_expression += " ( ";
			readMagicMatch (submatch_element, _expression, _matches);

			submatch_element = submatch_element.nextSiblingElement ("match");
			if (!submatch_element.isNull ())
				_expression += " ) | ";
			else
				_expression += " )";
		}

		_expression += " ) ";
	}
}

/**
 * @brief Определяет приоритет указанной операции
 */
static int
operationPriority (const QString& _op)
{
	int priority = 0;

	if (_op == "&")
		priority = 3;
	else if (_op == "|")
		priority = 2;
	else if (_op == "(")
		priority = 1;
	else
		Q_ASSERT_X (0, Q_FUNC_INFO, "Unknown arithmetic operator");

	return priority;
}

/**
 * @brief Преобразует булево выражение в инфиксной нотации в обратную польскую запись,
 * удобную для последующей интерпретации
 */
static QString
convertToPolish (const QString& _bool_expr)
{
	QString polish_expr;
	QStack<QString> op_stack;
	QStringList expr_tokens = _bool_expr.split (QRegExp ("\\s+"), QString::SkipEmptyParts);

	//
	// По всем токенам выражения
	//
	// NOTE: стирание пробелов перед и после операндами нарушат корректную работу функции!
	//
	foreach (const QString& token, expr_tokens)
	{
		//
		// Допустимы три вида токенов: переменная или константа,
		// открывающая или закрывающая скобка, логический оператор И/ИЛИ:
		// - первые просто добавляем сразу в выходное выражение;
		// - открывающую скобку добавляем в стек;
		// - для закрывающей скобки выталкиваем в стек все операторы вплоть до ближайшей открывающей скобки;
		// - для оператора выталкиваем в выражение из стека все операции с меньшем приоритетом, чем у текущего
		//
		if (tokenIsVariable (token))
			polish_expr += QString (token) + " ";
		else if (token == "(")
			op_stack.push (token);
		else if (token == ")")
		{
			Q_ASSERT (!op_stack.isEmpty ());

			QString op;
			while ((op = op_stack.pop ()) != "(")
				polish_expr += op + " ";
		}
		else if (tokenIsOperator (token))
		{
			while (!op_stack.isEmpty ()
				   && (operationPriority (token) >= operationPriority (op_stack.top ()))
				   && (op_stack.top () != "("))
			{
				polish_expr += op_stack.pop () + " ";
			}

			op_stack.push (token);
		}
		else
			Q_ASSERT_X (0, Q_FUNC_INFO, "Unknown token was detected");
	}

	while (!op_stack.isEmpty ())
		polish_expr += op_stack.pop () + " ";

	return polish_expr.trimmed ();
}

QList<OAF::MimeTypeInfo>
OAF::CMimeDatabase::readMimesFromXML (const QString& _path)
{
	QList<MimeTypeInfo> mime_types;

	QFile file (_path);
	Q_ASSERT (file.exists ());
	if (!file.open (QIODevice::ReadOnly | QIODevice::Text))
	{
		qWarning ("Could not open file: %s", qPrintable (_path));
		return QList<MimeTypeInfo> ();
	}

	//
	// Открываем XML-документ и парсим его
	// FIXME: переписать на QXmlStreamReader, который однако глючит на MIME-файлах (readNextStartElement возвращает false);
	//        если это удастся побороть, то нужно еще убрать модуль xml из liboaf-std.pro и всех его клиентов
	//
	QDomDocument doc;
	if (!doc.setContent (&file))
	{
		qWarning ("QDomDocument::setContent has failed on file %s", qPrintable (_path));
		file.close ();
		return QList<MimeTypeInfo> ();
	}

	QDomElement root_item = doc.documentElement ();
	QDomNode xml_node = root_item.firstChild ();
	while (!xml_node.isNull ())
	{
		QDomElement xml_item = xml_node.toElement ();
		if (!xml_item.isNull () && (xml_item.tagName ().compare ("mime-type", Qt::CaseInsensitive) == 0))
		{
			MimeTypeInfo mime_type_info;

			//
			// Получаем атрибут type, где хранится название MIME-типа
			//
			if (xml_item.hasAttribute ("type"))
			{
				QString mime_type = xml_item.attribute ("type");
				if (!mime_type.isEmpty ())
					mime_type_info.mime_type = mime_type;
			}
			else
				qWarning ("mime-type element must have type attribute");

			QDomNode mime_node = xml_item.firstChild ();
			while (!mime_node.isNull ())
			{
				QDomElement mime_item = mime_node.toElement ();
				if (mime_item.tagName ().compare ("alias", Qt::CaseInsensitive) == 0)
				{
					if (mime_item.hasAttribute ("type"))
					{
						QString alias = mime_item.attribute ("type");
						if (!alias.isEmpty ())
							mime_type_info.alias_mimes.append (alias);
					}
					else
						qWarning ("mime-type/alias element must have type attribute");
				}
				else if (mime_item.tagName ().compare ("sub-class-of", Qt::CaseInsensitive) == 0)
				{
					if (mime_item.hasAttribute ("type"))
					{
						QString parent_mime = mime_item.attribute ("type");
						if (!parent_mime.isEmpty ())
							mime_type_info.parent_mimes.append (parent_mime);
					}
					else
						qWarning ("mime-type/sub-class-of element must have type attribute");
				}
				else if (mime_item.tagName ().compare ("glob", Qt::CaseInsensitive) == 0)
				{
					MimeTypeInfo::Extension ext (QString (), false);

					if (mime_item.hasAttribute ("pattern"))
					{
						QString pattern = mime_item.attribute ("pattern");
						ext.first = pattern;
					}
					else
						qWarning ("mime-type/glob element must have pattern attribute");

					if (mime_item.hasAttribute ("case-sensitive"))
					{
						QString cs = mime_item.attribute ("case-sensitive");
						ext.second = (cs.compare ("true", Qt::CaseInsensitive) == 0);
					}

					mime_type_info.extensions.append (ext);
				}
				else if (mime_item.tagName ().compare ("magic", Qt::CaseInsensitive) == 0)
				{
					// Получаем значение приоритета данного магического значения (в диапазоне [50; 100]);
					// чем больше данное значение, тем более специфический MIME-тип можно обнаружить таким magic'ом
					//
					uint priority = readPriorityValue (mime_item.attribute ("priority"));

					//
					// Считываем magic-данные для определения принадлежности файла к данному MIME-типу
					//
					// NOTE: элементы match могут быть вложенными друг в друга без ограничений на уровень:
					// таким образом задаётся логической порядок вычисления;
					// "принадлежит к" значит AND, "соседствует с" значит OR
					//
					QList<MimeTypeInfo::MagicMatch> magic_values;
					QString expr;
					QDomElement match_element = mime_item.firstChildElement ("match");
					while (!match_element.isNull ())
					{
						expr += " ( ";
						readMagicMatch (match_element, expr, magic_values);

						match_element = match_element.nextSiblingElement ("match");

						if (!match_element.isNull ())
							expr += " ) | ";
						else
							expr += " ) ";
					}

					//
					// Запоминаем порядок вычисления magic'ов в булевом выражении в обратной польской нотации;
					// т.к. в словаре в качестве функции сравнения указан оператор ">",
					// то magic-описания в словаре окажутся отсортированы по убыванию приоритетов
					//
					MimeTypeInfo::MagicMatchExpr magic_match_expr (convertToPolish (expr) + " ", magic_values);
					mime_type_info.magic_values[priority] << magic_match_expr;
				}
				else if (mime_item.tagName ().compare ("root-XML", Qt::CaseInsensitive) == 0)
				{
					if (mime_item.hasAttribute ("namespaceURI"))
					{
						QString namespaceURI = mime_item.attribute ("namespaceURI");
						if (!namespaceURI.isEmpty ())
							mime_type_info.namespace_uri = namespaceURI;
					}

					if (mime_item.hasAttribute ("localName"))
					{
						QString root_name = mime_item.attribute ("localName");
						if (!root_name.isEmpty ())
							mime_type_info.root_xml = root_name;
					}
				}
				else if (mime_item.tagName ().compare ("comment", Qt::CaseInsensitive) == 0)
				{
					QString lang = "en";
					if (mime_item.hasAttribute ("xml:lang"))
					{
						lang = mime_item.attribute ("xml:lang");
						if (lang.isEmpty ())
							lang = "en";
					}

					QString comment = mime_item.text ();
					if (!comment.isEmpty ())
						mime_type_info.descriptions.insert (lang, comment);
				}
				else if (mime_item.tagName ().compare ("acronym", Qt::CaseInsensitive) == 0)
				{
					if (mime_item.hasAttribute ("acronym"))
					{
						QString acronym = mime_item.attribute ("acronym");
						if (!acronym.isEmpty ())
							mime_type_info.acronym = acronym;
					}
				}
				else if (mime_item.tagName ().compare ("expanded-acronym", Qt::CaseInsensitive) == 0)
				{
					QString lang = "en";
					if (mime_item.hasAttribute ("xml:lang"))
					{
						lang = mime_item.attribute ("xml:lang");
						if (lang.isEmpty ())
							lang = "en";
					}

					QString expanded_acronym = mime_item.text ();
					if (!expanded_acronym.isEmpty ())
						mime_type_info.expanded_acronym.insert (lang, expanded_acronym);
				}
				else if (mime_item.tagName ().compare ("icon", Qt::CaseInsensitive) == 0)
				{
					if (mime_item.hasAttribute ("name"))
					{
						QString icon_name = mime_item.attribute ("name");
						if (!icon_name.isEmpty ())
							mime_type_info.icon = icon_name;
					}
				}
				else if (mime_item.tagName ().compare ("generic-icon", Qt::CaseInsensitive) == 0)
				{
					if (mime_item.hasAttribute ("name"))
					{
						QString icon_name = mime_item.attribute ("name");
						if (!icon_name.isEmpty ())
							mime_type_info.generic_icon = icon_name;
					}
				}

				mime_node = mime_node.nextSibling ();
			}

			//
			// Заполняем техническое название MIME-типа
			//
			mime_type_info.name = mime_type_info.mime_type;
			mime_type_info.name.replace ("/", "_");
			mime_type_info.name.replace ("-", "_");

			//
			// NOTE: Пока расширением по умолчанию выбираем первое в списке, но это
			//       легко изменить
			//
			if (!mime_type_info.extensions.isEmpty ())
			{
				mime_type_info.def_extension = mime_type_info.extensions.first ().first;
				mime_type_info.def_extension.remove ("*.");
			}

			//
			// Добавляем очередной MIME-тип в коллекцию
			//
			mime_types.append (mime_type_info);
		}

		//
		// Переходим к следующему узлу XML
		//
		xml_node = xml_node.nextSibling ();
	}

	file.close ();

	return mime_types;
}

void
OAF::CMimeDatabase::loadMimesFromDir (const QDir& _d)
{
	//
	// Получаем список XML-файлов в данной директории
	//
	QDir packages (_d);
	packages.setFilter (QDir::Files | QDir::Readable | QDir::NoDotAndDotDot | QDir::NoSymLinks);
	QFileInfoList file_list = packages.entryInfoList ();

	foreach (const QFileInfo& fi, file_list)
	{
		//
		// Проверяем, что очередной файл имеет формат XML
		//
		// TODO: т.к. БД MIME-типов ещё не загружена, то проверить тип можно лишь по расширению;
		//       возможно, критерии распознавания XML-файлов нужно захардкодить прямо здесь?
		//
		if (fi.suffix ().compare ("xml", Qt::CaseInsensitive) == 0)
		{
			//
			// Загружаем очередной файл с описанием MIME-типов
			//
			QList<MimeTypeInfo> mime_types = readMimesFromXML (fi.filePath ());

			//
			// Для каждого описания
			//
			foreach (const MimeTypeInfo& mime_type_info, mime_types)
			{
				//
				// Если оно валидно
				//
				if (mime_type_info.isValid ())
				{
					//
					// Повторные определения MIME-типов просто пропускаем
					//
					if (mimeExists (mime_type_info.mime_type))
						continue;

					//
					// Добавляем очередной MIME-тип в нашу "базу данных"
					//
					m_mimes.append (mime_type_info);
				}
			}
		}
	}
}

void
OAF::CMimeDatabase::detectCurrentLang ()
{
	//
	// Определяем язык системной локали в короткой форме (ru, de, en и т.д.)
	//
	QLocale loc = QLocale::system ();
	QLocale::Language lang_enum = loc.language ();
	QString lang_short = loc.languageToString (lang_enum);
	if (lang_short.length () > 2)
		lang_short = lang_short.mid (0, 2).toLower ();

	m_lang = lang_short;
}

void
OAF::CMimeDatabase::parentMimes (const MimeTypeInfo& _mime_type_info, QStringList& _parent_mimes)
{
	//
	// Проходим по всем родительским MIME-типам указанного типа
	//
	foreach (const QString& p, _mime_type_info.parent_mimes)
	{
		//
		// Получаем описание очередного родительского MIME-типа
		//
		MimeTypeInfo parent = mimeInfo (p);
		if (parent.isValid ())
		{
			//
			// Добавляем MIME-тип если он не был добавлен ранее
			//
			if (!_parent_mimes.contains (parent.mime_type, Qt::CaseInsensitive))
				_parent_mimes.append (parent.mime_type);

			//
			// Если для родительского типа тоже определены родительские типы,
			// то добавим и их
			//
			if (!parent.parent_mimes.isEmpty ())
				parentMimes (parent, _parent_mimes);
		}
	}
}

static int
appendOAFDirs (QList<QDir>& _out)
{
	//
	// Если определена переменная окружения OAFMIMES
	//
	QProcessEnvironment pe = QProcessEnvironment::systemEnvironment ();
	if (pe.contains ("OAFMIMES"))
	{
		//
		// Получаем значение переменной и преобразуем ее в список директорий
		//
		QString oaf_mimes = pe.value ("OAFMIMES");

		//
		// Разбиваем значение на части по ";"
		//
		QStringList package_dirs = oaf_mimes.split (';', QString::SkipEmptyParts);

		//
		// Проверяем существование каждой директории, и при необходимости добавляем разделитель в конец
		//
		foreach (const QString& i, package_dirs)
		{
			QDir d (i);

			if (!d.exists ())
			{
				qWarning ("MIME directory from OAFMIMES %s does not exists", qPrintable (d.path ()));
				continue;
			}

			_out.append (d);
		}
	}

	//
	// Добавляем директорию по умолчанию
	//
	QDir d (OAF::defaultDataPath () + QDir::separator () + "mime" + QDir::separator () + "packages");
	if (d.exists ())
		_out.append (d);

	return _out.count ();
}

static int
appendUNIXDirs (QList<QDir>& _out)
{
	//
	// Если определена переменная окружения XDG_DATA_HOME
	//
	QProcessEnvironment pe = QProcessEnvironment::systemEnvironment ();
	if (pe.contains ("XDG_DATA_HOME"))
	{
		QDir d (pe.value ("XDG_DATA_HOME") + QDir::separator () + "mime" + QDir::separator () + "packages");
		if (d.exists ())
			_out.append (d);
	}

	//
	// Если определена переменная среды XDG_DATA_DIRS
	//
	if (pe.contains ("XDG_DATA_DIRS"))
	{
		//
		// Получаем значение переменной и преобразуем ее в список директорий
		//
		QString xdg_data_dirs = pe.value ("XDG_DATA_DIRS");

		//
		// Разбиваем значение на части по :
		//
		QStringList package_dirs = xdg_data_dirs.split (':', QString::SkipEmptyParts);

		//
		// Проверяем существование каждой директории из указанных в переменной среды
		//
		foreach (const QString& i, package_dirs)
		{
			QDir d (i);
			if (!d.exists ())
			{
				qWarning ("MIME directory from XDG_DATA_DIRS %s does not exists", qPrintable (d.path ()));
				continue;
			}

			//
			// Теперь проверяем существование подкаталога mime/packages;
			// если всё в порядке, добавляем данную директорию в список путей к базам MIME-типов
			//
			QString dir_path = i;
			if (!dir_path.endsWith (QDir::separator ()))
				dir_path += QDir::separator ();
			d.setPath (dir_path + "mime" + QDir::separator () + "packages");
			if (d.exists ())
				_out.append (d);
		}
	}

	//
	// Добавляем несколько директорий по умолчанию
	//
	QDir d1 ("/usr/share/mime/packages");
	if (d1.exists ())
		_out.append (d1);

	QDir d2 ("/usr/local/share/mime/packages");
	if (d2.exists ())
		_out.append (d2);

	return _out.count ();
}

OAF::CMimeDatabase::CMimeDatabase ()
{
	//
	// Список директорий, в которых содержатся описания MIME-типов
	//
	QList<QDir> dirs;
	appendOAFDirs (dirs);
	appendUNIXDirs (dirs);
	//
	// Проверяем, что хотя бы одна директория MIME-типами была обнаружена;
	// если же нет, то ругаемся в консоль и выходим - без базовых MIME-типов работа невозможна
	//
	if (dirs.isEmpty ())
	{
		qWarning ("No MIME database directory was specified, could not continue execution");
		qWarning ("Please check out values of OAFMIMES environment variable:");
		qWarning ("It should contain at least one path to directory with XML-files, each of those is describing MIME type");

		exit (EXIT_FAILURE);
	}

	//
	// Считываем описания MIME-типов
	//
	foreach (const QDir& d, dirs)
		loadMimesFromDir (d);

	//
	// Определяем системную локаль и язык
	//
	detectCurrentLang ();
}

OAF::CMimeDatabase&
OAF::CMimeDatabase::instance ()
{
	static OAF::CMimeDatabase inst;
	return inst;
}

QString
OAF::CMimeDatabase::currentLang () const
{
	return m_lang;
}

/**
 * @brief Проверяет, удовлетворяет ли указанный файл указанным в описании MIME-типа "магическим" данным
 * @param _mime Описание MIME-типа, необходимое для определения принадлежности файлов к нему
 * @param _size Размер файла в байтах
 * @param _d Устройство ввода/вывода, связанное с интересующим файлом
 * @return true, если файл пуст или содержит нужные magic-данные в заголовке, и false в противном случае
 */
static bool
checkMimeMagic (const OAF::MimeTypeInfo& _mime_type, qint64 _size, QIODevice* _d)
{
	Q_ASSERT (_mime_type.isValid ());
	Q_ASSERT (_mime_type.hasMagicData ());

	//
	// Если размер потока данных меньше или равен нулю,
	// или не задано устройство ввода/вывода для загрузки данных
	//
	// NOTE: Проверка на <= 0 необходима для того, чтобы не проверять пустые или отсутствующие
	// файлы данных. Такие файлы автоматически считаются нужного типа по расширению, так
	// как в них можно записать любые данные
	//
	if ((_size <= 0) || !_d)
		return true;

	//
	// Считать заголовок файла нужно только раз, так как ввод/вывод - штука весьма небыстрая;
	// для этого пройдёмся по описаниям magic-данных и определим максимальный размер требуемых данных,
	// равный максимальному смещению + размер максимального magic-значения
	//
	// TODO: это значение надо вычислить сразу после загрузки базы и сохранить для каждого MIME-типа,
	// вычислять каждый раз заново излишне
	//
	int max_data_size = 0;
	OAF::MimeTypeInfo::MagicExpressions::const_iterator iExprPair = _mime_type.magic_values.begin ();
	for ( ; iExprPair != _mime_type.magic_values.end (); ++iExprPair)
	{
		QList<OAF::MimeTypeInfo::MagicMatchExpr>::const_iterator iExpr = iExprPair->second.begin ();
		for ( ; iExpr != iExprPair->second.end (); ++iExpr)
		{
			Q_ASSERT (iExpr->isValid ());

			QList<OAF::MimeTypeInfo::MagicMatch>::const_iterator iMatch = iExpr->matches.begin ();
			for ( ; iMatch != iExpr->matches.end (); ++iMatch)
			{
				int offset = qMax (iMatch->offset.first, iMatch->offset.second);
				int value_length = iMatch->value.pattern ().length ();
				if (offset + value_length > max_data_size)
					max_data_size = offset + value_length;
			}
		}
	}
	//
	// Мы не сможем считать из устройства больше данных, чем там есть
	//
	max_data_size = qMin (max_data_size, (int)_size);

	//
	// Открываем устройство для чтения (если оно еще не открыто для чтения)
	//
	char* header_raw_data = new char [max_data_size];
	memset (header_raw_data, 0, max_data_size);
	if ((_d->isOpen () && _d->isReadable ()) || _d->open (QIODevice::ReadOnly))
	{
		//
		// Считываем данные в количестве, найденном выше
		//
		if (_d->read (header_raw_data, max_data_size) <= 0)
		{
			qWarning ("Unable to read data from device for the MIME type detection");
			return false;
		}
		//
		// Больше устройства нам не нужно, можно закрывать
		//
		_d->close ();
	}
	else
	{
		qWarning ("Could not open specified device for the MIME type detection");
		return false;
	}

	QByteArray header_data (header_raw_data, max_data_size);
	delete[] header_raw_data;
	bool matched = false;

	//
	// Выполняем сравнение magic-данных из MIME-типа с данными заголовка файла, по убыванию приоритета;
	// порядок сравнения описывается с помощью булевского выражения в обратной польской записи
	//
	iExprPair = _mime_type.magic_values.begin ();
	for ( ; iExprPair != _mime_type.magic_values.end (); ++iExprPair)
	{
		QList<OAF::MimeTypeInfo::MagicMatchExpr>::const_iterator iExpr = iExprPair->second.begin ();
		for ( ; iExpr != iExprPair->second.end (); ++iExpr)
		{
			if (iExpr->dataIsMatching (header_data))
			{
				matched = true;
				break;
			}
		}
	}

	return matched;
}

/**
 * @brief Проверяет, совпадает ли тег и пространство имён корневого элемента XML-файла с указанным в MIME-типе
 * @param _mime_type Описание MIME-типа, необходимое для определения принадлежности файлов к нему
 * @param _root_item Корневой элемент XML-файла
 * @return true, если файл пуст или содержит нужные magic-данные в заголовке, и false в противном случае
 */
static bool
checkXmlMagic (const OAF::MimeTypeInfo& _mime_type, const QDomElement& _root_item)
{
	Q_ASSERT (_mime_type.hasMagicXmlData ());

	bool matched = true;
	//
	// Проверяем только непустые XML-файлы, для которых указанное устройство связано с корректным файлом;
	// пустые XML считаем проходящими проверку
	//
	if (!_root_item.isNull ())
	{
		QString namespace_uri = _root_item.namespaceURI ();
		QString root_tag = _root_item.tagName ();

		if (!_mime_type.namespace_uri.isEmpty ())
		{
			if (namespace_uri.compare (_mime_type.namespace_uri, Qt::CaseInsensitive) != 0)
				matched = false;
		}
		if (!_mime_type.root_xml.isEmpty ())
		{
			if (root_tag.compare (_mime_type.root_xml, Qt::CaseInsensitive) != 0)
				matched = false;
		}
		if (root_tag.isEmpty ())
			matched = false;
	}
	else
		matched = false;

	return matched;
}

QStringList
OAF::CMimeDatabase::lookupMimeTypes (const QString& _ext, qint64 _size, QIODevice* _d)
{
	//
	// Пытаемся загрузить указанный файл как XML
	//
	// NOTE: второй параметр в setContent (равный здесь true) отвечает за включение обработки пространств имён XML-файла
	//
	QDomDocument doc;
	QDomElement root_item;
	if (_d->open (QIODevice::ReadOnly))
	{
		if (doc.setContent (_d->readAll (), true))
		{
			root_item = doc.documentElement ();
			Q_ASSERT (!root_item.isNull ());
		}

		_d->close ();
	}

	QStringList res;

	//
	// Собираем подходящие MIME-типы в список по таблице: сначала проверяем только по расширениям, так как это очень быстро
	//
	foreach (const MimeTypeInfo& mime_type_info, m_mimes)
	{
		foreach (const MimeTypeInfo::Extension& extension, mime_type_info.extensions)
		{
			if (extension.first.compare ("*." + _ext, extension.second ? Qt::CaseSensitive : Qt::CaseInsensitive) == 0)
			{
				//
				// Добавляем найденный MIME-тип в коллекцию
				//
				if (!res.contains (mime_type_info.mime_type, Qt::CaseInsensitive))
					res.append (mime_type_info.mime_type);

				//
				// Добавляем все синонимы MIME-типа (alias)
				//
				foreach (const QString& m, mime_type_info.alias_mimes)
				{
					if (!res.contains (m, Qt::CaseInsensitive))
						res.append (m);
				}

				//
				// Добавляем все родительские типы
				//
				parentMimes (mime_type_info, res);
			}
		}
	}

	if (res.isEmpty ())
	{
		//
		// Если же расширение ничем не помогло, то придется считывать заголовок файла
		// и проверять эти данные на соответствие указанным в MIME-типах (т.н. "магические" данные);
		// также для XML-файлов проверяем название корневого тега и его пространства имён
		//
		// NOTE: данный способ может давать кучу совершенно неподходящих MIME-типов,
		// т.к. у некоторых из них могут быть указаны очень мягкие условия (только знак процента в начале файла и т.п.)
		//
		foreach (const MimeTypeInfo& mime_type_info, m_mimes)
		{
			bool magic_match = (mime_type_info.hasMagicData () && checkMimeMagic (mime_type_info, _size, _d));
			bool xml_magic_match = (mime_type_info.hasMagicXmlData () && checkXmlMagic (mime_type_info, root_item));

			if (magic_match || xml_magic_match)
			{
				//
				// Добавляем найденный MIME-тип в коллекцию
				//
				// NOTE: обнаруженный с помощью "магии" MIME-тип является более специализированным,
				// поэтому должен добавляться в начало списка типов
				//
				if (!res.contains (mime_type_info.mime_type, Qt::CaseInsensitive))
					res.prepend (mime_type_info.mime_type);

				//
				// Добавляем все синонимы MIME-типа (alias)
				//
				foreach (const QString& m, mime_type_info.alias_mimes)
				{
					if (!res.contains (m, Qt::CaseInsensitive))
						res.append (m);
				}

				//
				// Добавляем все родительские типы
				//
				parentMimes (mime_type_info, res);
			}
		}
	}

	//
	// Любой файл в самом общем случае является потоком байт
	//
	res.push_back ("application/octet-stream");

	//
	// Возвращаем собранные MIME-типы
	//
	return res;
}

QStringList
OAF::CMimeDatabase::mimeSuccessors (const QString& _mime_type)
{
	QStringList mimes;

	//
	// NOTE: alias'ы сюда пока не добавляем, т.к. зачем показывать в фильтре QFileDialog несколько одинаковых фильтров
	//

	foreach (const MimeTypeInfo& mime_type_info, m_mimes)
	{
		//
		// Проверяем MIME-тип
		//
		if (mime_type_info.mime_type.compare (_mime_type, Qt::CaseInsensitive) == 0)
		{
			if (!mimes.contains (mime_type_info.mime_type, Qt::CaseInsensitive))
				mimes.append (mime_type_info.mime_type);
		}

		//
		// Иначе проверяем псевдонимы
		//
		else if (mime_type_info.alias_mimes.contains (_mime_type, Qt::CaseInsensitive))
		{
			if (!mimes.contains (mime_type_info.mime_type, Qt::CaseInsensitive))
				mimes.append (mime_type_info.mime_type);
		}

		//
		// Иначе проверяем прямых и косвенных родителей
		//
		else
		{
			QStringList parents;
			parentMimes (mime_type_info, parents);
			if (parents.contains (_mime_type, Qt::CaseInsensitive))
			{
				if (!mimes.contains (mime_type_info.mime_type, Qt::CaseInsensitive))
					mimes.append (mime_type_info.mime_type);
			}
		}
	}

	return mimes;
}

OAF::MimeTypeInfo
OAF::CMimeDatabase::mimeInfo (const QString& _mime_type)
{
	foreach (const MimeTypeInfo& mime_type_info, m_mimes)
	{
		if (mime_type_info.mime_type.compare (_mime_type, Qt::CaseInsensitive) == 0)
			return mime_type_info;

		if (mime_type_info.alias_mimes.contains (_mime_type, Qt::CaseInsensitive))
			return mime_type_info;
	}

	return MimeTypeInfo ();
}

bool
OAF::CMimeDatabase::mimeExists (const QString& _mime_type)
{
	return mimeInfo (_mime_type).isValid ();
}

QString
OAF::CMimeDatabase::filterFromMime (const QString& _mime_type)
{
	QString filter;

	if (mimeExists (_mime_type))
	{
		MimeTypeInfo mime_type_info = mimeInfo (_mime_type);

		filter = mime_type_info.descriptions.value (m_lang) + " (" + mime_type_info.filterString () + ")";
	}

	return filter;
}

QString
OAF::CMimeDatabase::filterFromMime (const QString& _desc, const QStringList& _mime_types)
{
	QString filter = _desc + " (";

	bool found = false;
	foreach (const QString& m, _mime_types)
	{
		if (mimeExists (m))
		{
			MimeTypeInfo mime_type_info = mimeInfo (m);
			filter += mime_type_info.filterString () + " ";
			found = true;
		}
	}

	filter = filter.trimmed ();
	filter += ")";

	return (found ? filter : QString::null);
}

QString
OAF::CMimeDatabase::filterFromMime (const QString& _desc, const QString& _mime_type)
{
	QString filter = _desc + " (";

	if (mimeExists (_mime_type))
	{
		MimeTypeInfo mime_type_info = mimeInfo (_mime_type);
		filter += mime_type_info.filterString () + " ";
	}

	filter = filter.trimmed ();
	filter += ")";

	return filter;
}

QStringList
OAF::CMimeDatabase::filterFromMime (const QStringList& _mime_types, bool _all_supported, const QStringList& _all_supported_mimes)
{
	QStringList filters;

	//
	// Первым добавляем пункт фильтра "Все поддерживаемые файлы", если требуется
	//
	if (_all_supported)
		filters.append (filterFromMime (QCoreApplication::translate ("OAF::MimeTypeInfo", "All supported files"), _all_supported_mimes));

	foreach (const QString& m, _mime_types)
	{
		QString filter = filterFromMime (m);
		if (!filter.isEmpty ())
			filters.append (filter);
	}

	return filters;
}
