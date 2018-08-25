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
  const unsigned int maxiters,
  const unsigned int w,
  const unsigned int h)
{
  GaussianDiffusion *diff;
  unsigned int i, t;

  diff = new GaussianDiffusion (w, h, 5);

  for (t=0; t<maxiters; t++)
  {
    fprintf (stdout, "\riteration %03i/%03i...", t, maxiters);
    fflush (stdout);

    diff->diffusionStep (input);
  }

  fprintf (stdout, "\rcompleted %i steps\n", maxiters);

  delete diff;
}

// non-linear anisotropic diffusion
void nand (
  double *input,
  const unsigned int maxiters,
  const unsigned int w,
  const unsigned int h)
{
  AnisotropicDiffusion *diff = NULL;
  unsigned int i, t;
  char buf[256];

  diff = new AnisotropicDiffusion (w, h, 0.12f, 0.02f);

  for (t=0; t<maxiters; t++)
  {
    fprintf (stdout, "\riteration %03i/%03i...", t, maxiters);
    fflush (stdout);
    diff->diffusionStep (input);
  }

  fprintf (stdout, "\rcompleted %i steps\n", maxiters);

  delete diff;
}

void connected_components (
  unsigned char *input,
  const unsigned int w,
  const unsigned int h)
{
  ConnectedComponents *cc = NULL;
  unsigned int label_count;
  unsigned int i;
  unsigned int *labels = NULL;
  unsigned char *clr = NULL;
  float rgb[3], hsv[3];

  cc = new ConnectedComponents (w, h);
  labels = new unsigned int[w*h];

  // do the connected components
  label_count = cc->produceLabeling (input, labels);

  for (i=0; i<w*h; i++)
  {
    hsv[0] = (float)((((double)labels[i])/((double)(label_count)))*360.0);
    hsv[1] = 1.0f;
    hsv[2] = 1.0f;

    ColourUtils::hsv2rgb (hsv, rgb);

    input[(i*4)+0] = static_cast<unsigned char>(rgb[0] * 255.0f);
    input[(i*4)+1] = static_cast<unsigned char>(rgb[1] * 255.0f);
    input[(i*4)+2] = static_cast<unsigned char>(rgb[2] * 255.0f);
    input[(i*4)+3] = 255;
  }

  // write the labels to disk
/*
  FILE *f;

  sprintf (buf, "LABEL-%03i.label", t);
  f = fopen (buf, "wb");
  fwrite (labels, sizeof (unsigned int), w*h, f);
  fclose (f);
*/
  delete cc;
  delete[] labels;
  delete[] clr;
}

class app_state_t
{
public:
  const char *image_filename;
  const char *output_filename;
  unsigned int iterations;

  bool aniso_diffusion;
  bool iso_diffusion;
  bool connected_components;

public:
  app_state_t ()
  {
    image_filename = NULL;
    output_filename = NULL;
    iterations = 1;
    aniso_diffusion = false;
    iso_diffusion = false;
    connected_components = false;
  }

  bool is_valid ()
  {
    if ((image_filename == NULL) ||
        (output_filename == NULL) ||
        (iterations == 0) ||
        ((aniso_diffusion == false) && (iso_diffusion == false) && (connected_components == false)) ||
        ((aniso_diffusion == true) && (iso_diffusion == true)))
    {
      // FIXME: check we only have one of aniso, iso and ccoms
      return false;
    }

    return true;
  }

  void help (const char *app_name)
  {
    fprintf (stdout, "%s : -i <input image filename> -o <output image filename> -n <number iterations> -a -g\n");
  }

  void parse_args (int argc, char **argv)
  {
    for (int i=0; i<argc; i++)
    {
      if ((strcmp (argv[i], "-i") == 0) ||
          (strcmp (argv[i], "--input") == 0))
      {
        image_filename = argv[++i];
      }
      else if ((strcmp (argv[i], "-o") == 0) ||
               (strcmp (argv[i], "--output") == 0))
      {
        output_filename = argv[++i];
      }
      else if ((strcmp (argv[i], "-n") == 0) ||
               (strcmp (argv[i], "--iterations") == 0))
      {
        iterations = atoi (argv[++i]);
      }
      else if ((strcmp (argv[i], "-a") == 0) ||
               (strcmp (argv[i], "-aniso") == 0))
      {
        aniso_diffusion = true;
      }
      else if ((strcmp (argv[i], "-g") == 0) ||
               (strcmp (argv[i], "--iso") == 0))
      {
        iso_diffusion = true;
      }
      else if ((strcmp (argv[i], "-h") == 0) ||
               (strcmp (argv[i], "--help") == 0))
      {
        help (argv[0]);
      }
      else if ((strcmp (argv[i], "-c") == 0) ||
               (strcmp (argv[i], "--cc") == 0))
      {
        connected_components = true;
      }
    }
  }
};

// main
int main (int argc, char** argv)
{
  unsigned int w, h, ct, bd, i;
  double *img;
  unsigned char *inp, *oup;
  app_state_t app_state;

  app_state.parse_args (argc, argv);

  if (app_state.is_valid () == false)
  {
    fprintf (stdout, "ERR: invalid parameter combination\n");
    return -1;
  }

  if (PNGIO::readPixelBuffer (
        app_state.image_filename,
        NULL, w, h, bd, ct) != PNGIO::ERR_NOERR)
  {
    fprintf (stdout, "ERR: Could not open '%s'\n", app_state.image_filename);
    return -2;
  }

  if (ct != PNGIO::GS)
  {
    fprintf (stdout, "ERR: Only greyscale images accepted\n");
    return -3;
  }

  fprintf (stdout, "allocating 2x%i bytes for images\n", w*h*sizeof (unsigned char));
  fprintf (stdout, "allocating %i bytes for buffer\n", w*h*sizeof (double));

  inp = new unsigned char[w*h]; memset (inp, 0, w*h);
  img = new double[w*h];        memset (img, 0, w*h*sizeof (double));

  fprintf (stdout, "reading image data...\n");

  if (PNGIO::readPixelBuffer (app_state.image_filename, inp, w, h, bd, ct) != PNGIO::ERR_NOERR)
  {
    fprintf (stdout, "ERR: Could not read '%s'\n", app_state.image_filename);
  }
  else
  {
    // convert to float
    fprintf (stdout, "converting to double...\n");

    for (i=0; i<w*h; i++)
    {
      img[i] = ((double)inp[i])/255.0f;
    }

    if (app_state.aniso_diffusion)
    {
      fprintf (stdout, "running anisotropic-diffusion...\n");
      nand (img, app_state.iterations, w, h);
      fprintf (stdout, "complete\n");
    }

    // gaussian
    if (app_state.iso_diffusion)
    {
      fprintf (stdout, "running isotropic-diffusion...\n");
      gauss (img, app_state.iterations, w, h);
      fprintf (stdout, "complete\n");
    }

    if (app_state.connected_components)
    {
      fprintf (stdout, "running connected components...\n");
      connected_components (inp, w, h);
      fprintf (stdout, "complete\n");

      // this is just to avoid a branch later... bit ugly
      for (i=0; i<w*h; i++)
        img[i] = (double)(inp[i])/255.0;
    }
  }

  fprintf (stdout, "saving to '%s'\n", app_state.output_filename);

  for (i=0; i<w*h; i++)
    inp[i] = (unsigned char)(img[i]*255.0f);

  PNGIO::writePixelBuffer (app_state.output_filename, inp, w, h, 8, 1);

  // cleanup
  fprintf (stdout, "cleanup\n");
  delete[] img;
  delete[] inp;
  fprintf (stdout, "complete\n");

  return 0;
}
