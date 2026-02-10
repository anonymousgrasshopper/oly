cp_bin:
	-sudo cp build/bin/oly /usr/local/bin

build:
	@cmake --build build
	@just cp_bin

release:
	@cmake -DCMAKE_BUILD_TYPE=Release -B build
	@just build

debug:
	@cmake -DCMAKE_BUILD_TYPE=Debug -B build
	@just build

relwithdeb:
	@cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -B build
	@just build

install: release cp_bin
	oly --version

sync_assets:
	rm -rf ./assets/typst/packages/local/oly
	cp -r ~/.local/share/typst/packages/local/oly ./assets/typst/packages/local/

	rm ./assets/typst/oly-scheme-handler/oly-handler
	cp ~/.local/bin/oly-handler ./assets/typst/oly-scheme-handler/

	rm ./assets/typst/oly-scheme-handler/oly.desktop
	cp ~/.local/share/applications/oly.desktop ./assets/typst/oly-scheme-handler/

	rm ./assets/extras/typst.lua
	cp ~/.config/nvim/after/ftplugin/typst/oly.lua ./assets/extras/typst.lua
	rm ./assets/extras/tex.lua
	cp ~/.config/nvim/after/ftplugin/tex.lua ./assets/extras/tex.lua

test:
	-./test.lua

lint:
	@-run-clang-tidy -p build -source-filter='^.*/oly/(src|include).*' -quiet
