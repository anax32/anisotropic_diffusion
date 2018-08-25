#pragma comment(lib, "../../Image/PNG/libpng13.lib")  // libpng

#define NOMINMAX
#include <windows.h>                  // windows
#include "../../Utils/Win/Utils.h"
#include "../../Utils/Win/Console.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <assert.h>

// opengl
#define _GLUTILS_DEF_LIBS_
#define _GLUTILS_USE_GLEW_
#include "../../Utils/GL/GLUtils.h"

#include "../../Image/pngreader.h"            // libpng

#define  FILE_OPEN  1
#define  FILE_CLOSE  2
#define FILE_EXIT  3
#define MIPMAP_GEN  10
#define SHADER_LOAD  11
#define SHADER_DELT 12

#define TEST_IMAGE  "C:\\Users\\csed\\Documents\\Projects\\Image\\Quadtree\\geo_sm1.png\0"
//#define TEST_IMAGE  "C:\\Users\\csed\\Documents\\Projects\\Image\\Quadtree\\geo_sm2.png\0"
//#define TEST_IMAGE  "C:\\Users\\csed\\Documents\\Projects\\Image\\Quadtree\\geo_sm3.png\0"

/*===============================================
 Mipmap building
===============================================*/
unsigned int buildGLMipmaps (const unsigned int texid)
{
  unsigned char  *img;
  unsigned int  newtexid;
  int  w, h;
  unsigned int  t;

  // read the image
  glBindTexture (GL_TEXTURE_2D, texid);
  glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

  img = new unsigned char[w*h];
  glGetTexImage (GL_TEXTURE_2D, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, img);

  // upload the new texture with new parameters
  glGenTextures (1, &newtexid);
  glBindTexture (GL_TEXTURE_2D, newtexid);

  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, img);

//  glHint (GL_GENERATE_MIPMAP_HINT, GL_NICEST);
//  glHint (GL_GENERATE_MIPMAP_HINT, GL_FASTEST);
  glHint (GL_GENERATE_MIPMAP_HINT, GL_DONT_CARE);

  t = clock ();
  glGenerateMipmapEXT (GL_TEXTURE_2D);
  t = clock () - t;
  fprintf (stdout, "glGenerateMipmap () in %0.6fs\n\0", ((float)t)/CLOCKS_PER_SEC);

  delete[] img;

  return newtexid;
}

unsigned int buildMuMipmaps (const unsigned int texid, unsigned int *shaderProfile)
{
#define MAX_LODS  10

  unsigned int  newtexid, i, l;
  int        w, h;
  unsigned int  fbo;
  unsigned int  t;

  // read the image
  glBindTexture (GL_TEXTURE_2D, texid);
  glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

  unsigned char  *img = new unsigned char[w*h];
  glGetTexImage (GL_TEXTURE_2D, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, img);

  // create the new texture for storage
  glGenTextures (1, &newtexid);
  glBindTexture (GL_TEXTURE_2D, newtexid);
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, img);
  glGenerateMipmapEXT (GL_TEXTURE_2D);

  delete[] img;

  // upload via the fragment program
  glGenFramebuffersEXT (1, &fbo);
  glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, fbo);

  // start the loop
  glPushAttrib (GL_VIEWPORT_BIT);
  glMatrixMode (GL_PROJECTION);  glPushMatrix ();  glLoadIdentity (); glOrtho (0,1,0,1,0,1);
  glMatrixMode (GL_MODELVIEW);  glPushMatrix ();  glLoadIdentity ();

  glUseProgram (shaderProfile[2]);
  l = glGetUniformLocation (shaderProfile[2], "MipLevel\0");
  glUniform1i (glGetUniformLocation (shaderProfile[2], "MipTex\0"), 0);

  glEnable (GL_TEXTURE_2D);
  glBindTexture (GL_TEXTURE_2D, texid);
  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  t = clock ();

  for (i=0; i<MAX_LODS; i++)
  {
    glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, newtexid, i);
    glUniform1f (l, (float)i);

    glViewport (0, 0, w>>i, h>>i);

    glBegin (GL_QUADS);
    glTexCoord2i (0, 0);  glVertex2i (0, 0);
    glTexCoord2i (0, 1);  glVertex2i (0, 1);
    glTexCoord2i (1, 1);  glVertex2i (1, 1);
    glTexCoord2i (1, 0);  glVertex2i (1, 0);
    glEnd ();
  }

  t = clock () - t;
  fprintf (stdout, "FBO Mipmaps in %0.6fs\n\0", ((float)t)/CLOCKS_PER_SEC);

  // end
  glDisable (GL_TEXTURE_2D);
  glUseProgram (0);
  glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
  glPopAttrib ();
  glMatrixMode (GL_PROJECTION);  glPopMatrix ();
  glMatrixMode (GL_MODELVIEW);  glPopMatrix ();

  // return
  return newtexid;
}
/*===============================================
 Drawing
===============================================*/
void drawTextures (const unsigned int texid, const unsigned int glMipmap)
{
  int    w, h;

  glBindTexture (GL_TEXTURE_2D, texid);
  glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

  glEnable (GL_TEXTURE_2D);
  glBindTexture (GL_TEXTURE_2D, texid);
  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glBegin (GL_QUADS);
  glTexCoord2i (0, 1);  glVertex2i (0, 0);
  glTexCoord2i (0, 0);  glVertex2i (0, h>>1);
  glTexCoord2i (1, 0);  glVertex2i (w>>1, h>>1);
  glTexCoord2i (1, 1);  glVertex2i (w>>1, 0);
  glEnd ();
  glDisable (GL_TEXTURE_2D);

  glEnable (GL_TEXTURE_2D);
  glBindTexture (GL_TEXTURE_2D, glMipmap);
  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glBegin (GL_QUADS);
  glTexCoord2i (0, 1);  glVertex2i (w>>1, 0);
  glTexCoord2i (0, 0);  glVertex2i (w>>1, h>>1);
  glTexCoord2i (1, 0);  glVertex2i (w, h>>1);
  glTexCoord2i (1, 1);  glVertex2i (w, 0);
  glEnd ();
  glDisable (GL_TEXTURE_2D);
}
#define MIP_VERT  "C:\\Users\\csed\\Documents\\Projects\\Image\\Quadtree\\fixed_func.vert\0"
#define MIP_FRAG  "C:\\Users\\csed\\Documents\\Projects\\Image\\Quadtree\\draw_mip.frag\0"
#define MUP_FRAG  "C:\\Users\\csed\\Documents\\Projects\\Image\\Quadtree\\mumip.frag\0"
#define MAX_MIP_DRAWN  8

