
FLAGS=-Wall -m32 -x c -ansi -std=c89

build: vendor/SDK/amxplugin.c simplesocket.c
	gcc $(FLAGS) -shared -o simplesocket.$(PLUGINTYPE) \
		vendor/SDK/amxplugin.c \
		simplesocket.c

