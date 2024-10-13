DEBUG ?= 0
ifeq ($(DEBUG), 1)
	debug_flags := -g
	output_dir := debug
else
	debug_flags := 
	output_dir := release
endif

header_files := $(wildcard ../src/*.h)
source_files := $(wildcard ../src/*.c)
asset_files := $(wildcard ../src/assets/*.*)
libraries := -D_REENTRANT -I/usr/include/SDL2 -lSDL2

stratego: main.o assets
	gcc -o "$(output_dir)/stratego" $(wildcard $(output_dir)/*.o) $(libraries)

main.o: $(header_files) $(source_files)
	@echo "Header files are " $(header_files)
	@echo "Source files are " $(source_files)
	gcc -c $(debug_flags) -Wall $(source_files)
	rm -rf $(output_dir)
	mkdir $(output_dir)
	mv *.o $(output_dir)/

assets: $(asset_files)
	@echo "Asset files are " $(asset_files)
	rm -rf $(output_dir)/assets
	mkdir $(output_dir)/assets
	cp -v $(asset_files) $(output_dir)/assets/
