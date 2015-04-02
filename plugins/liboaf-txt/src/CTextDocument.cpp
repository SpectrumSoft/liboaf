/**
 * @file
 * @brief Реализация HTML/plain документа
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <idl/ITextDocument.h>

#include <OAF/OAF.h>
#include <OAF/HelpersTXT.h>
#include <OAF/HtmlHelpers.h>
#include <OAF/StreamUtils.h>
#include <OAF/diff_match_patch.h>

#include "CTextDocument.h"

using namespace OAF::TXT;

/**
 * @brief Читаем данные как поток байт
 */
static size_t
readAll (QDataStream& _is, QByteArray& _out)
{
	char buf[1024];

	while (int count = _is.readRawData (buf, sizeof (buf)))
	{
		if (count < 0)
			break;

		_out.append (buf, count);

		if (_is.atEnd ())
			break;
	}

	return _out.size ();
}

/**
 * @brief Проверяет, есть ли хоть одно различие в указанных документах
 */
static bool
hasPlainDiff (QTextDocument* _d1, QTextDocument* _d2)
{
	//
	// Движок для сравнения простых текстов
	//
	QScopedPointer<Google::diff_match_patch> differ (new Google::diff_match_patch ());

	//
	// Тексты для сравнения
	//
	QString d1 = _d1->toPlainText ();
	QString d2 = _d2->toPlainText ();

	//
	// Если нашли хоть одно изменение, то возвращаем true
	//
	QList<Google::Diff> diff = differ->diff_main (d1, d2);
	foreach (Google::Diff d, diff)
	{
		if (d.operation != Google::_EQUAL)
			return true;
	}

	//
	// Иначе возвращаем false
	//
	return false;
}

/**
 * @brief Возвращает различия между заданными документами
 */
static QString
findPlainDiff (QTextDocument* _d1, QTextDocument* _d2)
{
	//
	// Движок для сравнения простых текстов
	//
	QScopedPointer<Google::diff_match_patch> differ (new Google::diff_match_patch ());

	//
	// Тексты для сравнения
	//
	QString d1 = _d1->toPlainText ();
	QString d2 = _d2->toPlainText ();

	//
	// Генерируем отчет об изменениях в виде HTML-текста
	//
	return differ->diff_prettyHtml (differ->diff_main (d1, d2));
}

QVariant
CTextDocument::loadResource (int _type, const QUrl& _name)
{
	//
	// Если ресурс является изображением и его имя не пусто
	//
	if ((_type == QTextDocument::ImageResource) && !_name.isEmpty())
	{
		//
		// Создаём объект ввода. Здесь будут обработаны и raw данные (в формате raw:<hex>)
		// и локальные файлы (в формате file:///путь/локальный_файл.расширение) и любые
		// другие, для которых реализована поддержка в плагине liboaf-io
		//
		if (URef<OAF::IIODevice> d = OAF::createFromName<OAF::IIODevice> (_name.toString ()))
		{
			//
			// Если это устройство успешно открыто для чтения
			//
			if (d->device ()->open (QIODevice::ReadOnly))
			{
				//
				// Загружаем изображение
				//
				QImage img = QImageReader (d->device ()).read ();

				//
				// Кэшируем загруженную картинку в ресурсах QTextDocument, чтобы не грузить
				// её каждый раз при отрисовке
				//
				addResource (_type, _name, img);

				//
				// Возвращаем загруженную картинку
				//
				return img;
			}
		}
	}

	return QTextDocument::loadResource (_type, _name);
}

OAF::IPropertyBag::Value
CTextDocument::getVirtualValue (const Key& _key) const
{
	if (_key == "path")
		return m_path;

	return Value ();
}

void
CTextDocument::setVirtualValue (const Key& _key, const Value& _value)
{
	if (_key == "path")
	{
		//
		// Если файл с таким путём уже открыт, то такой путь задать нельзя
		//
		if (m_factory->get (_value.toString ()))
			throw OAF::IPropertyNotify::Cancel ();

		if (!m_path.isEmpty ())
			m_factory->remove (m_path);

		m_path = _value.toString ();

		if (!m_path.isEmpty ())
			m_factory->add (m_path, this);
	}
}

bool
CTextDocument::isVirtualDefined (const Key& _key) const
{
	if (_key == "path")
		return !m_path.isEmpty ();

	return false;
}

