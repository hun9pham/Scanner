CFLAGS		+= -I./sources/app
CFLAGS		+= -I./sources/app/interfaces
CPPFLAGS	+= -I./sources/app
CPPFLAGS	+= -I./sources/app/interfaces

VPATH += sources/app
VPATH += sources/app/interfaces

SOURCES_CPP += sources/app/app.cpp
SOURCES_CPP += sources/app/app_bsp.cpp
SOURCES_CPP += sources/app/app_data.cpp
SOURCES_CPP += sources/app/task_console.cpp
SOURCES_CPP += sources/app/task_list.cpp
SOURCES_CPP += sources/app/task_deviceManager.cpp
SOURCES_CPP += sources/app/task_system.cpp

# SOURCES_CPP += sources/app/interfaces/task_if.cpp
# SOURCES_CPP += sources/app/interfaces/task_cpu_serial_if.cpp
