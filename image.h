#ifndef IMAGE_H
#define IMAGE_H
#if 0
#include <assert.h>
template<class BaseType, unsigned char Dimension>
class Sample
{
public:
  BaseType  v[Dimension];

  void set (const BaseType *buffer)
  {
    for (int i=0; i<Dimension; i++, buffer++)
      v[i] = *buffer;
  }

  void set (const unsigned char d, BaseType t)
  {
    v[d] = t;
  }

  void get (BaseType *buffer) const
  {
    for (int i=0; i<Dimension; i++, buffer++)
      *buffer = v[i];
  }

  BaseType get (unsigned char d) const
  {
    return v[d];
  }
};

template<class BaseType, unsigned char ImageDimension, unsigned char SampleDimension>
class Image
{
public:
  typedef enum Dimensions
  {
    WIDTH = 0, HEIGHT = 1, DEPTH = 2
  };

protected:
  unsigned int  size[ImageDimension];    // 1D = length, 2D = <width, height>, 3D = <width, height, depth>
  unsigned int  sampleCnt;

  void allocSamples ()
  {
    assert (samples == NULL);

    if (sampleCnt > 0)
      samples = new Sample<BaseType, SampleDimension>[sampleCnt];
  }

  void cleanSamples ()
  {
    if (samples != NULL)
    {
      delete[] samples;
      samples = NULL;
    }
  }

public:
  Sample<BaseType, SampleDimension>  *samples;

  Image (unsigned int sampleCount)
    : sampleCnt (sampleCount), samples (NULL)
  {
    setSampleCount (sampleCnt);
  }

  ~Image ()
  {
    setSampleCount (0);
    assert (samples == NULL);
  }

  void setSampleCount (const unsigned int sampleCount)
  {
    if (sampleCount != sampleCnt)
    {
      cleanSamples ();
      sampleCnt = sampleCount;
      allocSamples ();
    }
  }

  void setSize (Dimensions dimension, const unsigned int dsize)
  {
    size[dimension] = dsize;
  }

  const unsigned int getSize (Dimensions dimension) const
  {
    return size[dimension];
  }

  Sample<BaseType, SampleDimension>& getSample (const unsigned int sampleIndex)
  {
    return samples[sampleIndex];
  }
};
#endif
#endif