QString
CTextDocument::imgPathsToRelative (const QString& _path)
{
	//
	// Делаем копию документа и заменяем в ней все пути картинок на относительные;
	// курочить оригинал не стоит, т.к. loadResource работает по абсолютным путям
	//
	QScopedPointer <QTextDocument> doc (clone (this));

	//
	// Делаем обход документа и заменяем для всех QTextImageFormat
	// пути картинок, ссылающихся на файлы,
	// с текущих абсолютных на относительные относительно текущего потока (xml/text)
	//
	QTextCursor cursor (doc.data ());
	for (QTextBlock block = doc->begin (); block.isValid () && (block != doc->end ().next ()); block = block.next ())
	{
		for (QTextBlock::iterator it = block.begin (); !it.atEnd (); ++it)
		{
			QTextFragment fragment = it.fragment ();
			if (fragment.isValid ())
			{
				if (fragment.charFormat ().isImageFormat ())
				{
					QTextImageFormat img_format = fragment.charFormat ().toImageFormat ();
					if (img_format.isValid ())
					{
						img_format.setName (OAF::makeRelativePath (_path, img_format.name ()));

						int img_pos = fragment.position();
						cursor.setPosition (img_pos);
						cursor.setPosition (img_pos + fragment.length(), QTextCursor::KeepAnchor);
						cursor.setCharFormat (img_format);
					}
				}
			}
		}
	}

	return doc->toHtml ();
}

void
CTextDocument::imgPathsFromRelative (const QString& _path)
{
	//
	// Делаем обход документа и заменяем для всех QTextImageFormat
	// пути картинок, ссылающихся на файлы, с платформенно-независимых
	// относительных на абсолютные для корректной работы loadResource
	//
	QTextCursor cursor (this);
	for (QTextBlock block = begin (); block.isValid () && (block != end ()); block = block.next ())
	{
		for (QTextBlock::iterator it = block.begin (); !it.atEnd (); ++it)
		{
			QTextFragment fragment = it.fragment ();
			if (fragment.isValid ())
			{
				if (fragment.charFormat ().isImageFormat ())
				{
					QTextImageFormat img_format = fragment.charFormat ().toImageFormat ();
					if (img_format.isValid ())
					{
						img_format.setName (OAF::makeAbsolutePath (_path, img_format.name ()));

						int img_pos = fragment.position();
						cursor.setPosition (img_pos);
						cursor.setPosition (img_pos + fragment.length(), QTextCursor::KeepAnchor);
						cursor.setCharFormat (img_format);
					}
				}
			}
		}
	}
}

void
CTextDocument::aboutContentsChanged ()
{
	//
	// Рассылаем уведомление об изменении текстового документа. В данном случае инициатором
	// изменений является сам текстовый документ, который не является слушателем и уведомлений
	// не получает
	//
	OAF::ITextDocumentNotify ev (this);
	notifyListeners (&ev, NULL);
}

CTextDocument::CTextDocument (CFactory* _factory, const QString& _cid) : CUnknown (_cid), m_factory (_factory)
{
	connect (this, SIGNAL (contentsChanged ()), this, SLOT (aboutContentsChanged ()));
}

CTextDocument::~CTextDocument ()
{
	disconnect (this, SIGNAL (contentsChanged ()), this, SLOT (aboutContentsChanged ()));

	//
	// Если данные связаны с источником, то удаляем запись о данных из кэша
	//
	if (!m_path.isEmpty ())
		m_factory->remove (m_path);
}

OAF::URef<OAF::IUnknown>
CTextDocument::setIODevice (OAF::IIODevice* _io)
{
	Q_ASSERT (_io != NULL);

	//
	// Если устройство больше никому не нужно, то после использования оно будет освобождено
	//
	OAF::URef<OAF::IIODevice> io = _io;

	//
	// Получаем ссылку на источник данных
	//
	QString path = io->getInfo (OAF::IIODevice::PATH).value<QString> ();

	//
	// Запрашиваем фабрику об уже имеющемся объекте, связанном с данным
	// источником. Если такой объект существует, то возвращаем его.
	//
	if (OAF::URef<OAF::IUnknown> document = m_factory->get (path))
		return document;

	//
	// Запрашиваем у устройства список его MIME-типов
	//
	QStringList mime_types = io->getInfo (OAF::IIODevice::CONTENT_TYPE).value<QStringList> ();

	//
	// Открываем устройство на чтение как текстовый поток
	//
	if (io->device ()->open (QIODevice::ReadOnly))
	{
		//
		// Собственно чтение текста
		//
		{
			QTextStream ios (_io->device ());
			load (ios, mime_types);
		}

		//
		// Закрываем устройство так как оно нам больше не нужно
		//
		io->device ()->close ();
	}

	//
	// Регистрируем связь в фабрике
	//
	m_factory->add ((m_path = path), this);

	//
	// Возвращаем ссылку на себя
	//
	return this;
}

