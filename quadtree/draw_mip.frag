uniform sampler2D	mipTex;
uniform float		mipLevel;

void main ()
{
  gl_FragColor = texture2DLod (mipTex, gl_TexCoord[0].st, mipLevel);
}
