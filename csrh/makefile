csrh: ./src/*.c
	mkdir -p ./build
	cp ../libxax/build/libxax.so ./build/libxax.so
	gcc -O2 -march=native -I./header -I../libxax/header -o ./build/xax ./src/*.c -L../libxax/build -lxax -Wl,-rpath,../libxax/build
	gcc -O2 -march=native -o ./build/mouse ./mouse/*.c

# TODO: ui-install/run?
