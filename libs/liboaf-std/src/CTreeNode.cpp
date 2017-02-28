/**
 * @file
 * @brief Реализация узла DOM-подобного дерева
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <idl/IObjectSource.h>

#include <OAF/Helpers.h>
#include <OAF/CTreeNode.h>

OAF::CTreeNode::CTreeNode () : m_parent (NULL), m_prev (NULL), m_next (NULL), m_first_child (NULL), m_last_child (NULL)
{}

OAF::CTreeNode::~CTreeNode ()
{
	for (OAF::URef<OAF::ITreeNode> n = m_first_child; n;)
	{
		OAF::URef<OAF::ITreeNode> r = n;
		n = r->nextSibling ();

		r->setParent (NULL);
		r->setPreviousSibling (NULL);
		r->setNextSibling (NULL);

		r->unref ();
	}

	m_first_child = NULL;
	m_last_child  = NULL;
}

void
OAF::CTreeNode::setParent (OAF::ITreeNode* _parent)
{
	m_parent = _parent;
}

void
OAF::CTreeNode::setPreviousSibling (OAF::ITreeNode* _prev)
{
	m_prev = _prev;
}

void
OAF::CTreeNode::setNextSibling (OAF::ITreeNode* _next)
{
	m_next = _next;
}

bool
OAF::CTreeNode::parentIs (OAF::ITreeNode* _parent)
{
	return m_parent == _parent;
}

bool
OAF::CTreeNode::previousSiblingIs (OAF::ITreeNode* _prev)
{
	return m_prev == _prev;
}

bool
OAF::CTreeNode::nextSiblingIs (OAF::ITreeNode* _next)
{
	return m_next == _next;
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::parent ()
{
	return m_parent;
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::previousSibling ()
{
	return m_prev;
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::nextSibling ()
{
	return m_next;
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::firstChild ()
{
	return m_first_child;
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::lastChild ()
{
	return m_last_child;
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::insertBefore (OAF::ITreeNode* _new, OAF::ITreeNode* _ref, OAF::INotifyListener* _origin)
{
	Q_UNUSED (_origin);

	//
	// Вставляемый узел должен существовать
	//
	Q_ASSERT (_new != NULL);
	//
	// Узел, который выбран для ссылки, должен быть дочерним узлом
	//
	Q_ASSERT ((_ref == NULL) || (_ref->parentIs (this)));

	//
	// Захватываем ссылку на вставляемый узел
	//
	OAF::URef<OAF::ITreeNode> new_node = _new;

	//
	// Если вставляемый узел является прокси-объектом, то заменяем его на
	// оригинальный узел
	//
	if (OAF::URef<OAF::IUnknownProxy> new_node_p = new_node.queryInterface<OAF::IUnknownProxy> ())
		new_node = new_node_p->getExtendedObject ().queryInterface<OAF::ITreeNode> ();

	//
	// Если узел, выбранный для вставки является частью другого дерева, то вместо него
	// вставляем соответствующий прокси-объект
	//
	if (!new_node->parentIs (NULL))
	{
		//
		// Проверяем циклические ссылки. Если обнаружена циклическая ссылка, то
		// ничего не вставляем
		//
		for (OAF::URef<OAF::ITreeNode> p = new_node->parent (); p; p = p->parent ())
		{
			if (p.is (new_node))
			{
				qWarning ("%s: Can't insert node as child of itself", Q_FUNC_INFO);
				return NULL;
			}
		}

		//
		// Пытаемся создать прокси-объект
		//
		if (OAF::URef<OAF::IUnknownProxy> px = OAF::createFromObject<OAF::IUnknownProxy> (new_node))
			new_node = px.queryInterface<OAF::ITreeNode> ();
		else
			new_node = NULL;

		//
		// Если прокси-объект не создан, то ничего не вставляем
		//
		if (new_node.isNull ())
		{
			qWarning ("%s: can't create proxy object of %s. Nothing to insert!", Q_FUNC_INFO, qPrintable (_new->cid ()));
			return NULL;
		}
	}

	if (_ref)
	{
		OAF::URef<OAF::ITreeNode> prev = _ref->previousSibling ();

		if (prev)
			prev->setNextSibling (new_node);
		else
			m_first_child = new_node.ptr ();

		new_node->setPreviousSibling (prev);
		new_node->setNextSibling (_ref);
		_ref->setPreviousSibling (new_node);
	}
	else
	{
		if (m_first_child)
		{
			m_first_child->setPreviousSibling (new_node);
			new_node->setNextSibling (m_first_child);
		}
		else
			m_last_child = new_node.ptr ();

		m_first_child = new_node.ptr ();
	}

	new_node->setParent (this);

	//
	// Храним в дереве ссылки на объекты
	//
	new_node->ref ();

	return new_node;
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::insertAfter (OAF::ITreeNode* _new, OAF::ITreeNode* _ref, OAF::INotifyListener* _origin)
{
	Q_UNUSED (_origin);

	//
	// Вставляемый узел должен существовать
	//
	Q_ASSERT (_new != NULL);
	//
	// Узел, который выбран для ссылки, должен быть дочерним узлом
	//
	Q_ASSERT ((_ref == NULL) || (_ref->parentIs (this)));

	//
	// Захватываем ссылку на вставляемый узел
	//
	OAF::URef<OAF::ITreeNode> new_node = _new;

	//
	// Если вставляемый узел является прокси-объектом, то заменяем его на
	// оригинальный узел
	//
	if (OAF::URef<OAF::IUnknownProxy> new_node_p = new_node.queryInterface<OAF::IUnknownProxy> ())
		new_node = new_node_p->getExtendedObject ().queryInterface<OAF::ITreeNode> ();

	//
	// Если узел, выбранный для вставки является частью другого дерева, то вместо него
	// вставляем соответствующий прокси-объект
	//
	if (!new_node->parentIs (NULL))
	{
		//
		// Проверяем циклические ссылки. Если обнаружена циклическая ссылка, то
		// ничего не вставляем
		//
		for (OAF::URef<OAF::ITreeNode> p = new_node->parent (); p; p = p->parent ())
		{
			if (p.is (new_node))
			{
				qWarning ("%s: Can't insert node as child of itself", Q_FUNC_INFO);
				return NULL;
			}
		}

		//
		// Пытаемся создать прокси-объект
		//
		if (OAF::URef<OAF::IUnknownProxy> px = OAF::createFromObject<OAF::IUnknownProxy> (new_node))
			new_node = px.queryInterface<OAF::ITreeNode> ();
		else
			new_node = NULL;

		//
		// Если прокси-объект не создан, то ничего не вставляем
		//
		if (new_node.isNull ())
		{
			qWarning ("%s: can't create proxy object of %s. Nothing to insert!", Q_FUNC_INFO, qPrintable (_new->cid ()));
			return NULL;
		}
	}

	if (_ref)
	{
		OAF::URef<OAF::ITreeNode> next = _ref->nextSibling ();

		if (next)
			next->setPreviousSibling (new_node);
		else
			m_last_child = new_node.ptr ();

		new_node->setNextSibling (next);
		new_node->setPreviousSibling (_ref);
		_ref->setNextSibling (new_node);
	}
	else
	{
		if (m_last_child)
		{
			m_last_child->setNextSibling (new_node);
			new_node->setPreviousSibling (m_last_child);
		}
		else
			m_first_child = new_node.ptr ();

		m_last_child = new_node.ptr ();
	}

	new_node->setParent (this);

	//
	// Храним в дереве ссылки на объекты
	//
	new_node->ref ();

	return new_node;
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::appendChild (OAF::ITreeNode* _new, OAF::INotifyListener* _origin)
{
	return insertAfter (_new, NULL, _origin);
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::removeChild (OAF::ITreeNode* _old, OAF::INotifyListener* _origin)
{
	Q_UNUSED (_origin);

	//
	// Удаляемый узел должен существовать
	//
	Q_ASSERT (_old != NULL);
	//
	// Удаляемый узел должен быть частью дерева
	//
	Q_ASSERT (_old->parentIs (this));

	OAF::URef<OAF::ITreeNode> prev = _old->previousSibling ();
	OAF::URef<OAF::ITreeNode> next = _old->nextSibling ();

	if (prev)
		prev->setNextSibling (next);
	else
		m_first_child = next.ptr ();

	if (next)
		next->setPreviousSibling (prev);
	else
		m_last_child = prev.ptr ();

	_old->setParent (NULL);
	_old->setPreviousSibling (NULL);
	_old->setNextSibling (NULL);

	//
	// Сохраняем ссылку на объект, так как возможно в дереве хранится единственная ссылка
	// и после её освобождения нечего будет возвращать
	//
	OAF::URef<OAF::ITreeNode> old = _old;

	//
	// Освобождаем ссылку на объект, хранящуюся в дереве
	//
	_old->unref ();

	return old;
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::replaceChild (OAF::ITreeNode* _new, OAF::ITreeNode* _old, OAF::INotifyListener* _origin)
{
	insertBefore (_new, _old, _origin);

	return removeChild (_old, _origin);
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::root (OAF::ITreeNode* _n)
{
	OAF::URef<OAF::ITreeNode> n = _n;
	while (n)
	{
		OAF::URef<OAF::ITreeNode> nn = n->parent ();
		if (!nn)
			return n;
		n = nn;
	}

	return NULL;
}

int
OAF::CTreeNode::childCount (OAF::ITreeNode* _n, bool (*_f) (OAF::ITreeNode*, const void*), const void* _a)
{
	int c = 0;

	//
	// Если задан корневой узел
	//
	if (_n)
	{
		//
		// Для всех дочерних узлов
		//
		for (OAF::URef<OAF::ITreeNode> n = _n->firstChild (); n; n = n->nextSibling ())
		{
			//
			// Если условие не задано или узел ему соответствует, увеличиваем счётчик узлов
			//
			if (!_f || _f (n, _a))
				++c;
		}
	}

	return c;
}

int
OAF::CTreeNode::row4child (OAF::ITreeNode *_n, bool (*_f) (OAF::ITreeNode*, const void*), const void* _a)
{
	//
	// Если заданный узел не соответствует условию, то его индекс -1
	//
	int r = -1;

	//
	// Проверяем, что заданный узел входит в число узлов, соответствующих условию
	//
	if (_n && (!_f || _f (_n, _a)))
	{
		//
		// Если узел соответствует заданному условию, то он как минимум имеет индекс 0
		//
		r = 0;

		//
		// По всем узлам, которые находятся до заданного узла
		//
		for (OAF::URef<OAF::ITreeNode> n = _n->previousSibling (); n; n = n->previousSibling ())
		{
			//
			// Если условие не задано или узел ему соответствует, увеличиваем значение индекса
			//
			if (!_f || _f (n, _a))
				++r;
		}
	}

	return r;
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::child4row (OAF::ITreeNode* _n, int _r, bool (*_f) (OAF::ITreeNode*, const void*), const void* _a)
{
	//
	// Если задан корневой узел
	//
	if (_n)
	{
		//
		// Начальный индекс
		//
		int r = 0;

		//
		// Для всех дочерних узлов заданного корневого узла
		//
		for (OAF::URef<OAF::ITreeNode> n = _n->firstChild (); n; n = n->nextSibling ())
		{
			//
			// Если условие не задано или узел ему соответствует
			//
			if (!_f || _f (n, _a))
			{
				//
				// Если найден узел, соответствующий заданному индексу
				//
				if (r == _r)
					return n;

				++r;
			}
		}
	}

	return NULL;
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::firstChildNode (OAF::ITreeNode* _node)
{
	//
	// Выполняется проход по всем первым дочерним подузлам начиная от текущего до тех пор,
	// пока не будет найден узел, не имеющий дочерних узлов.
	//
	OAF::URef<OAF::ITreeNode> first = _node;

	while (OAF::URef<OAF::ITreeNode> n = first->firstChild ())
		first = n;

	return first;
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::lastChildNode (OAF::ITreeNode* _node)
{
	//
	// Выполняется проход по всем последним дочерним подузлам начиная от текущего до тех пор,
	// пока не будет найден узел, не имеющий дочерних узлов.
	//
	OAF::URef<OAF::ITreeNode> last = _node;

	while (OAF::URef<OAF::ITreeNode> n = last->lastChild ())
		last = n;

	return last;
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::nextNode (OAF::ITreeNode* _node, OAF::ITreeNode* _root, bool _deep)
{
	if (_deep)
	{
		if (OAF::URef<OAF::ITreeNode> n = _node->firstChild ())
			return n;
	}

	if (_node == _root)
		return NULL;

	if (OAF::URef<OAF::ITreeNode> n = _node->nextSibling ())
		return n;

	//
	// Ищем следующего соседа не выходя за рамки заданного корневого узла
	//
	OAF::URef<OAF::ITreeNode> node = _node->parent ();
	OAF::URef<OAF::ITreeNode> next = node->nextSibling ();
	while (node && !next && !node.is (_root))
	{
		node = node->parent ();
		next = node->nextSibling ();
	}

	if (node && !node.is (_root))
		return next;

	return NULL;
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::previousNode (OAF::ITreeNode* _node, OAF::ITreeNode* _root, bool _deep)
{
	if (_node == _root)
		return NULL;

	if (OAF::URef<OAF::ITreeNode> n = _node->previousSibling ())
	{
		if (_deep)
			return lastChildNode (n);
		return n;
	}

	return _node->parent ();
}

OAF::URef<OAF::ITreeNode>
OAF::CTreeNode::commonAncestorContainer (OAF::ITreeNode* _a, OAF::ITreeNode* _b)
{
	OAF::URef<OAF::ITreeNode> temp_a;
	OAF::URef<OAF::ITreeNode> temp_b;
	unsigned int deep_a = 0;
	unsigned int deep_b = 0;
	unsigned int deep = 0;

	//
	// Тривиальный случай, если объекты равны
	//
	if (_a == _b)
		return _a;

	//
	// Для начала проходим по дереву вверх вычисляя глубину погружения объектов
	// и заодно проверяя другой тривиальный случай, когда один из сравниваемых
	// объектов является родителем другого и если да, то этот объект и есть
	// наименьший общий контейнер.
	//
	for (temp_a = _a; temp_a && !temp_a.is (_b); temp_a = temp_a->parent ())
		++deep_a;

	if (temp_a.is (_b))
		return _b;

	for (temp_b = _b; temp_b && !temp_b.is (_a); temp_b = temp_b->parent ())
		++deep_b;

	if (temp_b.is (_a))
		return _a;

	//
	// Вычисляем наименьшую глубину вложения объектов
	//
	deep = (deep_a < deep_b) ? deep_a : deep_b;
	deep_a -= deep;
	deep_b -= deep;

	//
	// Для более глубоко вложенного объекта определяем родителя,
	// глубина вложения которого находится на уровне другого
	// сравниваемого объекта (так как наименьший общий контейнер
	// не может по определению находится глубже одного из сравниваемых
	// объектов).
	//
	for (temp_a = _a; deep_a; deep_a--)
		temp_a = temp_a->parent ();
	for (temp_b = _b; deep_b; deep_b--)
		temp_b = temp_b->parent ();

	//
	// Начинаем подъем с глубины дерева сравнивая объекты до тех пор,
	// пока либо не закончится дерево (что означает, что у объектов нет
	// общего контейнера), либо пока не будет найден объект, содержащий
	// одновременно оба объекта. Первый такой объект и есть наименьший
	// общий контейнер.
	//
	while (temp_a && temp_b)
	{
		if (temp_a == temp_b)
			return temp_a;

		temp_a = temp_a->parent ();
		temp_b = temp_b->parent ();
	}

	//
	// Общего контейнера нет
	//
	return NULL;
}

int
OAF::CTreeNode::compare (OAF::ITreeNode* _a, OAF::ITreeNode* _b)
{
	if (_a == _b)
		return 0;

	OAF::URef<OAF::ITreeNode> cac = commonAncestorContainer (_a, _b);

	//
	// Дочерний узел в cac, который содержит _a
	//
	OAF::URef<OAF::ITreeNode> a = _a;
	while (a->parent () && (a->parent () != cac))
		a = a->parent ();

	//
	// Дочерний узел в cac, который содержит _b
	//
	OAF::URef<OAF::ITreeNode> b = _b;
	while (b->parent () && (b->parent () != cac))
		b = b->parent ();

	//
	// Начинаем двигаться от первого потомка наименьшего общего контейнера,
	// пока не встретим один из ранее найденых дочерних узлов. Объект,
	// родителя которого мы таким образом определили и является более
	// "ранним" с точки зрения PREORDER обхода дерева, и следовательно -
	// "меньшим" объектом
	//
	if (a->parent () && b->parent ())
	{
		for (OAF::URef<OAF::ITreeNode> t = a->parent ()->firstChild (); t; t = t->nextSibling ())
		{
			if (t == a)
				return -1;
			else if (t == b)
				return 1;
		}
	}

	//
	// Узлы сравнить невозможно
	//
	return 0;
}

OAF::CNotifyTreeNode::CNotifyTreeNode ()
{}

OAF::CNotifyTreeNode::~CNotifyTreeNode ()
{}

void
OAF::CNotifyTreeNode::notifyListeners (OAF::IInterface* _event, OAF::INotifyListener* _origin)
{
	//
	// Рассылка уведомлений подписчикам данного узла
	//
	OAF::CNotifySource::notifyListeners (_event, _origin);

	//
	// Рассылка уведомлений подписчикам вверх по дереву
	//
	if (OAF::URef<OAF::CNotifyTreeNode> n = parent ().queryInterface<OAF::CNotifyTreeNode> ())
		n->notifyListeners (_event, _origin);
}

OAF::URef<OAF::ITreeNode>
OAF::CNotifyTreeNode::insertBefore (OAF::ITreeNode* _new, OAF::ITreeNode* _ref, OAF::INotifyListener* _origin)
{
	try
	{
		//
		// Рассылка уведомления перед добавлением узла
		//
		OAF::IObjectNotify bev (OAF::IObjectNotify::BEFORE, OAF::IObjectNotify::APPEND, _new);
		notifyListeners (&bev, _origin);
	}
	//
	// Если кто-то из слушателей сказал, что добавление невозможно
	//
	catch (const OAF::IObjectNotify::Cancel&)
	{
		//
		// Рассылаем уведомление об отмене добавления узла
		//
		OAF::IObjectNotify cev (OAF::IObjectNotify::CANCELLED, OAF::IObjectNotify::APPEND, _new);
		notifyListeners (&cev, _origin);

		return NULL;
	}

	//
	// Собственно добавляем узел
	//
	OAF::URef<OAF::ITreeNode> node = OAF::CTreeNode::insertBefore (_new, _ref, _origin);

	//
	// Рассылка уведомления после добавления узла
	//
	{
		OAF::IObjectNotify aev (OAF::IObjectNotify::AFTER, OAF::IObjectNotify::APPEND, node);
		notifyListeners (&aev, _origin);
	}

	return node;
}

OAF::URef<OAF::ITreeNode>
OAF::CNotifyTreeNode::insertAfter (OAF::ITreeNode* _new, OAF::ITreeNode* _ref, OAF::INotifyListener* _origin)
{
	try
	{
		//
		// Рассылка уведомления перед добавлением узла
		//
		OAF::IObjectNotify bev (OAF::IObjectNotify::BEFORE, OAF::IObjectNotify::APPEND, _new);
		notifyListeners (&bev, _origin);
	}
	//
	// Если кто-то из слушателей сказал, что добавление невозможно
	//
	catch (const OAF::IObjectNotify::Cancel&)
	{
		//
		// Рассылаем уведомление об отмене добавления
		//
		OAF::IObjectNotify cev (OAF::IObjectNotify::CANCELLED, OAF::IObjectNotify::APPEND, _new);
		notifyListeners (&cev,_origin);

		return NULL;
	}

	//
	// Собственно добавляем узел
	//
	OAF::URef<OAF::ITreeNode> node = OAF::CTreeNode::insertAfter (_new, _ref, _origin);

	//
	// Рассылка уведомления после добавления узла
	//
	{
		OAF::IObjectNotify aev (OAF::IObjectNotify::AFTER, OAF::IObjectNotify::APPEND, node);
		notifyListeners (&aev, _origin);
	}

	return node;
}

OAF::URef<OAF::ITreeNode>
OAF::CNotifyTreeNode::removeChild (OAF::ITreeNode* _old, OAF::INotifyListener* _origin)
{
	try
	{
		//
		// Рассылка уведомления перед удалением узла
		//
		OAF::IObjectNotify bev (OAF::IObjectNotify::BEFORE, OAF::IObjectNotify::REMOVE, _old);
		notifyListeners (&bev, _origin);
	}
	//
	// Если кто-то сказал, что удаление невозможно
	//
	catch (const OAF::IObjectNotify::Cancel&)
	{
		//
		// Рассылаем уведомление об отмене удаления
		//
		OAF::IObjectNotify cev (OAF::IObjectNotify::CANCELLED, OAF::IObjectNotify::REMOVE, _old);
		notifyListeners (&cev, _origin);

		return NULL;
	}

	//
	// Собственно удаляем узел
	//
	OAF::URef<OAF::ITreeNode> node = OAF::CTreeNode::removeChild (_old, _origin);

	//
	// Рассылка уведомления после удаления узла
	//
	{
		OAF::IObjectNotify aev (OAF::IObjectNotify::AFTER, OAF::IObjectNotify::REMOVE, node.ptr ());
		notifyListeners (&aev, _origin);
	}

	return node;
}

void
OAF::CCollectionTreeNode::append (const QUuid& _id, OAF::ITreeNode* _n)
{
	QSet<OAF::ITreeNode*>& v = m_index[_id];
	v.insert (_n);
}

void
OAF::CCollectionTreeNode::remove (const QUuid& _id, OAF::ITreeNode* _n)
{
	//
	// Если найден соответствующий элемент в индексе
	//
	QMap<QUuid, QSet<OAF::ITreeNode*> >::iterator n = m_index.find (_id);
	if (n != m_index.end ())
	{
		n.value ().remove (_n);
		if (n.value ().isEmpty ())
			m_index.remove (_id);
	}
}

OAF::CCollectionTreeNode::CCollectionTreeNode ()
{}

OAF::CCollectionTreeNode::~CCollectionTreeNode ()
{
	m_index.clear ();
}

void
OAF::CCollectionTreeNode::notifyListeners (OAF::IInterface* _event, OAF::INotifyListener* _origin)
{
	//
	// Перестраиваем индекс в ответ на уведомление об изменении идентификатора узла
	//
	if (OAF::IPropertyNotify* pev = OAF::queryInterface<OAF::IPropertyNotify> (_event))
	{
		//
		// При изменении идентификатора
		//
		if (pev->values.contains ("id"))
		{
			//
			// До изменения удаляем узел со старым идентификатором из индекса
			//
			if (pev->when == OAF::IPropertyNotify::BEFORE)
				remove (pev->values["id"].value<QUuid> (), OAF::queryInterface<OAF::ITreeNode> (pev->object));
			//
			// При отмене изменений или при их фиксации индексируем объект
			//
			else if ((pev->when == OAF::IPropertyNotify::CANCELLED) || (pev->when == OAF::IPropertyNotify::AFTER))
				append (pev->values["id"].value<QUuid> (), OAF::queryInterface<OAF::ITreeNode> (pev->object));
		}
	}

	//
	// Перестраиваем индекс в ответ на уведомление о добавлении/удалении узла
	//
	else if (OAF::IObjectNotify* nev = OAF::queryInterface<OAF::IObjectNotify> (_event))
	{
		//
		// Если это уведомление после добавления узла
		//
		if ((nev->when == OAF::IObjectNotify::AFTER) && (nev->action == OAF::IObjectNotify::APPEND))
		{
			//
			// Проверяем что узел поддерживает интерфейс OAF::ITreeNode
			//
			if (OAF::URef<OAF::ITreeNode> object = OAF::queryInterface<OAF::ITreeNode> (nev->object))
			{
				//
				// Проверяем что узел поддерживает интерфейс IPeopretyBag
				//
				if (OAF::URef<OAF::IPropertyBag> po = object.queryInterface<OAF::IPropertyBag> ())
				{
					//
					// Если для узла определён идентификатор, то добавляем его в индекс
					//
					if (po->isDefined ("id"))
						append (po->getValue ("id").value<QUuid> (), object);

					//
					// Если сотояние объекта равно 0 == STATE_OK (поддержка возможности реализации отложенной
					// загрузки объектов). При загрузке объекта у него автоматически поменяется
					// идентификатор и он будет проиндексирован вместе с подобъектами, которые
					// будут вставляться при загрузке.
					//
					if (po->isDefined ("state") && (po->getValue ("state").toInt () == 0))
					{
						//
						// Добавляем все узлы поддерева в индекс
						//
						for (OAF::URef<OAF::ITreeNode> n = object->firstChild (); n; n = OAF::CTreeNode::nextNode (n, object))
						{
							//
							// Проверяем что узел поддерживает интерфейс OAF::IPeopretyBag
							//
							if (OAF::URef<OAF::IPropertyBag> p = n.queryInterface<OAF::IPropertyBag> ())
							{
								//
								// Если для узла определён идентификатор, то добавляем его в индекс
								//
								if (p->isDefined ("id"))
									append (p->getValue ("id").value<QUuid> (), n);
							}
						}
					}
				}
			}
		}

		//
		// Иначе если это уведомление после удаления узла (используем тот факт, что реально
		// узел удаляется только после оповещения, а нам нужны только его атрибуты и его
		// подузлы, поэтому является ли он частью общего дерева или уже нет нам уже не
		// важно)
		//
		else if ((nev->when == OAF::IObjectNotify::AFTER) && (nev->action == OAF::IObjectNotify::REMOVE))
		{
			//
			// Проверяем что узел поддерживает интерфейс OAF::ITreeNode
			//
			if (OAF::URef<OAF::ITreeNode> object = OAF::queryInterface<OAF::ITreeNode> (nev->object))
			{
				//
				// Проверяем что узел поддерживает интерфейс OAF::IPropretyBag
				//
				if (OAF::URef<OAF::IPropertyBag> po = object.queryInterface<OAF::IPropertyBag> ())
				{
					//
					// Если для узла определён идентификатор, то удаляем его из индекса
					//
					if (po->isDefined ("id"))
						remove (po->getValue ("id").value<QUuid> (), object);

					//
					// Если сотояние объекта равно 0 == STATE_OK (поддержка возможности реализации отложенной
					// загрузки объектов). В ином случае объект вообще не добавлялся в индекс,
					// следовательно и удалять его не надо
					//
					if (po->isDefined ("state") && (po->getValue ("state").toInt () == 0))
					{
						//
						// Удаляем все узлы поддерева из индекса
						//
						for (OAF::URef<OAF::ITreeNode> n = object; n; n = OAF::CTreeNode::nextNode (n, object))
						{
							//
							// Проверяем что узел поддерживает интерфейс IPeopretyBag
							//
							if (OAF::URef<OAF::IPropertyBag> p = n.queryInterface<OAF::IPropertyBag> ())
							{
								//
								// Если для узла определён идентификатор, то удаляем его из индекса
								//
								if (p->isDefined ("id"))
									remove (p->getValue ("id").value<QUuid> (), n);
							}
						}
					}
				}
			}
		}
	}

	//
	// Рассылка уведомлений по дереву выполняется после изменения индекса
	//
	OAF::CNotifyTreeNode::notifyListeners (_event, _origin);
}

std::size_t
OAF::CCollectionTreeNode::enumObjects (QStringList& _out, const QString& _params, std::size_t _loaded)
{
	Q_UNUSED (_params);
	Q_UNUSED (_loaded);

	//
	// FIXME: реализация отбора объектов по запросу
	//

	//
	// Возвращаем список идентификаторов всех подобъектов
	//
	for (QMap<QUuid, QSet<OAF::ITreeNode*> >::iterator n = m_index.begin (); n != m_index.end (); ++n)
		_out.append (n.key ().toString ());

	return _out.size ();
}

OAF::URef<OAF::IUnknown>
OAF::CCollectionTreeNode::getObject (const QString& _oid)
{
	//
	// Разбираем параметр как QUuid в строковой представлении
	//
	QUuid id (_oid);

	//
	// Если идентификатор успешно разобран
	//
	if (!id.isNull ())
	{
		//
		// Если найден соответствующий элемент в индексе
		//
		QMap<QUuid, QSet<OAF::ITreeNode*> >::const_iterator n = m_index.find (id);

		//
		// Если объекты с таким индексом есть в дереве
		//
		if ((n != m_index.end ()) && !n.value ().isEmpty ())
		{
			//
			// Ищем объект, который не является прокси объектом
			//
			for (QSet<OAF::ITreeNode*>::const_iterator nn = n.value ().begin (); nn != n.value ().end (); ++nn)
			{
				//
				// Если такой объект найден, то его и возвращаем
				//
				if (!OAF::queryInterface<OAF::IUnknownProxy> (*nn))
					return *nn;
			}

			//
			// Иначе возвращаем первый попавшийся
			//
			return *n.value ().begin ();
		}
	}

	//
	// Если объект не найден
	//
	return NULL;
}

void
OAF::CCollectionTreeNodeProxy::notify (OAF::IInterface* _event, OAF::INotifySource* _source, OAF::INotifyListener* _origin)
{
	Q_UNUSED (_source);

	//
	// В данную функцию уведомления могут придти только от проксируемого
	// узла поскольку уведомления об изменении дерева узлов для прокси
	// рассылаются через переопределённую notifyListeners. Соответственно
	// уведомления от проксируемого узла больше ни для чего не используются
	// и дальше не рассылаются.
	//
	// Уведомления об изменении дерева прокси-узла рассылаются при использовании
	// наследуемых функций модификации дерева и специальной обработки не
	// требуют. Перестроение индекса коллекции осуществляется через них же
	// при отработке наследуемой функции notifyListeners и так же не требует
	// дополнительного кода.
	//

	//
	// Свои собственные изменения в проксируемом узле не обрабатываем
	//
	if (_origin == (OAF::INotifyListener*)this)
		return;

	//
	// Обрабатываем добавление/удаление дочернего узла в проксируемый узел: нужно
	// зеркально отразить произошедшие изменения. Т.е. если в проксируемом узле
	// удалили подузел, то нужно удалить и соответствующий подузел в прокси-узле.
	// Если в проксируемый узел добавили подузел, то его же нужно добавить и
	// в прокси-узел в том же месте.
	//
	// При этом обрабатывать необходимость создания прокси для добавляемых узлов
	// не нужно. Это будет сделано автоматически в наследуемых функциях.
	//
	if (OAF::IObjectNotify* oev = OAF::queryInterface<OAF::IObjectNotify> (_event))
	{
		//
		// Добавленный/удалённый узел
		//
		if (OAF::URef<OAF::ITreeNode> n = OAF::queryInterface <OAF::ITreeNode> (oev->object))
		{
			//
			// Учитываем, что нам придут уведомления об удалении/добавлении подузлов проксируемого
			// узла для всех уровней вложенности, поэтому обрабатываем добавление/удаление только
			// для дочерних узлов проксируемого узла
			//
			if (m_object.is (n->parent ()))
			{
				//
				// Обрабатываем вставку/отмену удаления узла в/из проксируемого узла
				//
				if (((oev->action == OAF::IObjectNotify::APPEND) && (oev->when == OAF::IObjectNotify::AFTER)) ||
						((oev->action == OAF::IObjectNotify::REMOVE) && (oev->when == OAF::IObjectNotify::CANCELLED)))
				{
					//
					// Число узлов в прокси-узле
					//
					int child_count_p = childCount (this);

					//
					// Синхронизацию делаем только для случая, когда число узлов в прокси-узле меньше
					// числа узлов в проксируемом узле. Это позволит отсечь случаи, когда событие об
					// отмене удаления узла пришло в прокси-узел, в котором не было пред-обработки
					// удаления узла
					//
					if (child_count_p < childCount (m_object))
					{
						//
						// Определяем позицию добавленного узла в проксируемом узле
						//
						int idx_new = row4child (n);

						//
						// Если позиция вставки узла меньше числа узлов в прокси-узле, то определяем
						// дочерний узел прокси-узла перед которым необходимо вставить добавленный узел.
						// Иначе добавляем узел в конец списка дочерних узлов прокси-узла.
						//
						OAF::URef<OAF::ITreeNode> ref;
						if (idx_new < child_count_p)
							ref = child4row (this, idx_new);

						//
						// Добавляем узел к прокси-узлу перед заданным. Используем наследуемую
						// функцию добавления, так как нам нужно модифицировать только список
						// дочерних узлов прокси без модификации проксируемого узла.
						//
						// Если узел добавляется в конец списка узлов, то используем наследуемую
						// insertAfter, так как appendChild вызовет переопределённую для прокси-узла
						// insertAfter
						//
						if (ref)
							OAF::CCollectionTreeNode::insertBefore (n, ref, _origin);
						else
							OAF::CCollectionTreeNode::insertAfter (n, NULL, _origin);
					}
				}
				//
				// Обрабатываем удаление узла oev->object из проксируемого узла (выполняем
				// пре-обработку события, когда для узла ещё установлены все необходимые
				// связи)
				//
				else if ((oev->action == OAF::IObjectNotify::REMOVE) && (oev->when == OAF::IObjectNotify::BEFORE))
				{
					//
					// Определяем позицию удаляемого узла в проксируемом узле
					//
					int idx_old = row4child (n);

					//
					// Получаем подузел прокси, который сейчас находится по данному индексу
					// и удаляем его с помощью наследуемой функции, поскольку нам надо
					// только удалить узел из прокси-узла
					//
					if (OAF::URef<OAF::ITreeNode> old = child4row (this, idx_old))
						OAF::CCollectionTreeNode::removeChild (old, _origin);
				}
			}
		}
	}
}

OAF::CCollectionTreeNodeProxy::CCollectionTreeNodeProxy ()
{}

OAF::CCollectionTreeNodeProxy::~CCollectionTreeNodeProxy ()
{
	//
	// Отписываемся от прослушивания событий проксируемого объекта (если он был)
	//
	OAF::unsubscribe (m_object, this);
}

OAF::URef<OAF::IUnknown>
OAF::CCollectionTreeNodeProxy::setExtendedObject (OAF::IUnknown* _o)
{
	//
	// NOTE: Данный класс базовый, поэтому проксировать может что угодно:
	// лишь бы нужные интерфейсы поддерживались
	//

	//
	// Отписываемся от прослушивания событий старого объекта (если он был)
	//
	OAF::unsubscribe (m_object, this);

	//
	// Удаляем из прокси-узла все подузлы
	//
	while (OAF::URef<OAF::ITreeNode> n = firstChild ())
		OAF::CCollectionTreeNode::removeChild (n);

	//
	// Запоминаем новый объект для проксирования
	//
	m_object = OAF::queryInterface<OAF::ITreeNode> (_o);

	//
	// Если объект задан, то добавляем к прокси узлу все его подузлы. Используем
	// наследуемую функцию insertAfter, так как appendChild вызовет переопределённую
	// для прокси-узла insertAfter
	//
	if (m_object)
	{
		for (OAF::URef<OAF::ITreeNode> n = m_object->firstChild (); n; n = n->nextSibling ())
			OAF::CCollectionTreeNode::insertAfter (n, NULL, this);
	}

	//
	// Подписываемся на события нового объекта (если он задан)
	//
	OAF::subscribe (m_object, this);

	return this;
}

OAF::URef<OAF::IUnknown>
OAF::CCollectionTreeNodeProxy::getExtendedObject ()
{
	//
	// Возвращаем проксируемый объект
	//
	return m_object.queryInterface<OAF::IUnknown> ();
}

OAF::URef<OAF::ITreeNode>
OAF::CCollectionTreeNodeProxy::insertBefore (OAF::ITreeNode* _new, OAF::ITreeNode* _ref, OAF::INotifyListener* _origin)
{
	Q_ASSERT_X (m_object, Q_FUNC_INFO, "Proxied object was not set yet");

	//
	// Определяем узел, перед которым необходимо вставить заданный в проксируемом узле
	//
	OAF::URef<OAF::ITreeNode> ref;
	if (_ref)
		ref = child4row (m_object, row4child (_ref));

	//
	// Вставляем заданный узел в проксируемый
	//
	m_object->insertBefore (_new, ref, this);

	//
	// Вставляем тот же узел в дерево прокси (создание прокси объекта для вставляемого
	// узла отработает автоматически)
	//
	return OAF::CCollectionTreeNode::insertBefore (_new, _ref, _origin);
}

OAF::URef<OAF::ITreeNode>
OAF::CCollectionTreeNodeProxy::insertAfter (OAF::ITreeNode* _new, OAF::ITreeNode* _ref, OAF::INotifyListener* _origin)
{
	Q_ASSERT_X (m_object, Q_FUNC_INFO, "Proxied object was not set yet");

	//
	// Определяем узел, перед которым необходимо вставить заданный в проксируемом узле
	//
	OAF::URef<OAF::ITreeNode> ref;
	if (_ref)
		ref = child4row (m_object, row4child (_ref));

	//
	// Вставляем заданный узел в проксируемый
	//
	m_object->insertAfter (_new, ref, this);

	//
	// Вставляем тот же узел в дерево прокси (создание прокси объекта для вставляемого
	// узла отработает автоматически)
	//
	return OAF::CCollectionTreeNode::insertAfter (_new, _ref, _origin);
}

OAF::URef<OAF::ITreeNode>
OAF::CCollectionTreeNodeProxy::removeChild (OAF::ITreeNode* _old, OAF::INotifyListener* _origin)
{
	Q_ASSERT_X (m_object, Q_FUNC_INFO, "Proxied object was not set yet");

	//
	// Определяем узел, который необходимо удалить в проксируемом узле и удаляем его
	//
	if (OAF::URef<OAF::ITreeNode> old = child4row (m_object, row4child (_old)))
		m_object->removeChild (old, this);

	//
	// Удаляем заданный узел из дерева прокси
	//
	return OAF::CCollectionTreeNode::removeChild (_old, _origin);
}
