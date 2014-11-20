/**
 * @file
 * @brief Реализация вспомогательных классов и функций для загрузки/записи текстовых документов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QTextStream>
#include <QTextList>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocumentFragment>

#include <idl/ITXTPersist.h>

#include <OAF/Helpers.h>
#include <OAF/HelpersTXT.h>

using namespace OAF;

static const QString device_derived_cid = "OAF/DeviceDerived:1.0";

CDeviceDerived::CDeviceDerived (QByteArray* _data, OAF::IIODevice* _base) :
	CUnknown (device_derived_cid), m_base (_base)
{
	m_buffer = new QBuffer (this);
	m_buffer->setBuffer (_data);
}

CDeviceDerived::~CDeviceDerived ()
{}

QVariant
CDeviceDerived::getInfo (DeviceInfo _what)
{
	if (m_base)
		return m_base->getInfo (_what);

	//
	// В остальных случаях результат не определён
	//
	return QVariant ();
}

void
CDeviceDerived::setInfo (DeviceInfo _what, const QVariant& _v)
{
	if (m_base)
		m_base->setInfo (_what, _v);
}

QIODevice*
CDeviceDerived::device ()
{
	return m_buffer;
}

bool
CDeviceDerived::exists () const
{
	return (m_base ? m_base->exists () : false);
}

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
		if (OAF::URef<OAF::CDeviceDerived> d = new CDeviceDerived (&data, _is.device () ? OAF::queryInterface<OAF::IIODevice> (_is.device ()->parent ()) : NULL))
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
OAF::saveTXTToXML (QXmlStreamWriter& _os, const QStringList& _mime_types, OAF::ITextDocument* _document, OAF::CSaveParams& _save_params)
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
		if (OAF::URef<OAF::CDeviceDerived> d = new CDeviceDerived (&data, OAF::queryInterface<OAF::IIODevice> (_os.device ()->parent ())))
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
				ss->save (s, _mime_types, _save_params);
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
			/*else
				qDebug() << "Invalid char detected" << (*ci)
						 << ", code:" << (int)ci->toAscii () << ", pos:"
						 << std::distance (cdata_text.begin (), ci);*/
		}
		if (cdata_text.length () != cdata_text_validated.length ())
			qDebug () << QCoreApplication::translate ("helpers_txt", "Invalid symbols were skipped during saving the module");

		//
		// Записываем полученные данные в CDATA-секцию XML-потока
		//
		_os.writeCDATA (cdata_text_validated);
	}

	return _os;
}

/**
 * @brief Максимальный размер аннотации
 *
 * Это не строгий максимальный размер, реальный максимальный размер равен 2*MAX_BLOCK_LENGTH + 3.
 * Такой размер связан с тем, что мы копируем текст поблочно пока данная константа не окажется
 * превышена, при этом из каждого блока мы копируем не более чем MAX_BLOCK_LENGTH символов и если
 * документ скопирован не весь, то в конце добавляется троеточие.
 *
 * Данная величина подобрана экспериментально для экрана ноутбука 13".
 */
#define MAX_BLOCK_LENGTH 600

QString
OAF::getAnnonation(const QTextDocument& _doc)
{
	//
	// Документ для аннотации и курсор для его заполнения
	//
	QTextDocument ann_document;
	QTextCursor   ann_cursor (&ann_document);

	//
	// Максимальный размер аннотации
	//
	int limit = MAX_BLOCK_LENGTH;

	//
	// Копируем формат корневого фрейма документа
	//
	ann_document.rootFrame ()->setFrameFormat (_doc.rootFrame ()->frameFormat ());

	//
	// Переменные для копирования блоков, входящих в списки
	//
	QTextList* list_old = NULL;
	QTextList* list_new = NULL;

	//
	// Копируем блоки корневого фрейма документа до тех пор, пока не исчерпаем их или
	// не закончится лимит. Вложенные фреймы, изображения и таблицы пропускаем.
	//
	for (QTextFrame::iterator f = _doc.rootFrame ()->begin (); f != _doc.rootFrame ()->end (); ++f)
	{
		//
		// Блоки вложенных фреймов пропускаем. Поскольку таблицы являются вложенными фреймами,
		// то таблицы тоже пропускаем
		//
		if (f.currentFrame ())
			continue;

		//
		// Блок корневого фрейма
		//
		QTextBlock block = f.currentBlock ();

		//
		// Если это не первый блок документа, то создаём новый блок (первый блок документа
		// создаётся автоматически)
		//
		if (!ann_cursor.atStart ())
			ann_cursor.insertBlock ();

		//
		// Копируем формат блока
		//
		ann_cursor.setBlockFormat (block.blockFormat ());

		//
		// Копируем формат символов блока по умолчанию
		//
		ann_cursor.setCharFormat (block.charFormat ());

		//
		// Проходим по всем фрагментам текста в исходном блоке и копируем их в созданный блок
		//
		for (QTextBlock::iterator c = block.begin (); c != block.end (); ++c)
		{
			//
			// Фрагмент текста
			//
			QTextFragment fragment = c.fragment ();

			//
			// Фрагменты с рисунками пропускаем
			//
			QTextImageFormat imgf = fragment.charFormat ().toImageFormat ();
			if (imgf.isValid ())
				continue;

			//
			// Текст фрагмента для копирования
			//
			QString text_to_copy = fragment.text ().left (MAX_BLOCK_LENGTH);

			//
			// Копируем текст фрагмента вместе с форматированием
			//
			ann_cursor.insertText (text_to_copy, fragment.charFormat ());

			//
			// Уменьшаем лимит на размер скопированного текста
			//
			limit -= text_to_copy.length ();

			//
			// Если лимит исчерпан, то завершаем обработку текста
			//
			if (limit < 0)
				break;
		}

		//
		// Если копируемый блок находится в списке
		//
		if (block.textList ())
		{
			//
			// Если блок находится в новом списке
			//
			if (list_old != block.textList ())
				list_new = ann_cursor.createList ((list_old = block.textList ())->format ());
			//
			// Иначе блок находится в том же списке
			//
			else
				list_new->add (ann_cursor.block ());
		}

		//
		// Если лимит аннотации исчерпан
		//
		if (limit < 0)
		{
			//
			// Если документ скопирован в аннотацию не весь
			//
			if (!f.atEnd ())
			{
				//
				// Добавляем троеточие
				//
				QTextBlockFormat fmtb;
				fmtb.setAlignment (Qt::AlignHCenter);
				ann_cursor.insertBlock (fmtb);
				ann_cursor.insertText ("...");
			}

			break;
		}
	}

	//
	// В качестве аннотации возвращаем созданный документ, преобразованный в HTML
	//
	return ann_document.toHtml ();
}
