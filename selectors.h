#include "Image.h"

class Selectors
{
public:
  template<BaseType, 2, SampleDimension>
  static void getSubImage (
    Image<BaseType, 2, SampleDimension>& sourceImage,
    Image<BaseType, 2, SampleDimension>& destinationImage,
    unsigned int xmin,
    unsigned int ymin,
    unsigned int xmax,
    unsigned int ymax)
  {
    const unsigned int  dw = (xmax-xmin);
    const unsigned int  dh = (ymax-ymin);
    const unsigned int  sw = sourceImage.getSize (sourceImage.WIDTH);
    const unsigned int  sh = sourceImage.getSize (sourceImage.HEIGHT);
    unsigned int    x, y, sidx;
    Sample&        smpl;
    Sample&        dmpl;

  //  Image<BaseType, ImageDimension, SampleDimension> *subImage = new Image<BaseType, ImageDimension, SampleDimension> (sampleCount);
    destinationImage.setSampleCount (dw*dh);
    destinationImage.setSize (destinationImage.WIDTH, dw);
    destinationImage.setSize (destinationImage.HEIGHT, dh);

    // copy the data over
    for (y=ymin; y<ymax; y++)
    {
      for (x=xmin; x<xmax; x++)
      {
        sidx = (y*sw)+x;
        didx = ((y-ymin)*dw)+(x-xmin);

        smpl = sourceImage.getSample (sidx);
        dmpl = destinationImage.getSample (didx);

        dmpl.set (&smpl.v);
      }
    }
  }

  template<BaseType, 3, SampleDimension>
  static void getSpatialPlane (
    Image<BaseType, 3, SampleDimension>& sourceImage,
    Image<BaseType, 2, SampleDimension>& destinationImage,
    const Image<BaseType, 3, SampleDimensions>::Dimensions targetDimension,
    const unsigned int  planeIndex)
  {
    unsigned int  dw = (xmax-xmin);
    unsigned int  dh = (ymax-ymin);
    unsigned int  sw = sourceImage.getSize (sourceImage.WIDTH);
    unsigned int  sh = sourceImage.getSize (sourceImage.HEIGHT);
    unsigned int  x, y, z, i, sidx;
    Sample&      smpl;
    Sample&      dmpl;

    switch (targetDimension)
    {
      case sourceImage.WIDTH:
      {
        sw = sourceImage.getSize (sourceImage.HEIGHT);
        sh = sourceImage.getSize (sourceImage.DEPTH);
        break;
      }
      case sourceImage.HEIGHT:
      {
        sw = sourceImage.getSize (sourceImage.WIDTH);
        sh = sourceImage.getSize (sourceImage.DEPTH);
        break;
      }
      case sourceImage.DEPTH:
      {
        sw = sourceImage.getSize (sourceImage.WIDTH);
        sh = sourceImage.getSize (sourceImage.HEIGHT);
        break;
      }
      default:
        return;
    }

    destinationImage.setSampleCount (sw*sh);
    destinationImage.setSize (destinationImage.WIDTH, sw);
    destinationImage.setSize (destinationImage.HEIGHT, sh);

    switch (targetDimension)
    {
      case sourceImage.WIDTH:
      {
        for (z=0; z<sourceImage.getSize (sourceImage.DEPTH); z++)
        {
          for (y=0; y<sourceImage.getSize (sourceImage.HEIGHT); y++)
          {
          //  for (x=0; x<sourceImage.getSize (sourceImage.WIDTH); x++)
            {

            }
          }
        }
      }
  }
}
