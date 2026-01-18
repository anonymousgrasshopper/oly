build:
	@cmake --build build

release:
	@cmake -DCMAKE_BUILD_TYPE=Release -B build
	@cmake --build build

debug:
	@cmake -DCMAKE_BUILD_TYPE=Debug -B build
	@cmake --build build

relwithdeb:
	@cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -B build
	@cmake --build build

install: release
	sudo cp build/bin/oly /usr/local/bin

synctypst:
	rm -rf ./assets/typst/packages/local/oly
	cp -r ~/.local/share/typst/packages/local/oly ./assets/typst/packages/local/
