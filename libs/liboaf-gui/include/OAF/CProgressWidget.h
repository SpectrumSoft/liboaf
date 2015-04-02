/**
 * @file
 * @brief Интерфейс виджета для отображения прогресса длительной операции,
 *        а также для её отмены
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __CPROGRESSWIDGET_H
#define __CPROGRESSWIDGET_H

#include <QtCore>
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK (5, 0, 0))
#include <QtWidgets>
#endif

#include <idl/IUnknown.h>
#include <idl/IOperation.h>
#include <idl/IUIComponent.h>

#include <OAF/OafGuiGlobal.h>

namespace OAF
{
	/**
	 * @brief Виджет отображения прогресса длительной операции
	 *
	 * В сообщениях уведомлений о прогрессе ожидается следующие параметры:
	 *     * message        - строка сообщения для вывода в строку состояния;
	 *     * progress_max   - максимальное значение счётчика прогресса
	 *     * progress_value - текущее значение счётчика прогресса
	 *
	 * Для удобства можно воспользоваться соответствующими константами,
	 * определёнными в liboaf-std/include/OAF/Consts.h
	 *
	 * TODO: упрощение виджета:
	 *     * убрать все смарт-поинтеры и сделать вместо них обычные
	 *       указатели. Удаление объектов реализовать через механизм
	 *       дочерних объектов;
	 *     * убрать сигналы, сделать прямую установку данных виджета в
	 *       notify;
	 *     * кнопка Break должна напрямую вызывать operationStop. Объекты,
	 *       которые используют данный виджет, должны реагировать
	 *       на уведомление от операции, а не сигнал данного виджета (сейчас
	 *       получается что корректно можно среагировать на прерывание
	 *       операции только "своим" виджетом, но не со стороны).
	 */
	class OAFGUI_EXPORT CProgressWidget : public QWidget,
			//
			// Экспортируемые интерфейсы
			//
			virtual public OAF::IUIComponent,
			virtual public OAF::INotifyListener
	{
		Q_OBJECT

		/**
		 * @brief Интерфейс длительной операция, прогресс которой
		 *        отображает данный виджет
		 */
		OAF::URef<OAF::IOperation> m_operation;

		/**
		 * @brief Контейнер интерфейса пользователя для размещения
		 *        элементов пользовательского интерфейса прогресса
		 */
		OAF::IUIContainer* m_uic;

		/**
		 * @brief Элемент для отображения текстовых сообщений
		 */
		QScopedPointer<QLabel> m_operation_status;

		/**
		 * @brief Элемент для отображения прогресса в процентах выполнения
		 */
		QScopedPointer<QProgressBar> m_operation_progress;

		/**
		 * @brief Кнопка прерывания выполнения операции
		 */
		QScopedPointer<QPushButton> m_break_operation_button;

		/**
		 * @brief Нужно ли показывать кнопку прерывания выполнения
		 *        операции (по умолчанию = да)
		 */
		bool m_show_break_button;

		/**
		 * @brief Текстовое сообщение по умолчанию, отображаемый до
		 *        первого уведомления от операции
		 */
		QString m_default_status;

		/**
		 * @brief Идентификатор интерфейса виджета прогресса
		 */
		QUuid m_id_operation_status;

		/**
		 * @brief Текущее состояние активности виджета
		 */
		bool m_is_active;

		/**
		 * @brief Обновление данных по операции
		 */
		void updateOperation (const OAF::IPropertyBag::PropertySet& _v);

		/**
		 * @name Реализация интерфейса OAF::INotifyListener
		 */
		/** @{*/
		void notify (OAF::IInterface* _event, OAF::INotifySource* _source, OAF::INotifyListener* _origin);
		/** @}*/

	private slots:
		/**
		 * @brief Отображение или сокрытие виджетов прогресса операции
		 */
		void aboutOperationActivate (bool _activate);

	signals:
		/**
		 * @brief Сообщает об отмене операции пользователем
		 */
		void operationCancelled ();

		/**
		 * @name Сигналы процесса выполнения операции
		 */
		/** @{ */
		void updateOperationActivate (bool);
		void updateOperationStatus (const QString&);
		void updateOperationProgressMinimum (int);
		void updateOperationProgressMaximum (int);
		void updateOperationProgressValue (int);
		/** @} */

	public:
		CProgressWidget (QWidget* _parent = 0);
		~CProgressWidget ();

		/**
		 * @name Реализация интерфейса OAF::IUIComponent
		 */
		/** @{*/
		void setUILabel (const QString& _bag, const QString& _label);
		void setUIContainer (OAF::IUIContainer* _uic);
		OAF::IUIContainer* getUIContainer ();
		void activate (bool _active);
		QObject* getUIItem (const QString& _id);
		/** @}*/

		/**
		 * @brief Устанавливает объект операции
		 */
		void setOperationObject (OAF::IOperation* _op);

		/**
		 * @brief Устанавливает сообщение по умолчанию
		 */
		void setDefaultStatus (const QString& _status);

		/**
		 * @brief Показывает/скрывает кнопку для отмены операции
		 *
		 * По умолчанию кнопка показывается
		 */
		void setCancelButtonVisible (bool _visible = true);
	};
}

#endif /* __CPROGRESSWIDGET_H */
