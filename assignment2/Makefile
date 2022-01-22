CMAKE=cmake/bin/cmake

myshell:
	$(CMAKE) -B build/ -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release
	$(MAKE) -C build/

debug:
	$(CMAKE) -B build/ -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Debug
	$(MAKE) -C build/

run: built
	./build/myshell

.ONESHELL:
built:
	if [ ! -f ./build/myshell ]
	then
		$(CMAKE) -B build/ -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release
		$(MAKE) -C build/
	fi

compile:
	$(MAKE) -C build/

clean:
	rm -rf build
