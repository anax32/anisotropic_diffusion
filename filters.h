/*
=================================================
 Filters
=================================================
*/
template<typename BaseType, unsigned char SampleDimension>
class NearestNeighbour
{
public:
  static Image<BaseType, 1, SampleDimension> *resample (Image<BaseType, 1, SampleDimension> *input, unsigned int width)
  {}

  static Image<BaseType, 2, SampleDimension> *resample (Image<BaseType, 2, SampleDimension> *input, unsigned int width, unsigned int height)
  {
    unsigned int  nx, ny;
    double      xr, yr;
    unsigned int  ox, oy;
    BaseType    interpol[SampleDimension];

    Image<BaseType, 2, SampleDimension>  *output = new Image<BaseType, 2, SampleDimension>(width*height);
    output->sideLength[output->WIDTH] = width;
    output->sideLength[output->HEIGHT] = height;

#if 0
    for (unsigned int ni=0; ni<(width*height); ni++)
    {
      nx = ni%output->sideLength[output->WIDTH];  // coords of the new pixel
      ny = ni/output->sideLength[output->HEIGHT];

      xr = (double)nx/(double)output->sideLength[output->WIDTH];  // map to the unit image
      yr = (double)ny/(double)output->sideLength[output->HEIGHT];

      ox = static_cast<unsigned int>(xr*input->sideLength[input->WIDTH]);          // get the old image sample coords
      oy = static_cast<unsigned int>(yr*input->sideLength[input->HEIGHT]);

      input->samples[ox+(input->sideLength[input->WIDTH]*oy)].get (interpol);
      output->samples[ni].set (interpol);
    }
#endif
    for (ny=0; ny<height; ny++)
    {
      for (nx=0; nx<width; nx++)
      {
        xr = (double)nx/(double)width;
        yr = (double)ny/(double)height;

        ox = static_cast<unsigned int>(xr*input->sideLength[input->WIDTH]);          // get the old image sample coords
        oy = static_cast<unsigned int>(yr*input->sideLength[input->HEIGHT]);

        input->samples[(oy*input->sideLength[input->WIDTH])+ox].get (interpol);
        output->samples[(ny*width)+nx].set (interpol);
      }
    }

    return output;
  }
};

template<typename BaseType, unsigned char SampleDimension>
class LinearFilter
{
protected:
  static BaseType interpolate (BaseType p1, BaseType p2, double mu)
  {
    return static_cast<BaseType>((((double)p1)*(1-mu))+(((double)p2)*mu));
  }

public:
  static Image<BaseType, 1, SampleDimension> *resample (Image<BaseType, 1, SampleDimension> *input, unsigned int width)
  {}

  static Image<BaseType, 2, SampleDimension> *resample (Image<BaseType, 2, SampleDimension> *input, unsigned int width, unsigned int height)
  {
    unsigned int  nx, ny;
    double      xr, yr;
    unsigned int  ox, oy;
    BaseType    interpol[SampleDimension];
    BaseType    a, b, c, d;

    Image<BaseType, 2, SampleDimension>  *output = new Image<BaseType, 2, SampleDimension>(width*height);
    output->sideLength[output->WIDTH] = width;
    output->sideLength[output->HEIGHT] = height;

  //  for (unsigned int ni=0; ni<(width*height); ni++)
    for (unsigned int ny=0; ny<height; ny++)
    {
      for (unsigned int nx=0; nx<width; nx++)
      {
    //  nx = ni%output->sideLength[output->WIDTH];  // coords of the new pixel
    //  ny = ni/output->sideLength[output->HEIGHT];

        xr = (double)nx/(double)output->sideLength[output->WIDTH];  // map to the unit image
        yr = (double)ny/(double)output->sideLength[output->HEIGHT];

        ox = static_cast<unsigned int>(xr*(input->sideLength[input->WIDTH]-1));          // get the old image sample coords
        oy = static_cast<unsigned int>(yr*(input->sideLength[input->HEIGHT]-1));

        // get the four values from the old image and iterpolate
        for (unsigned char ch=0; ch<SampleDimension; ch++)
        {
          a = input->samples[ox+(input->sideLength[input->WIDTH]*oy)].get (ch);
          b = input->samples[ox+(input->sideLength[input->WIDTH]*(oy+1))].get (ch);

          c = input->samples[(ox+1)+(input->sideLength[input->WIDTH]*oy)].get (ch);
          d = input->samples[(ox+1)+(input->sideLength[input->WIDTH]*(oy+1))].get (ch);

          interpol[ch] = interpolate (interpolate (a, b, xr), interpolate (c,d, xr), yr);
        }

      //  output->samples[ni].set (interpol);
        output->samples[(ny*output->sideLength[output->WIDTH])+nx].set (interpol);
      }
    }

    return output;
  }
  static Image<BaseType, 3, SampleDimension> *resample (Image<BaseType, 3, SampleDimension> *input, unsigned int width, unsigned int height, unsigned int depth)
  {}
};