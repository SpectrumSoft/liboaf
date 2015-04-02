/**
 * @file
 * @brief Интерфейс общего класса для всех объектов компонента
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_TXT_CCOMMON_H
#define __OAF_TXT_CCOMMON_H

#include <QtCore>

namespace OAF
{
	namespace TXT
	{
		/**
		 * @brief Общий класс для всех объектов компонента
		 */
		class CCommon
		{
			/**
			 * @brief Глобальный счетчик объектов компонента
			 */
			static long m_objects;

		public:
			static const QString factory_cid;
			static const QString plain_document_cid;
			static const QString plain_edit_cid;
			static const QString html_document_cid;
			static const QString html_edit_cid;

			CCommon ()
			{
				m_objects++;
            }

			~CCommon ()
			{
				m_objects--;
            }

			/**
			 * @brief Доступ к глобальному счетчику объектов
			 */
			static long objects ()
			{
				return m_objects;
            }
		};
	}
}

#endif /* __OAF_TXT_CCOMMON_H */