QTextStream&
CTextDocument::load (QTextStream& _is, const QStringList& _mime_types)
{
	//
	// Устанавливаем нужный unicode-кодек
	//
	_is.setCodec ("UTF-8");

	//
	// Читаем документ
	//
	QString document = _is.readAll ();

	//
	// Загружаем документ в зависимости от его MIME-типа
	//
	for (QStringList::const_iterator m = _mime_types.begin (); m != _mime_types.end (); ++m)
	{
		//
		// Если первым встретился text/html, то открываем документ как HTML-документ
		//
		if (*m == "text/html")
		{
			setHtml (document);

			//
			// Преобразуем ссылки на изображения из относительных в абсолютные
			//
			QString path = OAF::getStreamPath (_is);
			if (!path.isEmpty ())
				imgPathsFromRelative (path);

			break;
		}

		//
		// Если первым встретился text/plain, то открываем документ как текстовый документ
		//
		if (*m == "text/plain")
		{
			setPlainText (document);
			break;
		}
	}

	//
	// Сбрасываем признак модификации документа
	//
	setModified (false);
	return _is;
}

QTextStream&
CTextDocument::save (QTextStream& _os, const QStringList& _mime_types)
{
	//
	// Устанавливаем нужный unicode-кодек
	//
	_os.setCodec ("UTF-8");

	//
	// Пишем документ в зависимости от его MIME-типа
	//
	for (QStringList::const_iterator m = _mime_types.begin (); m != _mime_types.end (); ++m)
	{
		//
		// Если первым встретился text/html, то пишем документ как HTML-документ
		//
		if (*m == "text/html")
		{
			//
			// Если задан путь к файлу документа, то преобразуем ссылки на
			// изображения из абсолютных в относительные
			//
			QString path = OAF::getStreamPath (_os);
			if (!path.isEmpty ())
				_os << imgPathsToRelative (path);
			else
				_os << toHtml ();

			break;
		}

		//
		// Если первым встретился text/plain, то пишем документ как текстовый документ
		//
		if (*m == "text/plain")
		{
			_os << toPlainText ();
			break;
		}
	}

	//
	// Сбрасываем признак модификации документа
	//
	setModified (false);
	return _os;
}

std::size_t
CTextDocument::enumKeys (KeyList& _out) const
{
	CNotifyPropertyBag::enumKeys (_out);
	_out.append ("path");

	return _out.size ();
}

OAF::IPropertyBag::PropertyFlags
CTextDocument::getFlags (const Key& _key) const
{
	if (_key == "path")
		return PROPERTY_WRITEABLE|PROPERTY_VIRTUAL|PROPERTY_PERSIST;

	return CNotifyPropertyBag::getFlags (_key);
}

void
CTextDocument::diff (OAF::CHtmlGenerator& _hg, bool _inserted)
{
	Q_UNUSED (_inserted);

	//
	// Задаем цвет фоновой заливки документа: зеленый цвет для добавленного в
	// другом объекте содержимого и красный для удаленного
	//
	// FIXME: т.к. просто вставлять один HTML-документ в другой есть некорректно
	//        (несколько html-тегов и т.д.), а делать по-другому (вынести все стили
	//        в заголовок главного документа) - сложно, то пока попросту убираем
	//        задание цвета для текста. Всё равно все содержимое таблицы будет
	//        окрашено.
//	Q_ASSERT (rootFrame ());
//	QTextFrameFormat ff = rootFrame ()->frameFormat ();
//	ff.setBackground (QBrush (QColor (_inserted ? "#e6ffe6" : "ffe6e6")));
//	rootFrame ()->setFrameFormat (ff);

	_hg.insertHtml (toHtml ());
}

bool
CTextDocument::isDiffer (IUnknown* _d)
{
	return (_d ? hasPlainDiff (this, OAF::queryInterface<QTextDocument> (_d)) : false);
}

void
CTextDocument::diff (OAF::CHtmlGenerator& _hg, IUnknown* _d)
{
	if (OAF::URef<QTextDocument> d = OAF::queryInterface<QTextDocument> (_d))
		_hg.insertHtml (findPlainDiff (this, d));
}

int
CTextDocument::match (const QString& _pattern, Qt::CaseSensitivity _cs)
{
	QTextCursor f;
	if (_cs == Qt::CaseSensitive)
		f = find (_pattern, 0, QTextDocument::FindCaseSensitively);
	else
		f = find (_pattern);

	return (!f.isNull ()) ? 1 : 0;
}

