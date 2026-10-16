all:
	colcon build --symlink-install --merge-install

clean:
	rm -rf build/ install/ log/

.PHONY: all
