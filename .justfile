release:
	@cmake -DCMAKE_BUILD_TYPE=Release -B build/Release
	@cmake --build build/Release

debug:
	@cmake -DCMAKE_BUILD_TYPE=Debug -B build/Debug
	@cmake --build build/Debug

relwithdeb:
	@cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -B build/RelWithDebInfo
	@cmake --build build/RelWithDebInfo

install: release
	cp build/Release/oly ~/.local/bin/
	oly --version

sync_assets:
	rsync -r ~/.local/share/typst/packages/local/oly ./assets/typst/packages/local/

	rsync ~/.local/bin/oly-handler ./assets/typst/oly-scheme-handler/

	rsync ~/.local/share/applications/oly.desktop ./assets/typst/oly-scheme-handler/

	rsync ~/.config/nvim/after/ftplugin/typst/oly.lua ./assets/extras/typst.lua
	rsync ~/.config/nvim/after/ftplugin/tex.lua ./assets/extras/tex.lua

test:
	-./test.lua

lint:
	@-run-clang-tidy -p build -source-filter='^.*/oly/(src|include).*' -quiet
