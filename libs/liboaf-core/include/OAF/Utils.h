/**
 * @file
 * @brief Вспомогательные классы и функции
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_UTILS_H
#define __OAF_UTILS_H

#include <OAF/OafCoreGlobal.h>

#include <QUuid>
#include <QUrl>
#include <QImage>
#include <QIODevice>
#include <QComboBox>

#include <idl/IInterface.h>
#include <idl/IIODevice.h>

namespace OAF
{
	/**
	 * @brief Шаблон установки и сброса значения переменной в области видимости
	 */
	template<typename _Value>
	class ScopedValue
	{
		Q_DISABLE_COPY (ScopedValue)

		_Value& m_value;
		_Value  m_end;

	public:
		ScopedValue (_Value& _value, const _Value& _begin, const _Value& _end) :
			m_value (_value), m_end (_end)
		{
			m_value = _begin;
		};

		~ScopedValue ()
		{
			m_value = m_end;
		};

		const _Value& value () const
		{
			return m_value;
		};
	};

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
	 * @brief Поиск индекса в комбобоксе по связанному идентификатору
	 *
	 * Qt отвратительно работает с QUuid :-(
	 */
	OAFCORE_EXPORT int findById (QComboBox* _c, const QUuid& _id);

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
		//
		// Для некоторых потоков устройство ввода-вывода оказывается не определено.
		// 
		//
		if (_s.device ())
		{
			if (OAF::IIODevice* d = OAF::queryInterface<OAF::IIODevice> (_s.device ()->parent ()))
				return d->getInfo (OAF::IIODevice::PATH).toString ();
		}

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
	 * @brief Получить изображение по заданному URL с кэшированием результата
	 */
	OAFCORE_EXPORT QImage getImage (const QUrl& _url);

	/**
	 * @brief Вероятный путь к директории @a share
	 * @note Не обязан быть существующим и вообще верным! Это просто догадка на крайний случай,
	 * когда не задана @a PIKET_DATA_DIR
	 */
	OAFCORE_EXPORT QString defaultDataPath ();

	/**
	 * @brief "Усыпляет" текущий процесс на указанное время
	 * @note Время указывается в миллисекундах
	 */
	OAFCORE_EXPORT void sleep (int _ms);
}

#endif /* __OAF_UTILS_H */
