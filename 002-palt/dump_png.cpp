#include <iostream>
#include <png++/png.hpp>

int main(int argc, char **argv) {

	if(argc < 3) {
	    std::cerr << "dump_png <original-image> <colors-file> <output-file>\n"
	    "Generates a PNG with the same dimensions as <original-image> replaced by colors from <colors-file>." << std::endl;
	    return -1;
	}

	std::ifstream colors(argv[2]);

	png::image< png::rgb_pixel > original_img(argv[1]);

	long int oc_height = original_img.get_height();
	long int oc_width = original_img.get_width();

	png::image< png::rgb_pixel > out_img(oc_width, oc_height);

	int r, b, g;

	for (png::uint_32 y = 0; y < oc_height; ++y)
	{
	    for (png::uint_32 x = 0; x < oc_width; ++x)
	    {
			if(colors >> r >> g >> b) {
		        out_img[y][x] = png::rgb_pixel(r, g, b);
			} else {
				std::cerr << "Colors file and image resolution don't match, aborting." << std::endl;
				return -1;
			}
	    }
	}
	if(colors >> r >> g >> b) {
		std::cerr << "Colors file and image resolution don't match, aborting." << std::endl;
		return -1;
	}

	out_img.write(argv[3]);
	return 0;
}
