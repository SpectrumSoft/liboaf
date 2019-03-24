/**
 * @file
 * @brief Вспомогательные классы и функции для ввода/вывода
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_STREAM_UTILS_H
#define __OAF_STREAM_UTILS_H

#include <QtCore>

#include <idl/IInterface.h>
#include <idl/IIODevice.h>

#include <OAF/CUnknown.h>
#include <OAF/OafCoreGlobal.h>

namespace OAF
{
	/**
	 * @brief Возвратить связанный с потоком OAF::IIODevice
	 *
	 * Предполагается, что все устройства QIODevice, связанные с
	 * OAF::IIODevice имеют объект, реализующий данный интерфейс,
	 * в качестве родительского объекта
	 */
	template<class _Stream>
	OAF::IIODevice* getStreamIODevice (const _Stream& _s)
	{
		//
		// Для некоторых потоков устройство ввода-вывода оказывается не определено.
		//
		if (_s.device ())
			return OAF::queryInterface<OAF::IIODevice> (_s.device ()->parent ());

		return NULL;
	}

	/**
	 * @brief Получить путь к файлу, с которым связан заданный поток
	 *
	 * @note Предполагается, что устройство ввода/вывода, с которым связан поток получено
	 *       с помощью объекта, поддерживающего интерфейс IIODevice.
	 *
	 * В качестве потоков можно использовать классы QXmlStreamReader, QXmlStreamWriter,
	 * QTextStream и другие, для которых определён метод device, возвращающий QIODevice.
	 */
	template<class _Stream>
	QString getStreamPath (const _Stream& _s)
	{
		if (OAF::IIODevice* d = OAF::getStreamIODevice (_s))
			return d->getInfo (OAF::IIODevice::PATH).toString ();

		return QString::null;
	}

	/**
	 * @brief Преобразовать второе абсолютное имя в относительное относительно первого
	 *
	 * @param _origin   путь, относительно которого строится результат
	 * @param _absolute путь, из которого строится результат
	 */
	OAFCORE_EXPORT QString makeRelativePath (const QString& _origin, const QString& _absolute);

	/**
	 * @brief Преобразовать второе имя из относительного относительно первого в абсолютное
	 */
	OAFCORE_EXPORT QString makeAbsolutePath (const QString& _origin, const QString& _relative);

	/**
	 * @brief Конструирование URL из локального файла
	 *
	 * Данная функция перенесена из Qt 4.x, так как не во всех версиях Qt 4.x она
	 * присутствует.
	 */
	OAFCORE_EXPORT QUrl fromLocalFile (const QString &local_file);

	/**
	 * @brief Представление URL в виде пути к локальному файлу
	 *
	 * Данная функция перенесена из Qt 4.x, так как не во всех версиях Qt 4.x она
	 * присутствует.
	 */
	OAFCORE_EXPORT QString toLocalFile (const QUrl& _url);

	/**
	 * @brief Вспомогательный класс для создания вторичного устройства ввода/вывода
	 *        на основе заданного
	 *
	 * Такие сложности нужны, чтобы обеспечить трансляцию абсолютных путей для ссылок
	 * на файлы в относительные и обратно для загрузки/сохранения текстовых документов
	 * в/из XML-потока связанного с файлом.
	 */
	class OAFCORE_EXPORT CDeviceDerived : public QObject,
		//
		// Импортируемые реализации
		//
		virtual public CUnknown,
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IUnknown,
		virtual public OAF::IIODevice
	{
		Q_OBJECT

		/**
		 * @brief Основное устройство ввода/вывода
		 */
		OAF::URef<OAF::IIODevice> m_base;

		/**
		 * @brief Буфер для ввода/вывода
		 */
		QBuffer m_buffer;

	public:
		CDeviceDerived (QByteArray* _data, OAF::IIODevice* _base = NULL);
		~CDeviceDerived ();

		/**
		 * @name Реализация интерфейса OAF::IIODevice
		 */
		/** @{*/
		QVariant getInfo (DeviceInfo _what);
		void setInfo (DeviceInfo _what, const QVariant& _v);
		QIODevice* device ();
		bool exists () const;
		/** @}*/
	};
}

#endif /* __OAF_STREAM_UTILS_H */
