/**
 * @file
 * @brief Динамический интерфейс пользователя
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 *
 * @page uicomponent Реализация динамического интерфейса пользователя
 *
 * @section uicomponent_defs Термины и определения
 *
 * - <c>элемент интерфейса</c> --- это действие (QAction) или виджет (QWidget), рассматриваемый как
 *   элементарный неделимый объект, который объединяясь по определённым правилам с другими
 *   элементами интерфейса составляют интерфейс пользователя;
 * - <c>контейнерный виджет</c> --- это один из Qt-виджетов - QToolBox, QTabWidget, QStackedWidget,
 *   QToolBar, QMenuBar, QMenu или QMainWindow, которые позволяют объединять элементы интерфейса;
 * - <c>ui-контейнер</c> --- это программный объект, реализующий программный интерфейс IUIContainer,
 *   и предоставляющий именованные контейнерные виджеты для размещения в них элементов интерфейса;
 * - <c>ui-компонент</c> --- это программный объект, реализующий программный интерфейс IUIComponent,
 *   и размещающий в контейнерных виджетах одного из вышестоящих ui-контейнеров свои элементы
 *   интерфейса.
 *
 * @section uicomponent_introduction Введение
 *
 * Программные интерфейсы IUIContainer и IUIComponent предназначены для реализации динамического
 * интерфейса пользователя, который зависит от текущего состояния программы. Текущий интерфейс
 * пользователя (то, что пользователь видит в конкретный момент времени - состав меню, панелей
 * инструментов и т.д.) состоит из множества объёдинённых по определённым правилам частей. Каждая
 * из этих частей принадлежит конкретному ui-компоненту, который взаимодействует с пользователем.
 * Все части состоят из набора элементов интерфейса, соответствующего тому или иному состоянию
 * ui-компонента. Отдельные элементы интерфейса группируются в единый интерфейс с помощью
 * контейнерных виджетов.
 *
 * @attention
 * IDL-UIC-001. Разработка общей структуры интерфейса пользователя (структуры меню, панелей
 * инструментов, дополнительных областей для размещения элементов интерфейса) является задачей
 * архитектора системы и её дизайнера и эта структура является составной частью общей архитектуры
 * системы.
 *
 * Размещение элементов интерфейса пользователя в контейнерных виджетах описывается с помощью
 * XML-определений, которые позволяют задать группировку элементов интерфейса и правила их
 * размещения друг относительно друга. Все контейнерные виджеты представлены в нём тэгом 'bag'.
 * Если контейнерный виджет допускает дополнительную вложенность элементов интерфейса (как QMenu
 * или QMainWindow), то вложенные уровни описываются с помощью тэга 'folder'. Отдельные элементы
 * интерфейса описываются с помощью тэга 'item'. Корневым тэгом всего описания всегда является
 * тэг 'uidef'. Если контенерный виджет позволяет визуально отделять элементы интерфейса друг от
 * друга, то для обозначения такого разделителя используется тэг 'separator'.
 *
 * Конкретный смысл тэгов и элементов определяется контейнерными виджетами. Каждый контейнерный
 * виджет должен иметь имя, определённое архитектором приложения, и служащее соглашением между
 * разработчиками для формирования единого интерфейса пользователя. Например:
 * @verbatim
 * <?xml version=1.0?>
 * <uidef>
 *     <!-- Главное меню программы -->
 *     <bag id="ui:menubar">
 *         <!-- Подменю "Расчёты" -->
 *         <folder id="ui:calculation" label="Расчёты">
 *             <!-- Подменю "План применения НК" -->
 *             <folder id="ui:nav-plan" label="План применения НК" icon=":/icons/icon.png">
 *                 <!-- Набор QAction, которые показываются как пункты меню -->
 *                 <item id="ui:nav-plan:select-reckoning-action"/>
 *                 <item id="ui:nav-plan:select-corrector-action"/>
 *                 <separator/>
 *                 <item id="ui:nav-plan:bounds-editor-action"/>
 *             </folder>
 *         </folder>
 *     </bag>
 *
 *     <!-- Табличный "док" главного окна для размещения горизонтально ориентированных данных -->
 *     <bag id="ui:table_dock">
 *         <!-- Панель, на которой отображается график ошибки навигации -->
 *         <item id="ui:nav-plan:panel" label="Ошибка навигации" tip="Ошибка навигации"/>
 *     </bag>
 *
 *     <!-- Контекстное меню панели отображения ошибки навигации -->
 *     <bag id="ui:nav-plan:panel:menu">
 *         <!-- Те же QAction, что размещены в главном меню, показываются и в контекстном
 *              меню панели (этот контейнерный виджет предоставляется ui-компонентом, реализующим
 *              редактор плана применения НК) -->
 *         <item id="ui:nav-plan:select-reckoning-action"/>
 *         <item id="ui:nav-plan:select-corrector-action"/>
 *         <item id="ui:nav-plan:bounds-editor-action"/>
 *     </bag>
 * </uidef>
 * @endverbatim
 *
 * В данном примере в главном меню с именем "ui:menubar", в субменю "Расчёты"/"План применения НК",
 * размещаются элементы интерфейса пользователя QAction с именами "ui:nav-plan:select-reckoning-action"
 * и т.д. Читаемые названия пунктов меню определены в самих QAction. В нижней плавающей панели с
 * именем "ui:table_dock" размещается виджет с именем "ui:nav-plan:panel", в котором будет рисоваться
 * график ошибки НК. Этот виджет размещается на вкладке с названем "Ошибка навигации" и соответствующей
 * подсказкой. В контекстном меню этого виджета, которое имеет имя "ui:nav-plan:panel:menu",
 * размещаются те же QAction, что и в главном меню.
 *
 * К любому из элементов может быть дополнительно приписан атрибут priority, который представляет
 * собой произвольное целое число. При его отсутствии считается, что он равен 0. Данный атрибут
 * показывает положение данного элемента интерфейса относительно других элементов. Чем меньше
 * число, тем выше будет размешён данный элемент. Использовать этот атрибут рекомендуется
 * только в необходимых случаях.
 *
 * Всему этому набору элементов при его размещении присваивается уникальный идентификатор, который
 * в дальнейшем может быть использован для его удаления из интерфейса пользователя.
 *
 * @section uicomponent_arch Архитектура и протокол взаимодействия
 *
 * Объекты, реализующие программные интерфейсы IUIContainer/IUIComponent, связываются в древовидную
 * структуру произвольной глубины, для которой должны соблюдаться следующие правила:
 *    - иерархия объектов не должна содержать циклов;
 *    - каждый ui-компонент может быть подключён не более чем к одному ui-контейнеру;
 *    - к любому ui-контейнеру может быть подключено произвольное количество ui-компонентов (если
 *      это не ограничено реализацией ui-контейнера);
 *    - ui-контейнеры всегда управляют временем жизни подключённых к ним ui-компонентов (ui-компоненты
 *      могут держать только указатели на свои ui-контейнеры, а не ссылки. Тогда как ui-контейнеры
 *      наоборот - обязаны держать ссылки на подключённые к ним ui-компоненты);
 *    - в период времени между циклами подключения/отключения ui-компонента ui-контейнер обязан
 *      предоставлять ему все сервисы, определяемые его интерфейсами. Таким образом ui-компонент
 *      может использовать любой из сервисов, любого из вышестоящих ui-контейнеров (для
 *      прохода вверх по иерархии предназначена процедура IUIComponent::getUIContainer);
 *    - иерархия ui-контейнер/ui-компонент не обязана отражать иерархию обрабатываемых ими данных.
 *
 * @attention
 * IDL-UIC-002. Запрещается подключать один ui-компонент к нескольким ui-контейнерам, так как это
 * сильно усложняет логику работы.
 *
 * Если требуется разделять данные между несколькими ui-компонентами, то их следует выделить в
 * отдельные объекты. То есть при разработке интерфейса пользователя настоятельно рекомендуется
 * использовать классическую MVC-модель (Model/View/Controller), в которой View/Controller
 * реализуются ui-компонентом, а Model - выносится в отдельный набор классов.
 *
 * @verbatim
 * +--------------+            +--------------+
 * | IUIContainer |1          *| IUIComponent |
 * +--------------+<>----------+--------------+
 * +--------------+            +--------------+
 * +--------------+            +--------------+
 * @endverbatim
 *
 * @verbatim
 *                          +---------------+
 *                          | ui-контейнер1 |
 *                          +---------------+
 *                             IUIContainer
 *                                  ^
 *                                  |
 *                 +------...-------+-------...---------+
 *                 |                                    |
 *            IUIComponent                         IUIComponent
 *         +---------------+                    +---------------+
 *         | ui-компонент1 |                    | ui-компонент2 |
 *         | ui-контейнер2 |                    | ui-контейнер3 |
 *         +---------------+                    +---------------+
 *            IUIContainer                         IUIContainer
 *                 ^                                    ^
 *                 |                                    |
 *                 .                                    .
 *                 .                                    .
 *                 .                                    .
 *                 |                                    |
 *        +---...--+---...---+                +---...---+---...---+
 *        |                  |                |                   |
 *     IUIComponent     IUIComponent      IUIComponent       IUIComponent
 *  +---------------+ +---------------+ +---------------+ +---------------+
 *  | ui-компонент3 | | ui-компонент4 | | ui-компонент5 | | ui-компонент6 |
 *  +---------------+ +---------------+ +---------------+ +---------------+
 * @endverbatim
 *
 * Обработка описания интерфейса пользователя выполняется рекурсивно снизу вверх по иерархии до тех
 * пор, пока все его элементы не окажутся размещёнными в заданных контейнерных виджетах или пока
 * не будет достигнут верхний ui-контейнер иерархии. Каждый из ui-контейнеров обрабатывает элементы
 * пользовательского интерфейса для "своих" контейнерных виджетов, после чего удаляет их из описания
 * и оставшуюся часть передаёт вверх по иерархии. Вся эта цепочка связывается вместе и в дальнейшем
 * так же целиком и удаляется.
 *
 * @verbatim
 *  +--------------+                 +--------------+
 *  | ui-контейнер |                 | ui-компонент |
 *  | IUIContainer |                 | IUIComponent |
 *  +--------------+                 +--------------+
 *        |                                | Этап 1
 *        |                                |   |
 *        |            setUIContainer(this)|   |
 *        |------------------------------->|   |
 *        |addUI                           |   |
 *        |<-------------------------------|   |
 *        |                       getUIItem|   |
 *        |------------------------------->|   |
 *        |                                |   V
 *        |                                |--------
 *        |                                | Этап 2
 *        |                                |
 *        |                  activate(true)|   |
 *        |------------------------------->|   |
 *        |addUI                           |   |
 *        |<-------------------------------|   |
 *        |                       getUIItem|   |
 *        |------------------------------->|   |
 *        |                                |   V
 *        |                                |--------
 *        |                                | Этап 3
 *        |                                |   |
 *        |                 activate(false)|   |
 *        |------------------------------->|   |
 *        |removeUI                        |   |
 *        |<-------------------------------|   |
 *        |                                |   V
 *        |                                |-------
 *        |                                | Этап 4
 *        |                                |   |
 *        |            setUIContainer(NULL)|   |
 *        |------------------------------->|   |
 *        |removeUI                        |   |
 *        |<-------------------------------|   |
 *        |                                |   V
 *        |                                |-------
 *        V                                V
 * @endverbatim
 *
 * Взаимодействие ui-контейнеров и ui-компонентов осуществляется в несколько этапов. При этом
 * инициатором взаимодействия всегда является ui-контейнер:
 * -# на первом этапе ui-контейнер "подключает" к себе ui-компонент. На этом этапе ui-компонент
 *    считается неактивным и он может разместить в интерфейсе пользователя свои элементы, которые
 *    не зависят от его активности;
 * -# на втором этапе ui-контейнер активирует ui-компонент и тот может разместить свои дополнительные
 *    элементы интерфейса пользователя;
 * -# на третьем этапе ui-контейнер деактивирует ui-компонент. На этом этапе ui-компонент должен
 *    удалить из интерфейса пользователя свою часть, размещённую на втором этапе;
 * -# на последнем, четвёртом, этапе ui-контейнер "отключает" от себя ui-компонент. На этом этапе
 *    ui-компонент должен удалить из интерфейса пользователя свои элементы, размещённые на первом
 *    этапе.
 *
 * При размещении элементов интерфейса пользователя сначала вызывается процедура ui-контейнера
 * addUI, которой передаётся описание интерфейса, а затем, по мере необходимости, вызывается
 * процедура ui-компонента getUIItem для получения и размещения конкретных элементов из этого
 * описания.
 *
 * ui-контейнеры могут выполнять произвольное число циклов активации/деактивации и подключения/
 * отключения ui-компонентов при условии, что циклы активации/деактивации вложены в циклы
 * подключения/отключения.
 *
 * Помимо данных этапов ui-компоненты могут определять свои собственные этапы взаимодействия с
 * пользователем, в которых добавлять и удалять части его интерфейса. Единственным условием
 * является обязательная вложенность этих этапов в цикл активации/деактивации.
 *
 * Для упрощения разработки в составе стандартной библиотеки OAF реализованы классы CUIManager,
 * CUIContainer_Simple и CUIContainer_Managed, которые реализуют обработку XML-описаний интерфейса
 * и заполнение в соответствии с ними заданных контейнерных виджетов.
 */
