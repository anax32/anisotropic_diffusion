/**
=================================================
 Diffusion interface
=================================================
*/
class Diffusion
{
protected:
  const unsigned int  w;    ///< image width
  const unsigned int  h;    ///< image height

public:
  /**
   * constructor
   * @param width x dimension of the image to be processed (or set of images of uniform size)
   * @param height y dimension of the image
   */
  Diffusion (unsigned int width, unsigned int height)
    : w (width), h (height)
  {}
  /**
   * empty destructor
   */
  virtual ~Diffusion ()
  {}
  /**
   * implementation interface
   * @param buf image to process, should be written over
   */
  virtual void diffusionStep (double *buf) = 0;
};
/**
=================================================
 Gaussian Diffusion (AKA gaussian blur)
=================================================
*/
class GaussianDiffusion : public Diffusion
{
protected:
  unsigned int  ks;      ///< kernel size
  double      *kernel;  ///< kernel description
  double      *box;    ///< local cache of the image under the kernel
  double      *hpass;    ///< local cache of the horizontal pass

  void allocKernel ()
  {
    unsigned int  i;

    assert (kernel == NULL);
    kernel = new double[ks];  // 1D
    memset (kernel, 0, ks*sizeof (double));

#if 0
    for (i=0; i<ks; i++)
      kernel[i] = 1.0/(double)ks;  // box filter for now
#else
    int    gs;
    double  s;

    // gaussian distribution
    for (i=0, s=0.0; i<ks; i++)
    {
      gs = (-((int)ks>>1))+i;
      s += kernel[i] = (1.0 / sqrt (2.0*M_PI)) * pow (M_E, -((gs*gs)/2.0));
    }

    // normalise
    for (i=0; i<ks; i++)
    {
      kernel[i] /= s;
    }
#endif
  }

  void cleanKernel ()
  {
    if (kernel != NULL)
    {
      delete[] kernel;
      kernel = NULL;
    }
  }
  void allocBox ()
  {
    assert (box == NULL);
    box = new double[ks];
    memset (box, 0, ks*sizeof (double));
  }
  void cleanBox ()
  {
    if (box != NULL)
    {
      delete[] box;
      box = NULL;
    }
  }
  void allocPassBuffers ()
  {
    assert (hpass == NULL);
    hpass = new double[w*h];
    memset (hpass, 0, w*h*sizeof (double));
  }
  void cleanPassBuffers ()
  {
    if (hpass != NULL)
    {
      delete[] hpass;
      hpass = NULL;
    }
  }
public:
  /**
   * constructor, builds the kernel and local cache
   * @param width target image width
   * @param height target image height
   * @param kernelSize size of the gaussian kernels in pixels (in one dimension, kernel is assumed to be square)
   */
  GaussianDiffusion (const unsigned int width, const unsigned int height, const unsigned int kernelSize)
    : Diffusion (width, height), ks (kernelSize),
    kernel (NULL), box (NULL), hpass (NULL)
  {
    allocKernel ();
    allocBox ();
  }
  /**
   * Destructor, memory cleanup
   */
  ~GaussianDiffusion ()
  {
    cleanKernel ();
    cleanBox ();
  }
  /**
   * Performs a single diffusion step on the input image
   * @param buf buffer containing the input image and will contain the output after the function completes
   * @todo this function only processes greyscale images ATM
   */
  void diffusionStep (double *buf)
  {
    const unsigned int  d = 1;  // only greyscale images for now
    unsigned int  c;  // channel (iterates over d)
    unsigned int  x, y;
    int p, pp;
    double      v;  // new pixel value

    allocPassBuffers ();

    // do the horizontal blur
    for (c=0; c<d; c++)
    {
      for (y=0; y<h; y++)
      {
        for (x=0; x<w; x++)
        {
          // clear the box
          memset (box, 0, ks*sizeof (double));

          // fill the box (horizontal values)
          for (p=(int)x - (int)(ks>>1), pp=0; p<(int)((x - (int)(ks>>1))+ks); p++, pp++)
          {
            if (p<0)
              box[pp] = buf[(y*w*d)+(0*d)+c];
            else if (p>(int)(w-1))
              box[pp] = buf[(y*w*d)+((w-1)*d)+c];
            else
              box[pp] = buf[(y*w*d)+(p*d)+c];
          }

          // convolve
          for (v=0.0, pp=0; pp<ks; pp++)
          {
            v += box[pp] * kernel[pp];
          }

          // write
          hpass[(y*w*d)+(x*d)+c] = v;
        }
      }
    }

    // vertical blur (written to img)
    for (c=0; c<d; c++)
    {
      for (x=0; x<w; x++)
      {
        for (y=0; y<h; y++)
        {
          // clear the box
          memset (box, 0, ks*sizeof (double));

          // fill the box (horizontal values)
          for (p=(int)y - (int)(ks>>1), pp=0; p<(int)((y - (ks>>1))+ks); p++, pp++)
          {
            if (p<0)
              box[pp] = hpass[(0*w*d)+(x*d)+c];
            else if (p>(int)(h-1))
              box[pp] = hpass[((h-1)*w*d)+(x*d)+c];
            else
              box[pp] = hpass[(p*w*d)+(x*d)+c];
          }

          // convolve
          for (v=0.0f, pp=0; pp<ks; pp++)
          {
            v += box[pp] * kernel[pp];
          }

          // write
          buf[(y*w*d)+(x*d)+c] = v;
        }
      }
    }

    cleanPassBuffers ();
  }

