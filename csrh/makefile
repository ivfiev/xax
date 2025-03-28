csrh: ./src/*.c
	mkdir -p ./build
	cp ../libxax/build/libxax.so ./build/libxax.so
	gcc -O3 -I./header -I../libxax/header -o ./build/xax ./src/*.c -L../libxax/build -lxax -Wl,-rpath,../libxax/build
	gcc -O3 -o ./build/mouse ./mouse/*.c

# TODO: ui-install/run?
