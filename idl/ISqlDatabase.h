/**
 * @file
 * @brief Доступ к SQL базам данных
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __ISQL_DATABASE_H
#define __ISQL_DATABASE_H

#include <QtCore>

#include <idl/IInterface.h>
#include <idl/IUnknown.h>

namespace OAF
{
	/**
	 * @brief Интерфейс доступа к SQL базам данных
	 */
	struct ISqlDatabase : virtual public OAF::IInterface
	{
		/**
		 * @brief Вид информации об SQL базе данных
		 */
		enum DatabaseInfo
		{
			/**
			 * @brief Полный путь к SQL базе данных в нормализованном виде (QString)
			 */
			PATH,

			/**
			 * @brief MIME-типы, ассоцированный с SQL базой данных в порядке увеличения общности (QStringList)
			 */
			CONTENT_TYPE
		};

		/**
		 * @brief Информация об SQL базе данных
		 */
		virtual QVariant getInfo (DatabaseInfo _what) = 0;

		/**
		 * @brief Задать параметры содинения с базой данных
		 *
		 * Позволяет задавать только DatabaseInfo::PATH
		 */
		virtual void setInfo (DatabaseInfo _what, const QVariant& _v) = 0;

		/**
		  * @brief Доступ к соединению с базой данных
		   */
		virtual QSqlDatabase database () = 0;
	};

	/**
	 * @brief Интерфейс расширения доступа к SQL базам данных
	 */
	struct ISqlDatabaseExtender : virtual public OAF::IInterface
	{
		/**
		 * @brief Задание SQL-базы данных для расширения
		 *
		 * @attention
		 * IDL-SQL-002. Клиент данного интерфейса должен иметь в виду, что в качестве результата
		 * вызова этого метода ему может вернуться ссылка на другой объект, который он и должен
		 * будет использовать.
		 *
		 * @return Объект, связанный с заданной базой данных
		 */
		virtual OAF::URef<OAF::IUnknown> setDatabase (OAF::ISqlDatabase* _dbs) = 0;
	};
}

#endif /* __ISQL_DATABASE_H */
