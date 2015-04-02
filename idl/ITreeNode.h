/**
 * @file
 * @brief Интерфейс узла DOM-подобного дерева
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 *
 * Реализация должна обеспечить возможность управления ссылками на объект, реализующий
 * данный интерфейс. Реализация должна обеспечить хранение ссылок на дочерние объекты.
 */
#ifndef __ITREE_NODE_H
#define __ITREE_NODE_H

#include <QtCore>

#include <idl/IUnknown.h>
#include <idl/INotifySource.h>

namespace OAF
{
	/**
	 * @brief Узел DOM-подобного дерева
	 */
	struct ITreeNode : virtual public OAF::IUnknown
	{
		/**
		 * @name Привязка узла к дереву
		 *
		 * В общем случае родительский объект хранит ссылки на свои дочерние объекты и
		 * отвечает за время их жизни. Поэтому данные функции предназначены для установки
		 * простых указателей на соответствующие объекты.
		 */
		/** @{*/
		/**
		 * @brief Задать указатель на родительский узел
		 */
		virtual void setParent (OAF::ITreeNode* _parent) = 0;

		/**
		 * @brief Задать указатель на предшествующего соседа
		 */
		virtual void setPreviousSibling (OAF::ITreeNode* _prev) = 0;

		/**
		 * @brief Задать указатель на следующего соседа
		 */
		virtual void setNextSibling (OAF::ITreeNode* _next) = 0;
		/** @}*/

		/**
		 * @name Проверка нахождения узла в определённой точке дерева
		 *
		 * Данные методы проверки нужны во-первых для оптимизации вычислений, а во-вторых из-за
		 * проблем, которые возникают при вызове removeChild в деструкторе родителя для проверки
		 * принадлежности к дереву.
		 */
		/** @{*/
		/**
		 * @brief Проверяем совпадение родителя
		 */
		virtual bool parentIs (OAF::ITreeNode* _parent) = 0;

		/**
		 * @brief Проверяем совпадение предыдущего соседа
		 */
		virtual bool previousSiblingIs (OAF::ITreeNode* _prev) = 0;

		/**
		 * @brief Проверяем совпадение следующего соседа
		 */
		virtual bool nextSiblingIs (OAF::ITreeNode* _next) = 0;
		/** @}*/

		/**
		 * @name Навигация по дереву
		 */
		/** @{*/
		/**
		 * @brief Родительский узел
		 */
		virtual OAF::URef<OAF::ITreeNode> parent () = 0;

		/**
		 * @brief Предыдущий узел
		 */
		virtual OAF::URef<OAF::ITreeNode> previousSibling () = 0;

		/**
		 * @brief Следующий узел
		 */
		virtual OAF::URef<OAF::ITreeNode> nextSibling () = 0;

		/**
		 * @brief Первый дочерний узел
		 */
		virtual OAF::URef<OAF::ITreeNode> firstChild () = 0;

		/**
		 * @brief Последний дочерний узел
		 */
		virtual OAF::URef<OAF::ITreeNode> lastChild () = 0;
		/** @}*/

		/**
		 * @name Модификация дерева
		 */
		/** @{*/
		/**
		 * @brief Вставить узел @a _new перед узлом @a _ref
		 *
		 * @param[in] _new    узел для добавления
		 * @param[in] _ref    узел, перед которым необходимо вставить новый узел. Если узел @a _ref
		 *                    не задан, то вставляем перед первым дочерним узлом
		 * @param[in] _origin инициатор добавления узла
		 *
		 * @return Вставленный узел
		 *
		 * @warning Возвращённая ссылка не обязательно эквивалентна @a _new. Это может быть ссылка
		 *          на прокси-объект, вставленный вместо основного объекта. Поэтому в дальнейшем
		 *          необходимо использовать именно её.
		 */
		virtual OAF::URef<OAF::ITreeNode> insertBefore (OAF::ITreeNode* _new, OAF::ITreeNode* _ref, OAF::INotifyListener* _origin = NULL) = 0;

		/**
		 * @brief Вставить узел @a _new после узла @a _ref
		 *
		 * @param[in] _new    узел для добавления
		 * @param[in] _ref    узел, перед которым необходимо вставить новый узел. Если узел @a _ref
		 *                    не задан, то вставляем после последнего дочернего узла
		 * @param[in] _origin инициатор добавления узла
		 *
		 * @return Вставленный узел
		 *
		 * @warning Возвращённая ссылка не обязательно эквивалентна @a _new. Это может быть ссылка
		 *          на прокси-объект, вставленный вместо основного объекта. Поэтому в дальнейшем
		 *          необходимо использовать именно её.
		 */
		virtual OAF::URef<OAF::ITreeNode> insertAfter (OAF::ITreeNode* _new, OAF::ITreeNode* _ref, OAF::INotifyListener* _origin = NULL) = 0;

		/**
		 * @brief Добавить узел @a _new в конец списка дочерних узлов
		 *
		 * @param[in] _new    узел для добавления
		 * @param[in] _origin инициатор добавления узла
		 *
		 * @return добавленный узел
		 *
		 * @warning Возвращённая ссылка не обязательно эквивалентна @a _new. Это может быть ссылка
		 *          на прокси-объект, вставленный вместо основного объекта. Поэтому в дальнейшем
		 *          необходимо использовать именно её.
		 */
		virtual OAF::URef<OAF::ITreeNode> appendChild (OAF::ITreeNode* _new, OAF::INotifyListener* _origin = NULL) = 0;

		/**
		 * @brief Удалить узел @a _old из дерева
		 *
		 * @param[in] _old    узел для удаления
		 * @param[in] _origin инициатор удаления узла
		 *
		 * @return удалённый узел
		 */
		virtual OAF::URef<OAF::ITreeNode> removeChild (OAF::ITreeNode* _old, OAF::INotifyListener* _origin = NULL) = 0;

		/**
		 * @brief Заменить узел @a _old на узел @a _new
		 *
		 * @param[in] _new    узел для добавления
		 * @param[in] _old    узел для замены
		 * @param[in] _origin инициатор замены узла
		 *
		 * @return заменённый узел
		 */
		virtual OAF::URef<OAF::ITreeNode> replaceChild (OAF::ITreeNode* _new, OAF::ITreeNode* _old, OAF::INotifyListener* _origin = NULL) = 0;
		/** @}*/
	};
}

#endif /* __ITREE_NODE_H */
