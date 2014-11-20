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

#include <OAF/OAF.h>
#include <OAF/CUnknown.h>

#include "CMainWindow.h"

static int
mainLoop (int _argc, char* _argv[])
{
	QApplication app (_argc, _argv);
	app.setQuitOnLastWindowClosed (true);

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
	// Выгружаем неиспользуемые библиотеки и информацию о классах. Данный этап нужен скорее для
	// контроля правильности разработки, нежели для функционирования программы
	//
	// FIXME: выгрузку библиотек не делаем из-за проблем с потоками в LyX (похоже какие-то
	//        связанные с потоками ресурсы не высвобождаются из-за чего приложение крашится
	//        при выходе на pthread_cond_wait);
	// раскомменчивание пока приведит к падению при выходе из Пикет под Windows,
	// если линковка некоторых библиотек (libs/) - динамическая
	//
//	OAF::unloadUnusedLibraries ();
	OAF::unloadClassInfo ();

	foreach (OAF::CUnknown* o, OAF::CUnknown::objects ())
		std::cerr << "Unfreed object = " << qPrintable (o->cid ()) << std::endl;

	return r;
}