std::size_t
CTextDocument::exportMimeTypes (QStringList& _out) const
{
	_out << "application/pdf" << "text/html" << "text/plain";

	return _out.size ();
}

std::size_t
CTextDocument::exportFeatures (QStringList& _out, const QString& _mime_type)
{
	Q_UNUSED (_mime_type);

	return _out.size ();
}

void
CTextDocument::exportVerify (OAF::IPropertyBag* _warnings) const
{
	Q_UNUSED (_warnings);
}

QDataStream&
CTextDocument::exportTo (QDataStream& _os, const QStringList& _mime_types, IPropertyBag* _options, const QString& _efp)
{
	Q_UNUSED (_options);
	Q_UNUSED (_efp);

	QByteArray data;

	//
	// Пишем документ в зависимости от его MIME-типа
	//
	for (QStringList::const_iterator m = _mime_types.begin (); m != _mime_types.end (); ++m)
	{
		//
		// Если первым встретился application/pdf, то пишем документ как PDF-документ
		//
		if (*m == "application/pdf")
		{
			//
			// FIXME: Создать временный файл для печати, напечатать туда документ,
			// вывести документ в поток и удалить временный файл
			//
//			if (!file_name.isEmpty ())
//			{
//				if (QFileInfo (file_name).suffix().isEmpty())
//					file_name.append (".pdf");
//				QPrinter printer (QPrinter::HighResolution);
//				printer.setOutputFormat (QPrinter::PdfFormat);
//				printer.setOutputFileName (file_name);
//				document()->print (&printer);
//			}

			break;
		}

		//
		// Если первым встретился text/html, то пишем документ как HTML-документ
		//
		if (*m == "text/html")
		{
			//
			// Если задан путь к файлу документа, то преобразуем ссылки на
			// изображения из абсолютных в относительные
			//
			QString path = OAF::getStreamPath (_os);
			if (!path.isEmpty ())
				data = imgPathsToRelative (path).toUtf8 ();
			else
				data = toHtml ().toUtf8 ();

			break;
		}

		//
		// Если первым встретился text/plain, то пишем документ как текстовый документ
		//
		if (*m == "text/plain")
		{
			data = toPlainText ().toUtf8 ();
			break;
		}
	}

	//
	// Пишем документ как поток байт в кодировке UTF-8
	//
	_os.writeRawData (data.constData (), data.size ());

	return _os;
}

std::size_t
CTextDocument::importMimeTypes (QStringList& _out) const
{
	_out << "text/html" << "text/plain";

	return _out.size ();
}

QDataStream&
CTextDocument::importFrom (QDataStream& _is, const QStringList& _mime_types)
{
	QString document;

	//
	// Читаем документ как поток байт в кодировке UTF-8
	//
	{
		QByteArray data;
		readAll (_is, data);

		document = QString::fromUtf8 (data.constData (), data.size ());
	}

	//
	// Загружаем документ в зависимости от его MIME-типа
	//
	for (QStringList::const_iterator m = _mime_types.begin (); m != _mime_types.end (); ++m)
	{
		//
		// Если первым встретился text/html, то открываем документ как HTML-документ
		//
		if (*m == "text/html")
		{
			setHtml (document);

			//
			// Преобразуем ссылки на изображения из относительных в абсолютные
			//
			QString path = OAF::getStreamPath (_is);
			if (!path.isEmpty ())
				imgPathsFromRelative (path);

			break;
		}

		//
		// Если первым встретился text/plain, то открываем документ как текстовый документ
		//
		if (*m == "text/plain")
		{
			setPlainText (document);
			break;
		}
	}

	//
	// Устанавливаем признак модификации документа
	//
	setModified (true);
	return _is;
}

OAF::URef<OAF::IPropertyBag>
CTextDocument::setConfiguration (OAF::IPropertyBag* _config)
{
	OAF::URef<OAF::IPropertyBag> old = m_config;

	m_config = _config;

	return old;
}

OAF::URef<OAF::IPropertyBag>
CTextDocument::getConfiguration ()
{
	return m_config;
}

void
CTextDocument::showConfigurationDialog ()
{
	//
	// FIXME: не реализовано
	//
}

QString
CTextDocument::getDocumentAnnotation ()
{
	return OAF::getAnnonation (*this);
}

bool
CTextDocument::isEmptyDocument ()
{
	return isEmpty ();
}

void
CTextDocument::clear ()
{
	QTextDocument::clear ();
}
