/*
Small app for testing anisotropic non-linear diffusion
*/
#define _USE_MATH_DEFINES
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "pngreader.h"
#include "diffusion.h"
#include "connected_components.h"
#include "colourutils.h"

#include <iostream>

#define ITERS    100

// gaussian
void gauss (
  double *input,
  unsigned char *oup,
  const unsigned int maxiters,
  const unsigned int w,
  const unsigned int h)
{
  GaussianDiffusion *diff;
  unsigned int i, t;
  char buf[256];

  diff = new GaussianDiffusion (w, h, 5);

  for (t=0; t<maxiters; t++)
  {
    fprintf (stdout, "\rStep %i/%i...", t, maxiters);
    diff->diffusionStep (input);

    // to char
    for (i=0; i<w*h; i++)
      oup[i] = (unsigned char)(input[i]*255.0f);

    sprintf (buf, "GAUSS_%03i.png", t);
    assert (PNGIO::writePixelBuffer (buf, oup, w, h, 8, 1) == PNGIO::ERR_NOERR);
  }

  fprintf (stdout, "\rCompleted %i steps\n", maxiters);

  delete diff;
}

// non-linear anisotropic diffusion
void nand (
  double *input,
  unsigned char *oup,
  const unsigned int maxiters,
  const unsigned int w,
  const unsigned int h)
{
  AnisotropicDiffusion *diff;
  ConnectedComponents *cc;
  unsigned int i, t;
  char buf[256];
  unsigned int *labels, lcnt;
  unsigned char *clr;
  float rgb[3], hsv[3];
  FILE *f;

  diff = new AnisotropicDiffusion (w, h, 0.12f, 0.02f);
  cc = new ConnectedComponents (w, h);

  labels = new unsigned int[w*h];
  clr = new unsigned char[w*h*4];

  for (t=0; t<maxiters; t++)
  {
    fprintf (stdout, "\rStep %i/%i...", t, maxiters);
    diff->diffusionStep (input);

    // to char
    for (i=0; i<w*h; i++)
      oup[i] = (unsigned char)(input[i]*255.0f);

    sprintf (buf, "NAD_%03i.png", t);
    PNGIO::writePixelBuffer (buf, oup, w, h, 8, 1);

    // uncomment to operate on the conduction coefficients
    for (i=0; i<w*h; i++)
      oup[i] = (unsigned char)(input[i]*255.0f);

    // do the connected components
    lcnt = cc->produceLabeling (oup, labels);

    for (i=0; i<w*h; i++)
    {
      hsv[0] = (float)((((double)labels[i])/((double)(lcnt)))*360.0);
      hsv[1] = 1.0;
      hsv[2] = 1.0;

      ColourUtils::hsv2rgb (hsv, rgb);

      clr[(i*4)+0] = static_cast<unsigned char>(rgb[0] * 255.0f);
      clr[(i*4)+1] = static_cast<unsigned char>(rgb[1] * 255.0f);
      clr[(i*4)+2] = static_cast<unsigned char>(rgb[2] * 255.0f);
      clr[(i*4)+3] = 255;
    }

    sprintf (buf, "LBL_%03i.png", t);
    PNGIO::writePixelBuffer (buf, clr, w, h, 8, 4);

    // write the labels to disk
    sprintf (buf, "LABEL-%03i.label", t);
    f = fopen (buf, "wb");
    fwrite (labels, sizeof (unsigned int), w*h, f);
    fclose (f);
  }

  fprintf (stdout, "\rCompleted %i steps\n", maxiters);

  delete diff;
  delete cc;
  delete[] labels;
}

// main
int main (int argc, char** argv)
{
  double *img;
  unsigned char *inp, *oup;
  unsigned int w, h, ct, bd, i;
  const char *image_filename;

  if (argc != 2)
  {
    fprintf (stdout, "ERR: insufficient args\n%s <input image>\n", argv[0]);
    return -1;
  }

  image_filename = argv[1];

  if (PNGIO::readPixelBuffer (image_filename, NULL, w, h, bd, ct) != PNGIO::ERR_NOERR)
  {
    fprintf (stdout, "ERR: Could not open '%s'\n", image_filename);
    return -2;
  }

  if (ct != PNGIO::GS)
  {
    fprintf (stdout, "ERR: Only greyscale images accepted\n");
    return -3;
  }

  inp = new unsigned char[w*h]; memset (inp, 0, w*h);
  oup = new unsigned char[w*h]; memset (oup, 0, w*h);
  img = new double[w*h];        memset (img, 0, w*h*sizeof (double));

  if (PNGIO::readPixelBuffer (image_filename, inp, w, h, bd, ct) != PNGIO::ERR_NOERR)
  {
    fprintf (stdout, "ERR: Could not read '%s'\n", image_filename);
  }
  else
  {
    // convert to float
    for (i=0; i<w*h; i++)  {img[i] = ((double)inp[i])/255.0f;}
    nand (img, oup, ITERS, w, h);

    // gaussian
  //  for (i=0; i<w*h; i++)  {img[i] = ((double)inp[i])/255.0f;}
  //  gauss (img, oup, ITERS, w, h);
  }

  // cleanup
  delete[] img;
  delete[] inp;
  delete[] oup;

  return 0;
}