  /**
   * change the kernel size
   * @param newSize new width of the kernel
   */
  void setKernelSize (const unsigned int newSize)
  {
    ks = newSize;

    cleanKernel ();
    cleanBox ();

    allocKernel ();
    allocBox ();
  }
  /**
   * retrieve the kernel size
   * @return current kernel width
   */
  const unsigned int getKernelSize () const
  {
    return ks;
  }
};
/*
=================================================
 Non-linear Anisotropic diffusion

 For more information see:
  P. Perona and J. Malik,
  Scale space and edge detection using anisotropic diffusion,
  IEEE Transactions on Pattern Analysis and Machine Intelligence 12:629-639, 1990.
=================================================
*/
class AnisotropicDiffusion : public Diffusion
{
public:
  static const unsigned int  K = 4;  ///< connectivity of pixels
  const float      lambda;  ///< max value of .25 for stability
  const float      kappa;  ///< conduction coefficient (20-100?)
  const double    *img;  ///< pointer to the image buffer (is this used?)

protected:
  double  *g;  ///< gradient image
  double  *c;  ///< conductivity image

  void setImage (const double *inputImage)
  {
    img = inputImage;
  }
  void allocEdgeImage ()
  {
    g = new double[w*h*K];
    memset (g, 0, w*h*K*sizeof (double));
  }
  void cleanEdgeImage ()
  {
    if (g != NULL)
    {
      delete[] g;
      g = NULL;
    }
  }
  /**
   * Produce a gradient image of the input via central differences.
   * @todo this should be an external class passed as a parameter
   */
  void createEdgeImage ()
  {
    unsigned int  x, y, i;
    double      v, d[K];

    assert (g != NULL);

    memset (g, 0, w*h*K*sizeof (double));

    for (y=1; y<h-1; y++)
    {
      for (x=1; x<w-1; x++)
      {
        v = img[(y*w)+x];

        d[0] = img[((y+1)*w)+x];
        d[1] = img[((y-1)*w)+x];
        d[2] = img[(y*w)+(x+1)];
        d[3] = img[(y*w)+(x-1)];
        // d[4]..[7]

        // compute the differences and update
        for (i=0; i<K; i++)
        {
          g[(y*w*K)+(x*K)+i] = d[i] - v;
        }
      }
    }
  }
  void allocCondCoeffs ()
  {
    c = new double[w*h*K];
    memset (c, 0, w*h*K*sizeof (double));
  }
  void cleanCondCoeffs ()
  {
    if (c != NULL)
    {
      delete[] c;
      c = NULL;
    }
  }
  /**
   * Calculate the conduction coefficients from the gradient image and current parameters
   */
  void createCondCoeffs ()
  {
    const unsigned int  elcnt = w*h*K;
    unsigned int    i;

    // copy the edge image
    for (i=w*K; i<elcnt; i++)
    {
      // apply the diffusion equation
    //  c[i] = exp (-((g[i]/kappa)*(g[i]/kappa)));
      c[i] = 1.0 / (1.0 + ((g[i]/kappa)*(g[i]/kappa)));
    }
  }
public:
  /**
   * constructor
   * @param width target image width
   * @param height target image height
   * @param lambda  see perona and malik
   * @param kappa    see perona and malik
   */
  AnisotropicDiffusion (
    unsigned int width,
    unsigned int height,
    const float lambdaValue = 0.25f,
    const float kappaValue = 0.2f)
    : Diffusion (width, height),
      g (NULL),
      lambda (lambdaValue),
      kappa (kappaValue)
  {
    allocEdgeImage ();
    allocCondCoeffs ();
  }
  /**
   * destructor, memory cleanup
   */
  ~AnisotropicDiffusion ()
  {
    cleanEdgeImage ();
    cleanCondCoeffs ();
  }
  /**
   * perform a single diffusion step on the input image.
   * After the function executes, the image will be blurred along areas of low gradient,
   * areas of high gradient (edges) will be preserved.
   * @param buf buffer containing the input image, and will contain the output
   */
  void diffusionStep (double *buf)
  {
    setImage (buf);

    // produce the gradient image (FIXME: make this a parameter call)
    createEdgeImage ();
    createCondCoeffs ();

    // update the buffer
    unsigned int  x, y, i;
    double      v, gr[K], cf[K], cont;

    for (y=0; y<h; y++)
    {
      for (x=0; x<w; x++)
      {
        v = buf[(y*w)+x];

        for (i=0, cont=0.0f; i<K; i++)
        {
          gr[i] = g[(y*w*K)+(x*K)+i];
          cf[i] = c[(y*w*K)+(x*K)+i];
          cont += cf[i]*gr[i];
        }

        v += lambda * cont;

        buf[(y*w)+x] = v;
      }
    }
  }
  /**
   * debug function, reads the edge image and formats it into buf
   * @param buf buffer to contain the edge magnitude image
   */
  void getEdgeMagnitudes (unsigned char *buf)
  {
    unsigned int  x, y, i;
    double      m;
    double      mn, mx;

    mn = FLT_MAX;
    mx = -FLT_MAX;

    for (y=0; y<h; y++)
    {
      for (x=0; x<w; x++)
      {
        m = 0.0f;

        for (i=0; i<K; i++)
        {
          m += abs (g[(y*w*K)+(x*K)+i]);
        }

        if (mn > m)
          mn = m;
        if (mx < m)
          mx = m;

      //  buf[(y*w)+x] = (unsigned char)(m*255.0f);
      }
    }

    for (y=0; y<h; y++)
    {
      for (x=0; x<w; x++)
      {
        m = 0.0f;

        for (i=0; i<K; i++)
          m += abs (g[(y*w*K)+(x*K)+i]);

        buf[(y*w)+x] = (unsigned char)(((m-mn)/(mx-mn))*255.0f);
      }
    }
  }
  /**
   * debug function to get the conduction coefficients
   * @param buf buffer to accept the coefficient image
   */
  void getConductionCoefficients (unsigned char *buf)
  {
    unsigned int  x, y, i;
    double      m;
    double      mn, mx;

    mn = FLT_MAX;
    mx = -FLT_MAX;

    for (y=0; y<h; y++)
    {
      for (x=0; x<w; x++)
      {
        m = 0.0f;

        for (i=0; i<K; i++)
        {
          m += abs (c[(y*w*K)+(x*K)+i]);
        }

        if (mn > m)
          mn = m;
        if (mx < m)
          mx = m;
      }
    }

    for (y=0; y<h; y++)
    {
      for (x=0; x<w; x++)
      {
        m = 0.0f;

        for (i=0; i<K; i++)
          m += abs (c[(y*w*K)+(x*K)+i]);

        buf[(y*w)+x] = (unsigned char)(((m-mn)/(mx-mn))*255.0f);
      }
    }
  }
};
