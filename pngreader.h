//#pragma comment (lib, "../../Image/PNG/libpng13.lib")  // FIXME: incorrect path for all occasions

#ifndef PNGIO_H
#define PNGIO_H

#include <png.h>

class PNGIO
{
public:
  static const int  ERR_NOERR = 1;
  static const int  ERR_FOPEN = -1;
  static const int  ERR_PNG_WRITE_STRUCT = -2;
  static const int  ERR_PNG_READ_STRUCT = -3;
  static const int  ERR_PNG_INFO_STRUCT = -4;
  static const int  ERR_PNG_JUMP_HIT = -5;
  static const int  ERR_BAD_COLOUR_TYPE = -6;
  static const int  ERR_INTERLACED_FORMAT = -7;
  static const int  ERR_INCORRECT_BUFFER_FORMAT = -8;
private:
  // custom read function and struct required by libpng incase the CRT libraries we use are incompatible
  static void png_custom_read_fn (
    png_structp png_ptr,
    png_bytep data,
    png_size_t length)
  {
    FILE *f = (FILE *) png_get_io_ptr (png_ptr);

    if (f == NULL)
      return;

    fread (data, 1, length, f);
  }

  static void png_custom_write_fn (
    png_structp png_ptr,
    png_bytep data,
    png_size_t length)
  {
    FILE *f = (FILE *) png_get_io_ptr (png_ptr);

    if (f == NULL)
      return;

    fwrite (data, length, 1, f);
  }

public:
#ifdef IMAGE_H
  // reads a png file from disk and creates an Image from it
  static Image<unsigned char, 2, 3> *readPNG (const char *filename)
  {
    FILE *f = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    int bd, ct, it;
    png_uint_32 w, h;

    Image<unsigned char, 2, 3> *image = NULL;

    // open the file
    f = fopen (filename, "rb\0");

    if (f == NULL)
      return NULL;

    // create the nessecary read structures
    if ((png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
    {
      fclose (f);
      return NULL;
    }

    if ((info_ptr = png_create_info_struct (png_ptr)) == NULL)
    {
      png_destroy_read_struct (&png_ptr, NULL, NULL);
      fclose (f);
      return NULL;
    }

    // basic error handling.
    if (setjmp (png_jmpbuf(png_ptr)))
    {
      png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
      fclose (f);
      return NULL;
    }

    png_set_read_fn (png_ptr, (voidp) f, png_custom_read_fn);  // set our custom read function
    png_read_info (png_ptr, info_ptr);              // start with the header
    png_get_IHDR (png_ptr, info_ptr, &w, &h, &bd, &ct, &it, NULL, NULL);    // dimensions

    png_byte    *data;

    // if the png can be read into our limited Image, read it
    if ((bd == 8) && (ct == PNG_COLOR_TYPE_RGB) && (it == PNG_INTERLACE_NONE))
    {
      data = new png_byte[w*h*3];          // initialise the internal image data array
      png_byte **row_pointers = new png_byte*[h];  // list of pointers to the begining of each row

      for (unsigned int i=0; i<h; i++)
      {
        row_pointers[i] = &data[i*(w*3)];
      }

      png_read_image (png_ptr, row_pointers);    // read the entire image into the array
      delete[] row_pointers;            // free the row pointers
    }

    png_read_end (png_ptr, info_ptr);            // finish reading
    png_destroy_read_struct (&png_ptr, &info_ptr, NULL);  // clean up
    fclose (f);                        // close the file

    image = new Image<unsigned char, 2, 3> (w*h);    // create the image
    image->setSize (image->WIDTH, w);        // set the dimensions
    image->setSize (image->HEIGHT, h);

    for (unsigned int i=0; i<(w*h); i++)    // copy the data
    {
    //  image->samples[i].set (data[(i*3)+0], data[(i*3)+1], data[(i*3)+2]);
      image->samples[i].set (&data[(i*3)]);
    }

    delete[] data;        // delete the buffer

    return image;        // return the image
  }

  static void writePNG (
    const Image<unsigned char, 2, 3> *image,
    const char *filename)
  {
    FILE *f = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    f = fopen (filename, "wb\0");

    if (f == NULL)
      return;

    // create the write stucture
    if ((png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
    {
      fclose (f);
      return;
    }

    // create the info structure
    if ((info_ptr = png_create_info_struct (png_ptr)) == NULL)
    {
       png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
       fclose (f);
       return;
    }

    // setjump for pnglib to return to this function on fail (it calls longjump)
    if (setjmp (png_jmpbuf (png_ptr)))
    {
       png_destroy_write_struct (&png_ptr, &info_ptr);
       fclose (f);
       return;
    }

    // A problem with the pnglib: by using the FILE structure we pass, we may have a different size
    // struct (depending on the link lib. So override the png write functions and provide our own.
    png_set_write_fn (png_ptr, (void *)f, png_custom_write_fn, NULL);

    png_set_IHDR (png_ptr, info_ptr, image->getSize (image->WIDTH), image->getSize (image->HEIGHT),
            8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info (png_ptr, info_ptr);        // start writing the file

    unsigned char *row = new unsigned char[image->getSize (image->WIDTH) * 3];    // buffer for a row of image data

    for (unsigned int i=0; i<image->getSize (image->HEIGHT); i++)
    {
      for (unsigned int j=0; j<image->getSize (image->WIDTH); j++)
        image->samples[j+(i*image->getSize (image->WIDTH))].get (&row[j*3]);  // get the pixel data

      png_write_row (png_ptr, row);  // write the row to disk
    }

    delete row;  // delete the buffer

    png_write_end (png_ptr, info_ptr);        // end writing
    png_destroy_write_struct (&png_ptr, &info_ptr);  // cleanup
    fclose (f);                    // close the file
  }
#endif
  // image buffer functions

/*===============================================
 write a pixel buffer to a png file on disk
===============================================*/
  static int writePixelBuffer (
    const char *filename,
    const unsigned char *buffer,
    const unsigned int width,
    const unsigned int height,
    const unsigned int bitdepth = 8,
    const unsigned int depth = 3,
    bool flip = false)
  {
    FILE *f = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    int colourType = 0;

    f = fopen (filename, "wb\0");

    if (f == NULL)
    {
      return ERR_FOPEN;
    }

    // create the write stucture
    if ((png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
    {
      fclose (f);
      return ERR_PNG_WRITE_STRUCT;
    }

    // create the info structure
    if ((info_ptr = png_create_info_struct (png_ptr)) == NULL)
    {
       png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
       fclose (f);
       return ERR_PNG_INFO_STRUCT;
    }

    // setjump for pnglib to return to this function on fail (it calls longjump)
    if (setjmp (png_jmpbuf (png_ptr)))
    {
       png_destroy_write_struct (&png_ptr, &info_ptr);
       fclose (f);
       return ERR_PNG_JUMP_HIT;
    }

    /* A problem with the pnglib: by using the FILE structure we pass, we may have a
    different size struct (depending on the link lib. we override the png write
    functions and provide our own.*/
    png_set_write_fn (png_ptr, (void *)f, png_custom_write_fn, NULL);

    switch (depth)
    {
    case 1: colourType = PNG_COLOR_TYPE_GRAY; break;
    case 2: colourType = PNG_COLOR_TYPE_GRAY_ALPHA; break;
    case 3: colourType = PNG_COLOR_TYPE_RGB; break;
    case 4: colourType = PNG_COLOR_TYPE_RGBA; break;
    default:
      png_destroy_write_struct (&png_ptr, &info_ptr);
      fclose (f);
      return ERR_BAD_COLOUR_TYPE;
    }

  //  png_set_IHDR (png_ptr, info_ptr, width, height, 8, colourType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_IHDR (png_ptr, info_ptr, width, height, bitdepth, colourType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info (png_ptr, info_ptr);        // start writing the file

    if (flip)
    {
      for (unsigned int i=height; i>0; i--)
        png_write_row (png_ptr, const_cast<png_bytep>(&buffer[(i-1)*width*depth]));  // write the row to disk
    }
    else
    {
      for (unsigned int i=0; i<height; i++)
        png_write_row (png_ptr, const_cast<png_bytep>(&buffer[i*width*depth]));  // write the row to disk
    }

    png_write_end (png_ptr, info_ptr);        // end writing
    png_destroy_write_struct (&png_ptr, &info_ptr);  // cleanup
    fclose (f);                    // close the file

    return ERR_NOERR;
  }

/*===============================================
 read a pixel buffer from a png file on disk.
 Note, buffer must be large enough to hold the
 file. If buffer == NULL, the parameters will
 be filled so a correctly sized buffer can be
 allocated.
===============================================*/
  static const unsigned int  GS = PNG_COLOR_TYPE_GRAY;
  static const unsigned int  RGB = PNG_COLOR_TYPE_RGB;
  static const unsigned int  RGBA = PNG_COLOR_TYPE_RGB_ALPHA;

  static int readPixelBuffer (
    const char *filename,
    unsigned char *buffer,
    unsigned int& width,
    unsigned int& height,
    unsigned int& bitdepth,
    unsigned int& colourType,
    const bool flip = true)
  {
    FILE *f = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    int bd, d, ct, it;
    png_uint_32 w, h;

    // open the file
    f = fopen (filename, "rb\0");

    if (f == NULL)
    {
      return ERR_FOPEN;
    }

    // create the nessecary read structures
    if ((png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
    {
      fclose (f);
      return ERR_PNG_READ_STRUCT;
    }

    if ((info_ptr = png_create_info_struct (png_ptr)) == NULL)
    {
      png_destroy_read_struct (&png_ptr, NULL, NULL);
      fclose (f);
      return ERR_PNG_INFO_STRUCT;
    }
    // basic error handling.
    if (setjmp (png_jmpbuf (png_ptr)))
    {
      png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
      fclose (f);
      return ERR_PNG_JUMP_HIT;
    }
    // set the custom read function and read the header
    png_set_read_fn (png_ptr, (void*)f, png_custom_read_fn);
    png_read_info (png_ptr, info_ptr);

    // get the dimensions of the image
    png_get_IHDR (png_ptr, info_ptr, &w, &h, &bd, &ct, &it, NULL, NULL);    // dimensions

    // caller wants the image information, not the image
    if (buffer == NULL)
    {
      width = w;
      height = h;
      bitdepth = bd;
      colourType = ct;

      png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
      fclose (f);
      return ERR_NOERR;
    }
    else  // caller wants the image, check the details are correct
    {
      if ((width != w) ||
          (height != h) ||
          (bitdepth != static_cast<unsigned int>(bd)) ||
          (colourType != static_cast<unsigned int>(ct)))
      {
        png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
        fclose (f);
        return ERR_INCORRECT_BUFFER_FORMAT;
      }

      switch (ct)
      {
      case GS:  d = 1; break;
      case RGB:  d = 3; break;
      case RGBA:  d = 4; break;
      }
    }

    // can't handle interlaced files yet (I don't know what the difference is)
    if (it != PNG_INTERLACE_NONE)
    {
      png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
      fclose (f);
      return ERR_INTERLACED_FORMAT;
    }

    // if the png can be read into the buffer
  //  data = new png_byte[w*h*3];          // initialise the internal image data array
    png_byte **row_pointers = new png_byte*[h];  // list of pointers to the begining of each row

    if (flip)
    {
      for (int i=h-1; i>0-1; i--)
        row_pointers[i] = &buffer[i*(w*d)];    // FIXME: '3' is the colour type * bitdepth or something
    }
    else
    {
      for (unsigned int i=0; i<h; i++)
        row_pointers[i] = &buffer[i*(w*d)];    // FIXME: '3' is the colour type * bitdepth or something
    }

    png_read_image (png_ptr, row_pointers);    // read the entire image into the array

    // cleanup
    delete[] row_pointers;            // free the row pointers
    row_pointers = NULL;

    png_read_end (png_ptr, info_ptr);            // finish reading
    png_destroy_read_struct (&png_ptr, &info_ptr, NULL);  // clean up
    fclose (f);                        // close the file

    // return success
    return ERR_NOERR;
  }
};
#endif