void drawMipLevels (const unsigned int texid, const unsigned int *shaderProfile)
{
  int        w, h, ww, hh;
  unsigned int  i;
  unsigned int  l;

  glUseProgram (shaderProfile[2]);
  l = glGetUniformLocation (shaderProfile[2], "mipLevel\0");
  glUniform1i (glGetUniformLocation (shaderProfile[2], "mipTex\0"), 0);

  glActiveTexture (GL_TEXTURE0);
  glEnable (GL_TEXTURE_2D);
  glBindTexture (GL_TEXTURE_2D, texid);
  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

  w>>=2;
  h>>=2;

  for (i=0, ww=0, hh=0; i<MAX_MIP_DRAWN; i++, ww+=w)
  {
    glUniform1f (l, (float)i);

    if ((i>0) && (i%3==0))  {ww=0; hh+=h;}

    glBegin (GL_QUADS);
    glTexCoord2i (0, 1);  glVertex2i (ww, hh);
    glTexCoord2i (0, 0);  glVertex2i (ww, hh+h);
    glTexCoord2i (1, 0);  glVertex2i (ww+w, hh+h);
    glTexCoord2i (1, 1);  glVertex2i (ww+w, hh);
    glEnd ();
  }

  glDisable (GL_TEXTURE_2D);

  glUseProgram (0);
}

