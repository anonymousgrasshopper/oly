build:
	@cmake --build build

release:
	@cmake -DCMAKE_BUILD_TYPE=Release -B build

debug:
	@cmake -DCMAKE_BUILD_TYPE=Debug -B build

relwithdeb:
	@cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -B build

install: release
	sudo cp build/bin/oly /usr/local/bin