#ifndef __IUICOMPONENT_H
#define __IUICOMPONENT_H

#include <QUuid>
#include <QString>
#include <QObject>

#include <idl/IInterface.h>

namespace OAF
{
	struct IUIComponent;

	/**
	 * @brief Интерфейс ui-контейнера
	 *
	 * Этот интерфейс содержит механизмы, позволяющие ui-компоненту создавать нужный ему интерфейс
	 * пользователя.
	 *
	 * В задачу ui-контейнера входит объединить вновь создаваемый интерфейс пользователя с уже
	 * существующим.
	 *
	 * @attention
	 * IDL-UIC-003. Все ui-компоненты должны держать указатели на ui-контейнеры, к которым они
	 * подключены а не ссылки.
	 */
	struct IUIContainer : virtual public OAF::IInterface
	{
		/**
		 * @brief Создать часть интерфейса пользователя по заданному описанию
		 *
		 * @param[in] _uidef XML-описание части интерфейса пользователя
		 * @param[in] _uic указатель на интерфейс компонента
		 *
		 * @return идентификатор созданной части интерфейса пользователя
		 */
		virtual QUuid addUI (const QString& _uidef, IUIComponent* _uic) = 0;

		/**
		 * @brief Удалить часть интерфейса пользователя с идентификатором _uid
		 *
		 * @param[in] _id идентификатор удаляемой части интерфейса пользователя
		 *
		 * @return при успешном удалении - пустой идентификатор
		 */
		virtual QUuid removeUI (const QUuid& _id) = 0;
	};

