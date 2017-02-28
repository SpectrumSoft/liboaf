/**
 * @file
 * @brief Вспомогательные интерфейсы для текстового документа
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __ITEXT_DOCUMENT_H
#define __ITEXT_DOCUMENT_H

#include <QtCore>

#include <idl/IInterface.h>
#include <idl/IUnknown.h>

namespace OAF
{
	/**
	 * @brief Текстовый документ
	 */
	struct ITextDocument : virtual public OAF::IInterface
	{
		/**
		 * @brief Аннотация документа, или его preview-версия (к примеру полезно для тултипов)
		 */
		virtual QString getDocumentAnnotation () = 0;

		/**
		 * @brief Проверить наличие текста в документе
		 */
		virtual bool isEmptyDocument () = 0;

		/**
		 * @brief Очистить документ
		 */
		virtual void clear () = 0;
	};

	/**
	 * @brief Уведомление об изменении содержимого текстового документа
	 *
	 * Используется в реализациях, которые предполагают рассылку уведомлений о изменениях своего
	 * состояния. Такие объекты должны также поддерживать интерфейс OAF::INotifySource.
	 */
	struct ITextDocumentNotify : virtual public OAF::IInterface
	{
		/**
		 * @brief Изменённый документ
		 *
		 * Данный параметр может не задаваться при рассылке уведомлений. Это зависит
		 * от их источника.
		 */
		OAF::IInterface* const object;

		/**
		 * @brief Подпись объекта, частью которого является данный документ
		 *
		 * Эта подпись нужна для того, чтобы отслеживать изменение объектов, которые
		 * включают в себя документы.
		 */
		OAF::IInterface* sign;

		/**
		 * @brief Конструктор уведомления
		 */
		ITextDocumentNotify (OAF::IInterface* _object, OAF::IInterface* _sign = NULL) :
			object (_object), sign (_sign)
		{}
	};

	/**
	  * @brief Поиск дочерних компонентов в дереве владельца
	  */
	struct ITextDocumentResolver : virtual public OAF::IInterface
	{
		/**
		 * @brief Получение компонента находящегося в структуре для поиска
		 */
		virtual OAF::IUnknown* getOwnerComponent () const = 0;

		/**
		 * @brief Установка компонента находящегося в структуре для поиска
		 */
		virtual void setOwnerComponent (OAF::IUnknown* _owner) = 0;

		/**
		 * @brief Добавление ссылки на указанный компонент в документ
		 */
		virtual void addComponent (const QUuid& _id, bool _collapse = false, bool _editable = true) = 0;

		/**
		 * @brief Проверка возможности добавления словарей
		 */
		virtual bool canAddDictionariesComponent () const = 0;

		/**
		 * @brief Добавление ссылки на компонент словарей в документ
		 */
		virtual void addDictionariesComponent (const QList<QUuid>& _id, int _includes) = 0;

		/**
		 * @brief Поиск компонента по его идентификатору
		 */
		virtual OAF::URef<OAF::IUnknown> findComponent (const QString& _id) const = 0;
	};
}

#endif /* __ITEXT_DOCUMENT_H */
