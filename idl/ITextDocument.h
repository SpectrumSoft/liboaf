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
	 * @brief Поиск дочерних компонентов в дереве владельца
	 *
	 * @obsoleted
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

	/**
	 * @brief Разрешение ссылок на внешние объекты в документах
	 *
	 * Объекты, которые возвращаются должны поддерживать интерфейсы:
	 *     * OAF::IPropertyBag - для генерации ссылок на объекты (должны быть
	 *       определены атрибуты id, uid и name);
	 *     * OAF::IExportable - для включения содержания объектов в документы.
	 */
	struct ITextDocumentRefResolver : virtual public OAF::IInterface
	{
		/**
		 * @brief Поиск объекта по его идентификатору
		 */
		virtual OAF::URef<OAF::IUnknown> refResolve (const QString& _id) const = 0;
	};

	/**
	 * @brief Определение словарных терминов и получение их описаний
	 */
	struct ITextDocumentTermResolver : virtual public OAF::IInterface
	{
		/**
		 * @brief Время последнего обновления базы терминов в формате time_t
		 */
		virtual uint termsUpdatedAt () = 0;

		/**
		 * @brief Заданное слово распознано как словарный термин
		 */
		virtual bool isTerm (const QString& _term) = 0;

		/**
		 * @brief Получить набор идентификаторов словарных статей для
		 *        заданного термина
		 *
		 * При специальном значении @a _term @@referred возвращается список
		 * используемых терминов.
		 */
		virtual std::size_t enumTermArticles (QStringList& _out, const QString& _term) = 0;

		/**
		 * @brief Получить словарную статью по её идентификатору
		 *
		 * Словарная статья должна реализовать интерфейс OAF::IPropertyBag с атрибутами:
		 *     * id          - идентификатор статьи (QUuid);
		 *     * name        - термин, который описывается в статье;
		 *     * description - HTML-аннотация статьи.
		 *
		 * Словарная статья должна реализовать интерфейс OAF::IObjectCollection, через
		 * который должен быть доступен объект с интерфейсами OAF::ITextDocument и
		 * OAF::IExportable и идентификатором description, который содержит текст статьи.
		 */
		virtual OAF::URef<OAF::IUnknown> getTermArticle (const QString& _id) = 0;

		/**
		 * @brief Получить словарь для заданной словарной статьи
		 *
		 * Словарь должен реализовать интерфейс OAF::IPropertyBag с атрибутами:
		 *     * id   - идентификатор словаря (QUuid);
		 *     * uid  - пользовательский идентификатор словаря (QString)
		 *     * name - наименование словаря (QString).
		 */
		virtual OAF::URef<OAF::IUnknown> getTermDictionary (const QString& _id) = 0;
	};

	/**
	 * @brief Текстовый документ
	 */
	struct ITextDocument : virtual public OAF::IInterface
	{
		/**
		 * @brief Установить объект, который должен разрешать ссылки
		 *        на внешние объекты в документе
		 */
		virtual void setRefResolver (ITextDocumentRefResolver* _rr) = 0;

		/**
		 * @brief Установить объект, который должен разрешать ссылки
		 *        на термины в документе
		 */
		virtual void setTermResolver (ITextDocumentTermResolver* _tr) = 0;

		/**
		 * @brief Проверка возможности добавления ссылок на внешние объекты в документ
		 */
		virtual bool canAddReferences () const = 0;

		/**
		 * @brief Добавление ссылки на внешний объект в документ
		 *
		 * FIXME: в случае когда представление отделено от документа для вставки
		 *        ссылки необходимо задать место документа, куда её вставлять.
		 *
		 * @param[in] _id идентификатор внешнего объекта
		 * @param[in] _collapse признак того, что при экспорте вместо объекта
		 *                      должна быть вставлена ссылка на него
		 * @param[in] _editable признак того, что объект может быть отредактирован
		 *                      в месте вставки
		 */
		virtual void addReference (const QUuid& _id, bool _collapse = false, bool _editable = true) = 0;

		/**
		 * @brief Проверка возможности добавления ссылок на словари в документ
		 */
		virtual bool canAddDictionaries () const = 0;

		/**
		 * @brief Добавление ссылок на словари в документ
		 */
		virtual void addDictionaries (const QList<QUuid>& _ids, int _includes) = 0;

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
}

#endif /* __ITEXT_DOCUMENT_H */
