/**
 * @file
 * @brief Реализация вспомогательных классов и функций для загрузки/записи текстовых документов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <idl/ITXTPersist.h>

#include <OAF/StreamUtils.h>
#include <OAF/Helpers.h>
#include <OAF/HelpersTXT.h>

using namespace OAF;

URef<OAF::ITextDocument>
OAF::createTXTFromXML (QXmlStreamReader& _is, const QStringList& _mime_types)
{
	//
	// Проверяем, что следующей секцией является CDATA
	//
	if ((_is.readNext () == QXmlStreamReader::Characters) && _is.isCDATA ())
	{
		//
		// Данные для загрузки
		//
		QByteArray data = _is.text ().toString ().toUtf8 ();

		//
		// Создаём вспомогательное устройство ввода/вывода
		//
		if (OAF::URef<OAF::CDeviceDerived> d = new CDeviceDerived (&data, OAF::getStreamIODevice (_is)))
		{
			//
			// Открываем вспомогательное устройство ввода/вывода для чтения
			//
			if (d->device ()->open (QIODevice::ReadOnly))
			{
				//
				// Создаём текстовый поток для данного устройства
				//
				QTextStream s (d->device ());

				//
				// Создаём объект
				//
				return OAF::createFromTXT<OAF::ITextDocument> (s, _mime_types);
			}
		}
	}

	return URef<OAF::ITextDocument> ();
}

QXmlStreamWriter&
OAF::saveTXTToXML (QXmlStreamWriter& _os, const QStringList& _mime_types, OAF::ITextDocument* _document)
{
	//
	// Если документ поддерживает запись в текстовый поток
	//
	if (OAF::ITXTPersist* ss = OAF::queryInterface<OAF::ITXTPersist> (_document))
	{
		//
		// Данные для записи в XML-поток
		//
		QByteArray data;

		//
		// Создаём вспомогательное устройство ввода/вывода
		//
		if (OAF::URef<OAF::CDeviceDerived> d = new CDeviceDerived (&data, OAF::getStreamIODevice (_os)))
		{
			//
			// Открываем вспомогательное устройство ввода/вывода для записи
			//
			if (d->device ()->open (QIODevice::WriteOnly))
			{
				//
				// Создаём текстовый поток для данного устройства
				//
				QTextStream s (d->device ());

				//
				// Сохраняем текстовый документ в потоке
				//
				ss->save (s, _mime_types);
			}
		}

		//
		// Проверяем текст на валидность в рамках XML: критерием служит "печатность" символа,
		// или принадлежность к разделителям
		//
		// TODO: возможно, упущены ещё какие-то непечатные, но нужные и корректные символы
		//
		QString cdata_text = QString::fromUtf8 (data, data.size ());
		QString cdata_text_validated;
		for (QString::iterator ci = cdata_text.begin (); ci != cdata_text.end (); ++ci)
		{
			if (ci->isPrint () || ci->isSpace ())
				cdata_text_validated += (*ci);
		}
		if (cdata_text.length () != cdata_text_validated.length ())
			qDebug ("Invalid symbols were skipped during saving the module");

		//
		// Записываем полученные данные в CDATA-секцию XML-потока
		//
		_os.writeCDATA (cdata_text_validated);
	}

	return _os;
}
