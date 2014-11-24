/**
 * @file
 * @brief Реализация вспомогательных функций для экспорта/импорта
 * @author Novikov Dmitriy <novikovdimka@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/ExchangeHelpers.h>

QString
OAF::exportToString (OAF::IExportable* _eo, const QString& _mime_type, OAF::IPropertyBag* _options, const QString& _efp)
{
	return exportToString (_eo, QStringList () << _mime_type, _options, _efp);
}

QString
OAF::exportToString (OAF::IExportable* _eo, const QStringList& _mime_types, OAF::IPropertyBag* _options, const QString& _efp)
{
	QByteArray export_data;
	QDataStream export_data_stream (&export_data, QIODevice::WriteOnly);
	_eo->exportTo (export_data_stream, _mime_types, _options, _efp);
	return QString::fromUtf8 (export_data);
}

void
OAF::importFromString (OAF::IImportable* _io, const QString& _data_to_import, const QString& _mime_type)
{
	importFromString (_io, _data_to_import, QStringList () << _mime_type);
}

void
OAF::importFromString (OAF::IImportable* _io, const QString& _data_to_import, const QStringList& _mime_types)
{
	QByteArray import_data = _data_to_import.toUtf8 ();
	QDataStream import_data_stream (&import_data, QIODevice::ReadOnly);

	_io->importFrom (import_data_stream, _mime_types);
}

static int NREQS=0;
QDataStream&
OAF::latexRawCommand (QDataStream& _os, const QString& _raw_cmd)
{
    QString tc = QString("\\typeout{-------------------------------------------%1}\n").arg(++NREQS);
    QByteArray raw_cmd = (tc +_raw_cmd).toUtf8 ();

    _os.writeRawData (raw_cmd.constData (), raw_cmd.size ());
    _os.writeRawData ("\n", 1);

    return _os;
}

QDataStream&
OAF::latexCommand (QDataStream& _os, const QString& _cmd, const QString& _val, const QString& _format)
{
	const QString data_to_write = QString (_format).replace ("%cmd", _cmd).replace ("%val1", _val);

	return latexRawCommand (_os, data_to_write);
}

QDataStream&
OAF::latexCommand (QDataStream& _os, const QString& _cmd, const QString& _val1, const QString& _val2, const QString& _format)
{
	const QString data_to_write = QString (_format).replace ("%cmd", _cmd).replace ("%val1", _val1).replace ("%val2", _val2);

	return latexRawCommand (_os, data_to_write);
}

QDataStream&
OAF::latexCommand (QDataStream& _os, const QString& _cmd, const QStringList& _values, const QString& _format)
{
	foreach (const QString& value, _values)
		latexCommand (_os, _cmd, value, _format);

	return _os;
}

QDataStream&
OAF::latexCommand (QDataStream& _os, const QString& _cmd, OAF::IExportable* _val, const QString &_format, const QString& _mime_type, OAF::IPropertyBag* _o, const QString& _efp)
{
	//
	// Т.к. при экспорте важен путь по которому запущен датастрим, использование exportToString не уместно
	//
	QStringList data_to_write_parts = QString (_format).replace ("%cmd", _cmd).split ("%val1");

	//
	// Записываем часть до значения документа
	//
	latexRawCommand (_os, data_to_write_parts.first ());

	//
	// Записываем сам документ
	//
	_val->exportTo (_os, QStringList () << _mime_type, _o, _efp);

	//
	// Записываем часть после значения документа
	//
	latexRawCommand (_os, data_to_write_parts.last ());

	return _os;
}

QString
OAF::cleanUid (const QUuid& _id)
{
	return _id.toString ().remove (QRegExp ("([{]|[}])"));
}

QString
OAF::asLatex (const QString& _s)
{
	//
	// Итоговая закодированная строка
	//
	QString res;

	//
	// Для всех символов строки
	//
	foreach (const QChar& c, _s)
	{
		//
		// Анализируем ASCII-представление символа
		//
		switch (c.toAscii ())
		{
			case '\\':
				res += "\\textbackslash{}";
				break;

//			case '<':
//				res += "\\textless{}";
//				break;

//			case '>':
//				res += "\\textgreater{}";
//				break;

			case '|':
				res += "\\textbar{}";
				break;

			case '\"':
				res += "\\char`\\\"{}";
				break;

			case '~':
				res += "\\textasciitilde{}";
				break;

			case '^':
				res += "\\textasciicircum{}";
				break;

			case '$':
			case '&':
			case '%':
			case '#':
			case '{':
			case '}':
				res +=  '\\';
				res += c;
				break;
			case '_':
                res += "\\UnderLine{}";
				break;

			case '*':
			case '[':
			case ']':
				res += '{';
				res += c;
				res += '}';
				break;

			default:
				res += c;
				break;
		}
	}

	return res;
}
