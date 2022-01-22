CMAKE=cmake/bin/cmake

caterpillar:
	$(CMAKE) -B build/ -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release
	$(MAKE) -C build/

debug:
	$(CMAKE) -B build/ -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Debug
	$(MAKE) -C build/

run: built
	./build/caterpillar

.ONESHELL:
built:
	if [ ! -f ./build/caterpillar ]
	then
		$(CMAKE) -B build/ -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release
		$(MAKE) -C build/
	fi

compile:
	$(MAKE) -C build/

clean:
	rm -rf build
