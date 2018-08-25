#ifndef COLOUR_UTILS_H
#define COLOUR_UTILS_H

#include <math.h>
#include <float.h>
#include <sstream>
#include <iomanip>

namespace ColourUtils
{
  /*
   * sets
   */
  void set (float* rgba, const float r, const float g, const float b, const float a)
  {
    rgba[0] = r;
    rgba[1] = g;
    rgba[2] = b;
    rgba[3] = a;
  }
  /*
   * colourspace conversion
   */
  // rgb and hsv
  void hsv2rgb (float h, float s, float v, float* r, float* g, float* b)
  {
    unsigned int  hi = ((int)(h/60))%6;
    float      f = h/60 - floor (h/60);
    float      p = v * (1 - s);
    float      q = v * (1 - f * s);
    float      t = v * (1 - (1 - f) * s);

    switch (hi)
    {
    case 0: *r=v; *g=t; *b=p; break;
    case 1: *r=q; *g=v; *b=p; break;
    case 2: *r=p; *g=v; *b=t; break;
    case 3: *r=p; *g=q; *b=v; break;
    case 4: *r=t; *g=p; *b=v; break;
    case 5: *r=v; *g=p; *b=q; break;
    }
  }
  void hsv2rgb (float hsv[3], float rgb[3])
  {
    hsv2rgb (hsv[0], hsv[1], hsv[2], &rgb[0], &rgb[1], &rgb[2]);
  }
  void hsv2rgb (float h, float s, float v, float rgb[3])
  {
    hsv2rgb (h, s, v, &rgb[0], &rgb[1], &rgb[2]);
  }
  void rgb2hsv (float rgb[3], float hsv[3])
  {
    unsigned int  i;
    float  mx = -FLT_MAX, mn = FLT_MAX;

    for (i=0; i<3; i++)
    {
      if (mx < rgb[i])
        mx = rgb[i];
      if (mn > rgb[i])
        mn = rgb[i];
    }

    // do the hue
    if (mx == mn)
      hsv[0] = 0.0f;
    else if (mx == rgb[0])
      hsv[0] = (float)((int)(60.0f * ((rgb[1] - rgb[2])/(mx-mn))+360)%360);
    else if (mx == rgb[1])
      hsv[0] = 60.0f * ((rgb[2] - rgb[0])/(mx-mn))+120.0f;
    else if (mx == rgb[2])
      hsv[0] = 60.0f * ((rgb[0] - rgb[1])/(mx-mn))+240.0f;

    // sat
    if (mx == 0.0f)
      hsv[1] = 0.0f;
    else
      hsv[1] = 1.0f - (mn/mx);

    // v
    hsv[2] = mx;
  }

  // rgb and yuv
  void rgb2yuv (float rgb[3], float yuv[3])
  {
    const float  w[3] = {0.299f, 0.114f, 0.587f};
    const float  umx = 0.436f;
    const float  vmx = 0.615f;

    yuv[0] = w[0]*rgb[0] + w[1]*rgb[1] + w[2]*rgb[2];
    yuv[1] = umx * ((rgb[2]-yuv[0])/(1.0f-w[2]));
    yuv[2] = vmx * ((rgb[0]-yuv[0])/(1.0f-w[0]));
  }
  /*
   * encoding conversion
   */
  void float2hex (const float *rgba, std::string& hex)
  {
    unsigned int  i;
    unsigned char  c;
    char      u,l;

    hex.clear ();
    hex.reserve (9);
    hex.push_back ('#');

    for (i=0;i<4;i++)
    {
      c = static_cast<unsigned char>(rgba[i]*255.0f);
      u = c/16;
      l = c%16;

      if (u<10)
        hex.push_back ('0'+u);
      else
        hex.push_back ('A'+(u-10));

      if (l<10)
        hex.push_back ('0'+l);
      else
        hex.push_back ('A'+(l-10));
    }
  }
  std::string float2hex (const float *rgba)
  {
    std::string  h;
    float2hex (rgba, h);
    return h;
  }
  unsigned char hex2int (unsigned char h)
  {
    if ((h>='0')&&(h<='9'))  {return h-'0';}
    if ((h>='A')&&(h<='F'))  {return 10 + (h-'A');}
    return -1;
  }
  bool hex2float (float *rgba, const std::string& hex)
  {
    unsigned int  i;
    char      u, l;

    if (hex.length () == 0)      {return false;}

    for (i=0;i<4;i++)
    {
      u = hex.at (1+((i*2)+0));
      l = hex.at (1+((i*2)+1));

      u = hex2int (u);
      l = hex2int (l);

      rgba[i] = ((static_cast<float>(u)*16.0f)+static_cast<float>(l))/255.0f;
    }

    return true;
  }

  // blending
  void blend (const float *c1, const float *c2, float *oc, const float t)
  {
    unsigned int  i;

    for (i=0;i<4;i++)
    {
      oc[i] = c1[i]+(c2[i]-c1[i])*t;
    }
  }
};


#endif
