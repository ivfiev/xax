csrh: ./src/*.c
  mkdir -p ./build
	cp ../libxax/build/libxax.so ./build/libxax.so
	gcc -O3 -I./header -I../libxax/header -o ./build/csrh ./src/*.c -L../libxax/build -lxax -Wl,-rpath,../libxax/build

# TODO: ui-install/run?
