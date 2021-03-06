#
# Идентификатор сборки
#
isEmpty(BUILD_ID):BUILD_ID = "Development"

#
# Определяем текущую дату и время для упрощения последующей идентификации версии
#
unix {
	OS = linux

	BUILD_DATE = $$system(date --rfc-3339=seconds)
	BUILD_VER  = $$sprintf("%1 [%2]", $$BUILD_ID, $$BUILD_DATE)
}

win32 {
	OS = win32

	BUILD_TIME = $$system(time /T)
	BUILD_DATE = $$system(date /T)
	BUILD_VER  = $$sprintf("%1 [%2 %3]", $$BUILD_ID, $$BUILD_DATE, $$BUILD_TIME)
}

#
# Определяем версию в виде глобального символа
#
# NOTE: стандартную переменную qmake DEFINES использовать не получилось,
#       т.к. пробелы в ней расцениваются как отдельные символы - нам же
#       нужна вся строка целиком
#
QMAKE_CXXFLAGS += -DBUILD_VER=\"\\\"$${BUILD_VER}\\\"\"
