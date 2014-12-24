/**
 * @file
 * @brief Текстовый редактор
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <iostream>

#include <QApplication>
#include <QProcessEnvironment>
#include <QTranslator>

#include <OAF/OAF.h>
#include <OAF/CUnknown.h>

#include "CMainWindow.h"

static int
mainLoop (int _argc, char* _argv[])
{
	QApplication app (_argc, _argv);
	app.setQuitOnLastWindowClosed (true);

	//
	// Доступ к переменным окружения
	//
	QProcessEnvironment pe = QProcessEnvironment::systemEnvironment ();

	//
	// Подключаем переводы qt4. Это необходимо на Ubuntu
	//
	QTranslator qt4_translator;
	if (!qt4_translator.load ("qt_" + QLocale::system ().name (), "/usr/share/qt4/translations"))
		qWarning ("Can't load qt4 translator file for locale %s", qPrintable (QLocale::system ().name ()));
	else
		app.installTranslator (&qt4_translator);

	//
	// Подключаем переводы libOAF
	//
	QString oaf_tr_dir = "translate";
	if (pe.contains ("OAF_TR_DIR"))
		oaf_tr_dir = pe.value ("OAF_TR_DIR");

	QTranslator liboaf_translator;
	if (!liboaf_translator.load ("liboaf_" + QLocale::system ().name (), oaf_tr_dir))
		qWarning ("Can't load liboaf translator file for locale %s", qPrintable (QLocale::system ().name ()));
	else
		app.installTranslator (&liboaf_translator);

	CMainWindow* w = new CMainWindow ();
	w->show ();

	return app.exec ();
}

int
main (int _argc, char* _argv[])
{
	//
	// Основной цикл программы
	//
	int r = mainLoop (_argc, _argv);

	//
	// Выгружаем неиспользуемые библиотеки и информацию о классах. Данный этап
	// нужен скорее для контроля правильности разработки, нежели для функционирования
	// программы
	//
	OAF::unloadUnusedLibraries ();
	OAF::unloadClassInfo ();

	foreach (OAF::CUnknown* o, OAF::CUnknown::objects ())
		std::cerr << "Unfreed object = " << qPrintable (o->cid ()) << std::endl;

	return r;
}
