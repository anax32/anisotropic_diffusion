#version 130

uniform sampler2D  MipTex;
uniform float    MipLevel;

void main ()
{
  // these values are constants (hence first letter is captialised)
  vec2  SrcSize = textureSize (MipTex, 0);
  vec2  DstSize = SrcSize / (MipLevel+1.0);
  vec2  SrcStep = vec2 (1.0/SrcSize.x, 1.0/SrcSize.y);
  vec2  DstStep = vec2 (1.0/DstSize.x, 1.0/DstSize.y);

  vec2  txc;
  vec2  mn, mx;  // region to sample in the large image
  float  x, y, v;
  vec4  rgba;

  txc = gl_TexCoord[0].st;    // normalised texture coord

  mn = (txc - DstStep);  // move across one pixel in the small image
  mx = (txc + DstStep);  // ditto for (+x)(+y) direction.

  // sample all the texels in the source region to find the max alpha value
  rgba = vec4 (0.0, 1.0, 0.0, 0.0);

  // FIXME: we should check (here, or on the CPU) that the region to be sampled is
  // not too large, 16x16 would muller the GPU and may exceed loop limits on older
  // hardware.
  for (y=mn.y; y<mx.y; y+=SrcStep.y)
  {
    for (x=mn.x; x<mx.x; x+=SrcStep.x)
    {
      v = texture2D (MipTex, vec2 (x, y)).r;

      rgba.r = max (rgba.r, v);
      rgba.g = min (rgba.g, v);
    }
  }
  
//  rgba.b = rgba.r-rgba.g;

  // return
  gl_FragColor = rgba;
}