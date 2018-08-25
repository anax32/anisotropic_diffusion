class ImageIO
{
public:
  static int writePixelBuffer (
    const char *filename,
    const unsigned char *buffer,
    const unsigned int width,
    const unsigned int height,
    const unsigned int bitdepth,
    const unsigned int channels,
    bool flip)
  {}
  static int readPixelBuffer (
    const char *filename,
    const char *buffer,
    unsigned int& width,
    unsigned int& height,
    unsigned int& bitdepth,
    unsigned int& colourType)
  {}
};

class BMPIO : public ImageIO
{
public:
  static int writePixelBuffer (
    const char *filename,
    const unsigned char *buffer,
    const unsigned int width,
    const unsigned int height,
    const unsigned int bitdepth,
    const unsigned int channels,
    bool flip)
  {}
  static int readPixelBuffer (
    const char *filename,
    const char *buffer,
    unsigned int& width,
    unsigned int& height,
    unsigned int& bitdepth,
    unsigned int& colourType)
  {}
};
