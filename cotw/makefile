csrh: ./src/*.c
	mkdir -p ./build
	cp ../libxax/build/libxax.so ./build/libxax.so
	gcc -O3 -march=native -I./include -I../libxax/header -o ./build/xax ./src/*.c -L../libxax/build -lxax -Wl,-rpath,../libxax/build