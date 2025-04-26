for dir in ../src/*/; do
	echo "compiling $dir"
	echo "$PWD"
	cd "$dir" || continue
	mkdir -p build
	cd build || continue
	cmake ..
	make -j10
	cd ../../../scripts
done
