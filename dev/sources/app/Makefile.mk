CFLAGS		+= -I./sources/app
CPPFLAGS	+= -I./sources/app

VPATH += sources/app

SOURCES_CPP += sources/app/app.cpp
SOURCES_CPP += sources/app/app_bsp.cpp
SOURCES_CPP += sources/app/app_data.cpp
SOURCES_CPP += sources/app/task_console.cpp
SOURCES_CPP += sources/app/task_list.cpp
SOURCES_CPP += sources/app/task_deviceManager.cpp
SOURCES_CPP += sources/app/task_system.cpp
