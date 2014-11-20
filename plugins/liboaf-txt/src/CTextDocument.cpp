/**
 * @file
 * @brief Реализация текстового документа
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QImageReader>
#include <QTextList>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocumentFragment>

#include <idl/ITextDocument.h>

#include <OAF/OAF.h>
#include <OAF/HelpersTXT.h>
#include <OAF/HtmlHelpers.h>
#include <OAF/Utils.h>
#include <OAF/diff_match_patch.h>

#include "CTextDocument.h"

using namespace OAF::TXT;

QVariant
CTextDocument::loadResource (int _type, const QUrl& _name)
{
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
CTextDocument::imgPathsToRelative (const QString& _module_path)
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
						img_format.setName (OAF::makeRelativePath (_module_path, img_format.name ()));

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
CTextDocument::imgPathsFromRelative (const QString& _module_path)
{
	//
	// Делаем обход документа и заменяем для всех QTextImageFormat
	// пути картинок, ссылающихся на файлы,
	// с платформенно-независимых относительных на абсолютные для корректной работы loadResource
	//
	QTextCursor cursor (this);
	for (QTextBlock block = begin (); block.isValid () && (block != end ().next ()); block = block.next ())
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
						img_format.setName (OAF::makeAbsolutePath (_module_path, img_format.name ()));

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

CTextDocument::CTextDocument (CFactory* _factory) : CUnknown (text_document_cid), m_factory (_factory)
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

			QString module_path = OAF::getStreamPath (_is);
			if (!module_path.isEmpty ())
				imgPathsFromRelative (module_path);
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
			QString module_path = OAF::getStreamPath (_os);
			if (!module_path.isEmpty ())
				_os << imgPathsToRelative (module_path);
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
	// Задаем цвет фоновой заливки документа: зеленый цвет для добавленного в другом объекте содержимого
	// и красный для удаленного
	//
	// FIXME: т.к. просто вставлять один HTML-документ в другой есть некорректно (несколько html-тегов и т.д.),
	// а делать по-другому (вынести все стили в заголовок главного документа) - сложно,
	// то пока попросту убрал задание цвета для текста. Всё равно все содержимое таблицы будет окрашено.
	/*Q_ASSERT (rootFrame ());
	QTextFrameFormat ff = rootFrame ()->frameFormat ();
	ff.setBackground (QBrush (QColor (_inserted ? "#e6ffe6" : "ffe6e6")));
	rootFrame ()->setFrameFormat (ff);*/

	_hg.insertHtml (toHtml ());
}

namespace
{
	//
	// Проверяет, есть ли хоть одно изменение в plain text-содержимом указанных документов
	//
	static
	bool
	hasPlainDiff (QTextDocument* _doc, QTextDocument* _other_doc)
	{
		QString text = _doc->toPlainText ();
		QString other_text = _other_doc->toPlainText ();

		static QScopedPointer <Google::diff_match_patch> dmp;
		if (! dmp)
			dmp.reset (new Google::diff_match_patch ());

		//
		// Ищем первое изменение текста - если нашли, возвращаем true
		//
		QList <Google::Diff> diff_data = dmp->diff_main (text, other_text);
		foreach (Google::Diff diff, diff_data)
		{
			if (diff.operation != Google::_EQUAL)
				return true;
		}

		return false;
	}

	//
	// Возвращает изменения в plain text-содержимом указанных документов
	//
	static
	QString
	findPlainDiff (QTextDocument* _doc, QTextDocument* _other_doc)
	{
		QString text = _doc->toPlainText ();
		QString other_text = _other_doc->toPlainText ();

		static QScopedPointer <Google::diff_match_patch> dmp;
		if (! dmp)
			dmp.reset (new Google::diff_match_patch ());

		//
		// Генерируем отчет об изменениях в виде HTML-текста
		//
		QList <Google::Diff> diff_data = dmp->diff_main (text, other_text);
		QString diff_res = dmp->diff_prettyHtml (diff_data);
		return diff_res;
	}
}

bool
CTextDocument::isDiffer (IUnknown* _other)
{
	OAF::URef<QTextDocument> other_doc = OAF::queryInterface <QTextDocument> (_other);
	return (other_doc ? hasPlainDiff (this, other_doc) : false);
}

void
CTextDocument::diff (OAF::CHtmlGenerator& _hg, IUnknown* _other)
{
	OAF::URef<QTextDocument> other_doc = OAF::queryInterface <QTextDocument> (_other);
	if (other_doc)
		_hg.insertHtml (findPlainDiff (this, other_doc));
}

int
CTextDocument::match (const QString& _pattern, Qt::CaseSensitivity _cs)
{
	QTextCursor cur;
	if (_cs == Qt::CaseSensitive)
		cur = find (_pattern, 0, QTextDocument::FindCaseSensitively);
	else
		cur = find (_pattern);

	if (!cur.isNull ())
		return 1;
	return 0;
}

std::size_t
CTextDocument::exportMimeTypes (QStringList& _out) const
{
	_out << "text/html" << "text/plain";
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
		// Если первым встретился text/html, то пишем документ как HTML-документ
		//
		if (*m == "text/html")
		{
			QString module_path = OAF::getStreamPath (_os);
			if (!module_path.isEmpty ())
				data = imgPathsToRelative (module_path).toUtf8 ();
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

			QString module_path = OAF::getStreamPath (_is);
			if (!module_path.isEmpty ())
				imgPathsFromRelative (module_path);
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