	/**
	 * @brief Интерфейс ui-компонента
	 *
	 * @attention
	 * IDL-UIC-004. ui-контейнеры должны управлять временем жизни подключённых к ним ui-компонентов
	 */
	struct IUIComponent : virtual public OAF::IInterface
	{
		/**
		 * @brief Задать метку объекту интерфейса, добавляемому в заданный контейнер
		 *
		 * Переопределение метки виджета, добавляемого в заданный контейнер интерфейса пользователя.
		 * Данный механизм предназначен для управления наименованиями элементов интерфейса пользователя
		 * со стороны контейнера. Например это необходимо если в интерфейсе есть несколько однотипных
		 * компонентов, но предназначенных для редактирования разных объектов данных.
		 */
		virtual void setUILabel (const QString& _bag, const QString& _label) = 0;

		/**
		 * @brief Задать ui-контейнер, с которым связан данный ui-компонент
		 *
		 * @attention
		 * IDL-UIC-005. ui-компонент должен правильно реагировать на смену ui-контейнера (как минимум
		 * на сброс ui-контейнера в NULL).
		 *
		 * @param[in] _uic ui-контейнер для данного ui-компонента
		 */
		virtual void setUIContainer (OAF::IUIContainer* _uic) = 0;

		/**
		 * @brief Получить ui-контейнер, с которым связан данный ui-компонент
		 *
		 * Эта функция может быть использована для получения доступа к интерфейсам объектов вверх по
		 * иерархии ui-контейнер/ui-компонент
		 */
		virtual OAF::IUIContainer* getUIContainer () = 0;

		/**
		 * @brief Активация/деактивация ui-компонента
		 *
		 * @param[in] _active признак активации/деактивации ui-компонента
		 */
		virtual void activate (bool _active) = 0;

		/**
		 * @brief Получить указатель на элемент интерфейса пользователя ui-компонента
		 *
		 * @attention
		 * IDL-UIC-006. Этот метод должен возвращать указатель на QAction или QWidget, в зависимости от
		 * типа контейнерного виджета, в котором должен быть размещён данный элемент интерфейса.
		 *
		 * @param[in] _id идентификатор элемента интерфейса пользователя
		 */
		virtual QObject* getUIItem (const QString& _id) = 0;
	};
}

#endif /* __IUICOMPONENT_H */
