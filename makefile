compile:
	make -C memoria
	make -C cpu
	make -C kernel
	make -C entradasalida
clean:
	make clean -C memoria
	make clean -C cpu
	make clean -C kernel
	make clean -C entradasalida
	make clean -C utils