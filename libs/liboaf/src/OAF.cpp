/**
 * @file
 * @brief Реализация базовых функций OAF/Qt4
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/CExpression.h>
#include <OAF/OQL.h>
#include <OAF/OAF.h>

#include "CFunctionFactory.h"
#include "CClassInfo.h"

using namespace OAF;

const OAF::IPropertyBag*
OAF::getClassInfo (const QString& _cid)
{
	return static_cast<const OAF::IPropertyBag*> (CClassInfo::getClassInfo (_cid));
}

std::size_t
OAF::query (QStringList& _out, const QString& _req)
{
	//
	// Разбираем запрос с подстановкой OAF-функций
	//
	if (OAF::URef<OAF::IQuery> req = OQL::parseQuery (_req, OAF::functionFactory ()))
	{
		//
		// Результат выполнения запроса представляет собой список CID с вычисленными
		// для каждого выражениями сортировки, упорядоченный заданным образом
		//
		QList<QPair<QString, QList<QVariant> > > result;

		//
		// Вычисляем нужное количество элементов результата
		//
		int limit = 0;
		if (req->limit ())
		{
			if (req->limit ()->type () != OAF::IExpression::_CONST)
				throw OAF::EvaluateException ("LIMIT clause must be _CONST expresion");

			QVariant limitv = req->limit ()->eval (NULL);
			if (!limitv.convert (QVariant::Int))
				throw OAF::EvaluateException ("LIMIT clause must be _INTEGER expresion");

			limit = limitv.value<int> ();
		}

		//
		// Сначала отбираем все подходящие OAF-классы с одновременным вычислением для них
		// выражений сортировки
		//
		// @todo пока делаем простым перебором
		//
		{
			const CClassInfo::ClassInfoMap& class_info_map = CClassInfo::enumClassInfo ();

			for (CClassInfo::ClassInfoMap::const_iterator i = class_info_map.begin (); i != class_info_map.end (); ++i)
			{
				//
				// По умолчанию отбираем всё
				//
				QVariant wherev (true);

				//
				// Если задано, то вычисляем выражение отбора для текущего
				// описания OAF-класса
				//
				if (req->where ())
				{
					wherev = req->where ()->eval (i.value ().ptr ());

					//
					// Если результат вычисления выражения - NULL, то считаем
					// его за false
					//
					if (wherev.isNull ())
						wherev = QVariant (false);

					//
					// Пытаемся преобразовать результат вычислений в bool
					//
					if (!wherev.convert (QVariant::Bool))
						throw OAF::EvaluateException ("WHERE clause must be _BOOLEAN expression");
				}

				//
				// Если выражение отбора вычислилось в TRUE
				//
				if (wherev.value<bool> ())
				{
					//
					// Вычисляем выражения сортировки для отобранного описания OAF-класса
					//
					QList<QVariant> order_values;
					if (req->order ())
						order_values = req->order ()->eval (i.value ().ptr ()).value<QList<QVariant> > ();

					//
					// Запоминаем CID и вычисленные для него выражения сортировки
					//
					result.append (qMakePair (i.key (), order_values));

					//
					// Если выражения сортировки нет и заданный лимит достигнут, то заканчиваем
					// подбор (это оптимизация вычислений, которая активно используется при
					// разборе именён моникеров)
					//
					if (!req->order () && (limit > 0) && (result.size () >= limit))
						break;
				}
			}
		}

		//
		// Если задано, то сортируем результат
		//
		// @todo Плохо, что в QT нет аналога partial_sort, а то можно было бы сортировать
		//       пока не отсортируется нужное количество элементов и не сортировать всё
		//
		if (req->order ())
			std::sort (result.begin (), result.end (), OQL::Comparator<QString> (req->order ()->eval (NULL).value<QList<QVariant> > ()));

		//
		// Вычисляем число записей для вывода
		//
		if ((limit == 0) || (result.size () < limit))
			limit = result.size ();

		//
		// Все вычисления завершены успешно и можно заполнить выходной массив
		//
		_out.clear ();
		for (int i = 0; i < limit; ++i)
			_out.append (result[i].first);
	}

	return _out.size ();
}

OAF::URef<OAF::IUnknown>
OAF::createFromCID (const QString& _cid)
{
	if (const CClassInfo* c = CClassInfo::getClassInfo (_cid))
		return c->createObject (_cid);

	return OAF::URef<OAF::IUnknown> ();
}

OAF::URef<OAF::IMoniker>
OAF::createMoniker (OAF::IMoniker* _left, const QString& _prefix, const QString& _suffix)
{
	//
	// Запрос на поиск моникера с заданным именем
	//
	static QString query = "repo_ids.has('OAF::IMoniker') AND monikers.has('%1') ORDER BY ifnull(priority, 0) LIMIT 1";

	//
	// Ищем первый подходящий моникер
	//
	QStringList cids;
	if (OAF::query (cids, query.arg (_prefix)) > 0)
	{
		//
		// Создаём объект моникера
		//
		if (OAF::URef<OAF::IUnknown> o = OAF::createFromCID (cids[0]))
		{
			//
			// Преобразуем к интерфейсу моникера
			//
			if (OAF::URef<OAF::IMoniker> m = o.queryInterface<OAF::IMoniker> ())
			{
				//
				// Если моникер успешно создан, то устанавливаем его параметры.
				// Моникер при этом запоминает ссылку на _left
				//
				m->set (_left, _prefix, _suffix);

				//
				// И возвращаем его в качестве результата
				//
				return m;
			}
			else
				qWarning ("Can't resolve OAF::IMoniker interface for class %s", qPrintable (interfaceName (typeid (*o))));
		}
		else
			qWarning ("Can't create moniker from cid %s", qPrintable (cids[0]));
	}
	else
		qWarning ("Can't find monikers with query %s", qPrintable (query.arg (_prefix)));

	//
	// Если создать новый моникер не удалось, то возвращаем старый
	//
	return _left;
}

OAF::URef<OAF::IMoniker>
OAF::parseName (const QString& _name)
{
	//
	// Последний моникер цепочки
	//
	OAF::URef<OAF::IMoniker> last;

	//
	// Текущие префикс и суффикс
	//
	QString prefix;
	QString suffix;

	//
	// Где мы находимся
	//
	enum
	{
		IN_PREFIX, // Обрабатывается префикс
		IN_SUFFIX  // Обрабатывается суффикс
	} state = IN_PREFIX;

	//
	// По всем символам имени
	//
	for (QString::const_iterator c = _name.begin (); c != _name.end (); ++c)
	{
		//
		// Если это экранирующий символ, то следующий символ добавляем
		// вне зависимости от его специального значения
		//
		if (*c == '\\')
		{
			if (++c == _name.end ())
				break;

			if (state == IN_PREFIX)
				prefix += *c;
			else
				suffix += *c;
		}

		//
		// Если это разделитель моникеров
		//
		else if (*c == '#')
		{
			//
			// Моникеры без префиксов просто игнорируем
			//
			if ((state == IN_SUFFIX) && !prefix.isEmpty ())
				last = OAF::createMoniker (last, prefix, suffix);

			prefix.clear ();
			suffix.clear ();

			state = IN_PREFIX;
		}

		//
		// Если это моникер '!'
		//
		else if (*c == '!')
		{
			//
			// Моникеры без префиксов просто игнорируем
			//
			if ((state == IN_SUFFIX) && !prefix.isEmpty ())
				last = OAF::createMoniker (last, prefix, suffix);

			prefix = '!';
			suffix.clear ();

			state = IN_SUFFIX;
		}

		//
		// Если это разделитель префикса
		//
		else if (*c == ':')
		{
			//
			// Если мы обрабатываем префикс, то переходим к
			// обработке суффикса
			//
			if (state == IN_PREFIX)
			{
				//
				// Если префикс пуст, то и оставляем его пустым.
				// Моникер с таким префиксом будет проигнорирован.
				// Если префикс не пуст, то ':' считается частью
				// префикса
				//
				if (!prefix.isEmpty ())
					prefix += *c;

				state = IN_SUFFIX;
			}
			//
			// Если мы обрабатываем суффикс, то так и продолжаем.
			// То есть в суффиксе символ ':' не имеет никакого
			// специального значения и может использоваться как
			// есть без экранировки
			//
			else
				suffix += *c;
		}

		//
		// Для всех остальных символов
		//
		else
		{
			if (state == IN_PREFIX)
				prefix += *c;
			else
				suffix += *c;
		}
	}

	//
	// Моникеры без префиксов просто игнорируем
	//
	if ((state == IN_SUFFIX) && !prefix.isEmpty ())
		last = OAF::createMoniker (last, prefix, suffix);

	return last;
}

QString
OAF::unparseName (OAF::IMoniker* _moniker)
{
	QString name;

	if (_moniker)
	{
		//
		// Запрашиваем имя для "левого" моникера
		//
		name = OAF::unparseName (_moniker->left ());

		//
		// Упаковка '!' моникера
		//
		if (_moniker->prefix () == "!:")
		{
			//
			// Никаких разделителей и префиксов не добавляем, так
			// как моникер '!' сам по себе является и разделителем
			// и префиксом
			//
			name += '!';
		}
		else
		{
			//
			// Если имя слева не пусто, то добавляем символ разделитель
			//
			if (!name.isEmpty ())
				name += '#';

			name += _moniker->prefix ();
		}

		name += _moniker->suffix ();
	}

	return name;
}

void
OAF::unloadUnusedLibraries ()
{
	const CClassInfo::ClassInfoMap& class_info_map = CClassInfo::enumClassInfo ();

	for (CClassInfo::ClassInfoMap::const_iterator i = class_info_map.begin (); i != class_info_map.end (); ++i)
		i.value ()->unloadUnusedLibrary ();
}

void
OAF::unloadClassInfo ()
{
	CClassInfo::unloadClassInfo ();
}

OAF::IFunctionFactory*
OAF::functionFactory ()
{
	static OAF::CFunctionFactory factory;

	return &factory;
}
