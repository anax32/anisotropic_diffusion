# anisotropic diffusion
implementation of anisotropic diffusion for greyscale images

# isotropic diffusion
comparative implementation of gaussian filtering

# connected components
compute connected components of an image.

e.g., run anisotropic diffusion for 80 iterations and compute the connected components to get a label map

# usage
	make apply
	bin/apply -i <input png file> -o <output anisodiff image> -a -n 100
	bin/apply -i <input png file> -o <output isodiff image> -g -n 100
	bin/apply -i <input png file> -o <output label image> -c -n 1
