/**
 * @file
 * @brief Интерфейс помощника, для изменения размеров QDockWidget
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __DOCKRESIZEHELPER_H
#define __DOCKRESIZEHELPER_H

#include <OAF/OafGuiGlobal.h>

#include <QDockWidget>
#include <QObject>

namespace OAF
{
	namespace Helpers
	{
		/**
		 * @brief Класс для организации изменения размеров QDockWidget
		 */
		class OAFGUI_EXPORT DockResizeHelper : public QObject
		{
			Q_OBJECT

			/**
			 * @brief Обслуживаемая панель
			 */
			QDockWidget* m_dock;

			/**
			 * @brief Старые размеры панели
			 */
			/** @{*/
			QSize m_old_min_size;
			QSize m_old_max_size;
			/** @}*/

		private slots:
			/**
			 * @brief Поддержка хака для изменения размеров панели
			 */
			void aboutReturnToOldMaxMinSizes ();

		public:
			DockResizeHelper (QDockWidget* _dock, QObject* _parent = NULL);

			/**
			 * @brief Задать размеры панели
			 */
			void setSize (const QSize& _sz);
		};
	}
}

#endif /* __DOCKRESIZEHELPER_H */