LRESULT CALLBACK MainProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  static unsigned int  texid;
  static unsigned int  noMipmap, glMipmap, muMipmap;
  static unsigned int  mipLevel;
  static GLUtils::ShaderProfile  mipShader;
  static GLUtils::ShaderProfile  muShader;

  switch (message)
  {
    case WM_CREATE:      // creation
    {
      GLUtils::setupGLContext (hwnd);

      glewInit ();

      // setup the fonts
      GLUtils::Text::createFonts (GetDC (hwnd), 12, "verdana\0");

      PostMessage (hwnd, WM_COMMAND, FILE_OPEN, 0);

      return 1;
    }

    case WM_SIZE:      // Resizing
    {
      RECT  r;
      GetClientRect (hwnd, &r);

      glViewport (0, 0, r.right-r.left, r.bottom-r.top);
      return 1;
    }

    case WM_PAINT:      // Drawing (WM_TIMER and WM_PAINT are combined)
    case WM_TIMER:
    {
      RECT  r;
      GetClientRect (hwnd, &r);

      glClearColor (1,1,1,0);
      glClear (GL_COLOR_BUFFER_BIT);

      glMatrixMode (GL_PROJECTION);  glLoadIdentity ();  glOrtho (0, r.right, r.bottom, 0, 0, 1);
      glMatrixMode (GL_MODELVIEW);  glLoadIdentity ();

      if (glIsTexture (texid) == GL_TRUE)
      {
      //  drawTextures (texid, glMipmap);
        drawMipLevels (texid, mipShader);
      }
      SwapBuffers (GetDC (hwnd));
      break;
    }
    case WM_COMMAND:    // Input commands
    {
      switch (LOWORD(wParam))
      {
        case FILE_OPEN:
        {
#if 0
          char  buf[MAX_PATH];

          if (!WinUtils::getOpenFileName (hwnd, buf, MAX_PATH, "*.png\\0"))
            break;
#else
          char  *buf = TEST_IMAGE;
#endif
          SendMessage (hwnd, WM_COMMAND, FILE_CLOSE, 0);

          glGenTextures (1, &noMipmap);

          if (!GLUtils::readImageToTexture (buf, noMipmap))
          {
            fprintf (stdout, "ERR: Unable to read '%s'\\n\\0", buf);
            SendMessage (hwnd, WM_COMMAND, FILE_CLOSE, 0);
            break;
          }

          texid = noMipmap;

          PostMessage (hwnd, WM_COMMAND, MIPMAP_GEN, 0);

          break;
        }
        case FILE_CLOSE:
        {
          SendMessage (hwnd, WM_COMMAND, SHADER_DELT, 0);

          glDeleteTextures (1, &noMipmap);  noMipmap = 0;
          glDeleteTextures (1, &glMipmap);  glMipmap = 0;
          glDeleteTextures (1, &muMipmap);  muMipmap = 0;
          texid = 0;
          break;
        }
        case FILE_EXIT:
        {
          SendMessage (hwnd, WM_COMMAND, FILE_CLOSE, 0);
          break;
        }
        case MIPMAP_GEN:
        {
          if (glIsTexture (noMipmap) == false)
            break;

          glDeleteTextures (1, &glMipmap);  glMipmap = 0;
          glDeleteTextures (1, &muMipmap);  muMipmap = 0;

          // load the shaders
          SendMessage (hwnd, WM_COMMAND, SHADER_DELT, 0);
          SendMessage (hwnd, WM_COMMAND, SHADER_LOAD, 0);

          glMipmap = buildGLMipmaps (noMipmap);
          muMipmap = buildMuMipmaps (noMipmap, muShader);
          break;
        }
        case SHADER_LOAD:
        {
          if (GLUtils::loadShader (MIP_VERT, NULL, MIP_FRAG, mipShader))
            fprintf (stdout, "Loaded mipmap shaders\n\0");
          else
            fprintf (stdout, "Failed to load mipmap shaders\n\0");

          if (GLUtils::loadShader (MIP_VERT, NULL, MUP_FRAG, muShader))
            fprintf (stdout, "Loaded mumip shaders\n\0");
          else
            fprintf (stdout, "Failed to load mumip shaders\n\0");

          break;
        }
        case SHADER_DELT:
        {
          GLUtils::cleanShader (mipShader);
          break;
        }
        default:
          break;
      }

      return 0;
    }

    case WM_KEYDOWN:    // Keyboard input
    {
      switch (wParam)
      {
        case VK_RIGHT:
        {
          mipLevel++;
          fprintf (stdout, "Miplevel: %i\n\0", mipLevel);
          break;
        }
        case VK_LEFT:
        {
          if (mipLevel>0)
            mipLevel--;

          fprintf (stdout, "Miplevel: %i\n\0", mipLevel);
          break;
        }
        case VK_UP:
        {
          if (texid == noMipmap)
            texid = glMipmap;
          else if (texid == glMipmap)
            texid = muMipmap;
          else if (texid == muMipmap)
            texid = noMipmap;

          PostMessage (hwnd, WM_COMMAND, WM_PAINT, 0);
          break;
        }
        case VK_DOWN:
        {
          if (texid == noMipmap)
            texid = muMipmap;
          else if (texid == glMipmap)
            texid = noMipmap;
          else if (texid == muMipmap)
            texid = glMipmap;

          PostMessage (hwnd, WM_COMMAND, WM_PAINT, 0);
          break;
        }
        case VK_RETURN:  // reload shaders
        {
          SendMessage (hwnd, WM_COMMAND, SHADER_DELT, 0);
          SendMessage (hwnd, WM_COMMAND, SHADER_LOAD, 0);
          PostMessage (hwnd, WM_PAINT, 0, 0);
          break;
        }
        case VK_ESCAPE:
        {
          SendMessage (hwnd, WM_COMMAND, MIPMAP_GEN, 0);
          PostMessage (hwnd, WM_PAINT, 0, 0);
          break;
        }
      default:
        break;
      }

      PostMessage (hwnd, WM_PAINT, 0, 0);
      break;
    }
    case WM_DESTROY:    // close and exit
    {
      GLUtils::Text::cleanFonts ();
      GLUtils::releaseGLContext (hwnd);
      return 0;
    }
    case WM_CLOSE:
    {
      SendMessage (hwnd, WM_COMMAND, FILE_CLOSE, 0);
      PostQuitMessage (0);
      return 0;
    }
  }
  return DefWindowProc (hwnd, message, wParam, lParam);
}

#define CLASS_NAME  "QuadTreeWinCls\\0"
#define APP_NAME  "QuadTree test\0"

int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow)
{
  WinUtils::Console  c;
    HWND  hwnd;

  WinUtils::createWindowClass (hInstance, MainProc, CLASS_NAME);
  hwnd = WinUtils::createWindow (APP_NAME, CLASS_NAME);

  WinUtils::showWindow (hwnd);
  WinUtils::messagePump (hwnd);
  WinUtils::destroyWindow (hwnd);

  return 1;
}
