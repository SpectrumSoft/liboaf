TEMPLATE = subdirs
CONFIG  += ordered
SUBDIRS  = libs plugins programs

#
# Интерфейсы
#
HEADERS += \
	idl/IConfig.h \
	idl/INotifySource.h \
	idl/IGenericFactory.h \
	idl/IInterface.h \
	idl/IIODevice.h \
	idl/IMoniker.h \
	idl/IObjectSource.h \
	idl/IPropertyBag.h \
	idl/IQuery.h \
	idl/ISqlDatabase.h \
	idl/ITextDocument.h \
	idl/ITextSignature.h \
	idl/ITXTPersist.h \
	idl/ITreeNode.h \
	idl/IUIComponent.h \
	idl/IXMLPersist.h \
	idl/IUnknown.h \
	idl/IDifference.h \
	idl/ISearch.h \
	idl/IExchange.h \
	idl/IOperation.h

#
# Переводы
#
TRANSLATIONS += \
	translate/liboaf_ru.ts

TRANSLATIONS_FILES += \
	translate/liboaf_ru.qm

#
# Путь установки общих данных
#
win32 {
	isEmpty(DATA_INSTALL_PATH):DATA_INSTALL_PATH = /data
}
else {
	isEmpty(DATA_INSTALL_PATH):DATA_INSTALL_PATH = /usr/share/oaf
}

#
# Установка общих данных
#
data.files = share/*
data.path  = $${DATA_INSTALL_PATH}

#
# Установка переводов
#
translations.files = $${TRANSLATIONS_FILES}
translations.path  = $${DATA_INSTALL_PATH}/translate

#
# Настройка инсталляции
#
INSTALLS += data translations
