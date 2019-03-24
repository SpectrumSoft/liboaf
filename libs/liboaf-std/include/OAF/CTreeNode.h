/**
 * @file
 * @brief Интерфейс реализации узла DOM-подобного дерева
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __CTREE_NODE_H
#define __CTREE_NODE_H

#include <QtCore>

#include <idl/IInterface.h>
#include <idl/IUnknown.h>
#include <idl/INotifySource.h>
#include <idl/ITreeNode.h>
#include <idl/IObjectSource.h>

#include <OAF/CNotifySource.h>
#include <OAF/OafStdGlobal.h>

namespace OAF
{
	/**
	 * @brief Реализация узла DOM-подобного дерева
	 */
	class OAFSTD_EXPORT CTreeNode :
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::ITreeNode
	{
		/**
		 * @brief Родительский узел
		 */
		ITreeNode* m_parent;

		/**
		 * @brief Предыдущий узел
		 */
		ITreeNode* m_prev;

		/**
		 * @brief Следующий узел
		 */
		ITreeNode* m_next;

		/**
		 * @brief Первый подузел
		 */
		ITreeNode* m_first_child;

		/**
		 * @brief Последний подузел
		 */
		ITreeNode* m_last_child;

	public:
		CTreeNode ();
		~CTreeNode ();

		/**
		 * @name Реализация интерфейса OAF::ITreeNode
		 */
		/** @{*/
		void setParent (OAF::ITreeNode* _parent);
		void setPreviousSibling (OAF::ITreeNode* _prev);
		void setNextSibling (OAF::ITreeNode* _next);
		bool parentIs (OAF::ITreeNode* _parent);
		bool previousSiblingIs (OAF::ITreeNode* _prev);
		bool nextSiblingIs (OAF::ITreeNode* _next);
		OAF::URef<OAF::ITreeNode> parent ();
		OAF::URef<OAF::ITreeNode> previousSibling ();
		OAF::URef<OAF::ITreeNode> nextSibling ();
		OAF::URef<OAF::ITreeNode> firstChild ();
		OAF::URef<OAF::ITreeNode> lastChild ();
		OAF::URef<OAF::ITreeNode> insertBefore (OAF::ITreeNode* _new, OAF::ITreeNode* _ref, OAF::INotifyListener* _origin = NULL);
		OAF::URef<OAF::ITreeNode> insertAfter (OAF::ITreeNode* _new, OAF::ITreeNode* _ref, OAF::INotifyListener* _origin = NULL);
		OAF::URef<OAF::ITreeNode> appendChild (OAF::ITreeNode* _new, OAF::INotifyListener* _origin = NULL);
		OAF::URef<OAF::ITreeNode> removeChild (OAF::ITreeNode* _old, OAF::INotifyListener* _origin = NULL);
		OAF::URef<OAF::ITreeNode> replaceChild (OAF::ITreeNode* _new, OAF::ITreeNode* _old, OAF::INotifyListener* _origin = NULL);
		/** @}*/

		/**
		 * @brief Корень дерева, в которое включен узел
		 */
		static OAF::URef<OAF::ITreeNode> root (OAF::ITreeNode* _n);

		/**
		 * @brief Количество дочерних узлов, соответствующих заданному условию
		 *
		 * @param[in] _n узел, для которого подсчитывается число дочерних узлов
		 * @param[in] _f условие, которому должен соответствовать узлы
		 * @param[in] _a дополнительные аргументы условия
		 */
		static int childCount (OAF::ITreeNode* _n, bool (*_f) (OAF::ITreeNode*, const void*) = NULL, const void* _a = NULL);

		/**
		 * @brief Номер узла в списке дочерних узлов, соответствующих заданному условию
		 *
		 * @param[in] _n узел, для которого считается индекс
		 * @param[in] _f условие, которому должны соответствовать узлы
		 * @param[in] _a дополнительные аргументы условия
		 * @return индекс заданного узла или -1, если узел не соответствует заданному условию
		 */
		static int row4child (OAF::ITreeNode* _n, bool (*_f) (OAF::ITreeNode*, const void*) = NULL, const void* _a = NULL);

		/**
		 * @brief Узел по индексу в списке дочерних узлов, соответствующих заданному условию
		 *
		 * @param[in] _n узел, дочерний узел которого ищется по заданному индексу
		 * @param[in] _f условие, которому должны соответствовать узлы
		 * @param[in] _a дополнительные аргументы условия
		 */
		static OAF::URef<OAF::ITreeNode> child4row (OAF::ITreeNode* _n, int _r, bool (*_f) (OAF::ITreeNode*, const void*) = NULL, const void* _a = NULL);

		/**
		 * @brief Самый глубоко вложенный первый подузел
		*/
		static OAF::URef<OAF::ITreeNode> firstChildNode (OAF::ITreeNode* _node);

		/**
		 * @brief Самый глубоко вложенный последний подузел
		 */
		static OAF::URef<OAF::ITreeNode> lastChildNode (OAF::ITreeNode* _node);

		/**
		 * @brief Следующий узел дерева в PREORDER-порядке его обхода
		 *
		 * @param[in] _root задает корневой узел, относительно которого осуществляется обход.
		 *                  Если задан @c NULL, то обход осуществляется относительно корня
		 *                  всего дерева.
		 * @param[in] _deep показывает, нужно ли заходить внутрь предшествующего узла
		 */
		static OAF::URef<OAF::ITreeNode> nextNode (OAF::ITreeNode* _node, OAF::ITreeNode* _root = NULL, bool _deep = true);

		/**
		 * @brief Предыдущий узел дерева в PREORDER-порядке его обхода
		 *
		 * @param[in] _root задает корневой узел, относительно которого осуществляется обход.
		 *                  Если задан @c NULL, то обход осуществляется относительно корня
		 *                  всего дерева.
		 * @param[in] _deep показывает, нужно ли заходить внутрь предшествующего узла
		 */
		static OAF::URef<OAF::ITreeNode> previousNode (OAF::ITreeNode* _node, OAF::ITreeNode* _root = NULL, bool _deep = true);

		/**
		 * @brief Поиск ближайшего общего контейнерного узла
		 */
		static OAF::URef<OAF::ITreeNode> commonAncestorContainer (OAF::ITreeNode* _a, OAF::ITreeNode* _b);

		/**
		 * @brief Сравнение порядка следования двух узлов (в PREORDER порядке обхода дерева)
		 */
		static int compare (OAF::ITreeNode* _a, OAF::ITreeNode* _b);
	};

	/**
	 * @brief Реализация узла DOM-подобного дерева с рассылкой уведомлений об изменении данных
	 *
	 * В качестве уведомления рассылаются уведомления OAF::IObjectNotify.
	 */
	class OAFSTD_EXPORT CNotifyTreeNode :
		//
		// Импортируемые реализации
		//
		virtual public OAF::CNotifySource,
		virtual public CTreeNode,
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::INotifySource,
		virtual public OAF::ITreeNode
	{
	public:
		CNotifyTreeNode ();

		~CNotifyTreeNode ();

		/**
		 * @brief Переопределение CNotifySource::notifyListeners
		 */
		void notifyListeners (OAF::IInterface* _event, OAF::INotifyListener* _origin);

		/**
		 * @name Реализация методов интерфейса OAF::ITreeNode
		 */
		/** @{*/
		OAF::URef<OAF::ITreeNode> insertBefore (OAF::ITreeNode* _new, OAF::ITreeNode* _ref, OAF::INotifyListener* _origin = NULL);
		OAF::URef<OAF::ITreeNode> insertAfter (OAF::ITreeNode* _new, OAF::ITreeNode* _ref, OAF::INotifyListener* _origin = NULL);
		OAF::URef<OAF::ITreeNode> removeChild (OAF::ITreeNode* _old, OAF::INotifyListener* _origin = NULL);
		/** @}*/
	};

	/**
	 * @brief Интерфейс доступа к дереву подобъектов как к коллекции данных
	 *
	 * Предполагается, что все объекты поддерживают интерфейс OAF::IPropertyBag и имеют
	 * обязательный атрибут id типа QUuid.
	 */
	class OAFSTD_EXPORT CCollectionTreeNode :
		//
		// Импортируемые реализации
		//
		virtual public CNotifyTreeNode,
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::INotifySource,
		virtual public OAF::ITreeNode,
		virtual public OAF::IObjectCollection
	{
		/**
		 * @brief Индекс по идентификаторам всех объектов коллекции
		 *
		 * В данном индексе храним только указатели на объекты. Все ссылки держим исключительно
		 * в дереве. В индексе регистрируются все подобъекты данного узла. Это индекс нужен
		 * для реализации интерфейса OAF::IObjectCollection.
		 */
		QMap<QUuid, QSet<OAF::ITreeNode*> > m_index;

		/**
		 * @brief Добавить объект в индекс
		 */
		void append (const QUuid& _id, OAF::ITreeNode* _n);

		/**
		 * @brief Удалить объект из индекса
		 */
		void remove (const QUuid& _id, OAF::ITreeNode* _n);

	public:
		CCollectionTreeNode ();
		~CCollectionTreeNode ();

		/**
		 * @brief Переопределение функции CNotifySource::notifyListeners
		 *
		 * Реализуем обновление индекса для всех дочерних объектов набора. Поскольку рассылка
		 * уведомлений идёт по всей цепочке родителей, то обновляются индексы всех родителей.
		 */
		void notifyListeners (OAF::IInterface* _event, OAF::INotifyListener* _origin);

		/**
		 * @name Реализация интерфейса OAF::IObjectCollection
		 */
		/** @{*/
		std::size_t enumObjects (QStringList& _out, const QString& _params, std::size_t _loaded);
		OAF::URef<OAF::IUnknown> getObject (const QString& _oid);
		/** @}*/
	};

	/**
	 * @brief прокси-объект для многократной вставки в дерево данных различных модулей
	 * одного и того же узла DOM-подобного дерева
	 */
	class OAFSTD_EXPORT CCollectionTreeNodeProxy :
		//
		// Импортируемые реализации
		//
		virtual public CCollectionTreeNode,
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IUnknownExtender,
		virtual public OAF::IUnknownProxy,
		virtual public OAF::INotifySource,
		virtual public OAF::ITreeNode,
		virtual public OAF::IObjectCollection,
		//
		// Внутренний интерфейс
		//
		virtual public OAF::INotifyListener
	{
	protected:
		/**
		 * @brief Объект для проксирования
		 */
		OAF::URef<OAF::ITreeNode> m_object;

		/**
		 * @name Реализация интерфейса OAF::INotifyListener
		 */
		/** @{*/
		void notify (OAF::IInterface* _event, OAF::INotifySource* _source, OAF::INotifyListener* _origin);
		/** @}*/

	public:
		CCollectionTreeNodeProxy ();
		~CCollectionTreeNodeProxy ();

		/**
		 * @name Реализация интерфейса OAF::IUnknownExtender
		 */
		/** @{*/
		OAF::URef<OAF::IUnknown> setExtendedObject (OAF::IUnknown* _o);
		/** @}*/

		/**
		 * @name Реализация интерфейса OAF::IUnknownProxy
		 */
		/** @{*/
		OAF::URef<OAF::IUnknown> getExtendedObject ();
		/** @}*/

		/**
		 * @name Частичная реализация интерфейса OAF::ITreeNode (бОльшую часть работы делает CTreeNode)
		 */
		/** @{*/
		OAF::URef<OAF::ITreeNode> insertBefore (OAF::ITreeNode* _new, OAF::ITreeNode* _ref, OAF::INotifyListener* _origin = NULL);
		OAF::URef<OAF::ITreeNode> insertAfter (OAF::ITreeNode* _new, OAF::ITreeNode* _ref, OAF::INotifyListener* _origin = NULL);
		OAF::URef<OAF::ITreeNode> removeChild (OAF::ITreeNode* _old, OAF::INotifyListener* _origin = NULL);
		/** @}*/
	};
}

#endif /* __CTREE_NODE_H */
