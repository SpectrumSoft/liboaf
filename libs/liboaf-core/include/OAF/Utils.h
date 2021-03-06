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

#include <QtCore>
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK (5, 0, 0))
#include <QtWidgets>
#endif

#include <idl/IInterface.h>
#include <idl/IIODevice.h>

#include <OAF/OafCoreGlobal.h>

namespace OAF
{
	/**
	 * @brief Шаблон временной установки значения переменной в текущей области
	 *        видимости
	 *
	 * После выхода из области видимости значение переменной возвращается к
	 * предыдущему значению
	 */
	template<typename _Value>
	class ScopedValue
	{
		/**
		 * @brief Ссылка на переменную
		 */
		_Value& m_value;

		/**
		 * @brief Предыдущее значение переменной
		 */
		_Value m_prev;

	public:
		ScopedValue (_Value& _value, const _Value& _new) : m_value (_value), m_prev (_value)
		{
			m_value = _new;
		}

		~ScopedValue ()
		{
			m_value = m_prev;
		}

		const _Value& value () const
		{
			return m_value;
		}
	};

	/**
	 * @brief Поиск индекса в комбобоксе по связанному идентификатору
	 *
	 * Qt отвратительно работает с QUuid :-(. Если использовать стандартную
	 * функцию findData, то ничего найдено не будет
	 */
	OAFCORE_EXPORT int findById (QComboBox* _c, const QUuid& _id);

	/**
	 * @brief Получить изображение по заданному URL с кэшированием результата
	 */
	OAFCORE_EXPORT QImage getImage (const QUrl& _url);

	/**
	 * @brief Путь к директории данных по умолчанию
	 *
	 * Не обязан существовать и вообще быть верным! Это просто догадка на крайний
	 * случай, когда не задана @a PIKET_DATA_DIR
	 */
	OAFCORE_EXPORT QString defaultDataPath ();

	/**
	 * @brief "Усыпляет" текущий процесс на указанное время
	 *
	 * Время указывается в миллисекундах. Используем собственную функцию,
	 * чтобы не зависеть от платформы.
	 */
	OAFCORE_EXPORT void sleep (int _ms);
}

#endif /* __OAF_UTILS_H */
