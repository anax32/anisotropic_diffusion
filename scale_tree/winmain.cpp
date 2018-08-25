#pragma comment(lib, "../../Image/PNG/libpng13.lib")  // libpng

#include <iostream>
#define NOMINMAX
#define OEMRESOURCE
#include <windows.h>                  // windows

#include "../../Utils/Patterns/Persistable.h"
#include "../../Utils/Structures/ParameterSet.h"
#include "../../Utils/Callbacks/Callback.h"


#include "../../Utils/Win/Utils.h"
#include "../../Utils/Win/Console.h"
#include "../../Utils/Win/Messages.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <assert.h>

// opengl
#define _GLUTILS_DEF_LIBS_
#define _GLUTILS_USE_GLEW_
#include "../../Utils/GL/GLUtils.h"
#include "../../Utils/GL/Camera.h"
#include "../../Utils/ImageUtils.h"

#include "../../Image/pngreader.h"            // libpng
#include "../Image.h"
#include "../Diffusion.h"

#define  FILE_OPEN  1
#define  FILE_CLOSE  2
#define FILE_EXIT  3
#define BUILD_SCALE_TEXTURES  10
#define CLEAN_SCALE_TEXTURES  11
#define SAVE_SCALE_IMAGE    20

#define SCALE_TEX_CNT    300

//#define TEST_IMAGE  "C:\\Users\\csed\\Documents\\Projects\\Image\\Quadtree\\geo_sm1.png\0"
//#define TEST_IMAGE  "C:\\Users\\csed\\Documents\\Projects\\Image\\Quadtree\\geo_sm2.png\0"
//#define TEST_IMAGE  "C:\\Users\\csed\\Documents\\Projects\\Image\\Quadtree\\geo_sm3.png\0"
#define TEST_IMAGE  "C:\\Users\\csed\\Documents\\Projects\\Image\\DiffusionFilter\\geo_sm.png\0"

void setGradients (const unsigned int *scaleTextures, const unsigned int scaleCount)
{
  unsigned int  s, i, t, j;
  int        w, h, x, y, xx, yy;
  float      *img, *emg;
  float      box[9], he, ve, me;
  float      frng[4], nrng[4];

  glBindTexture (GL_TEXTURE_2D, scaleTextures[0]);
  glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

  img = new float[w*h*2];
  emg = new float[w*h*2];

  nrng[0] = 0.0f; nrng[1] = 0.0f; nrng[2]=1.0f; nrng[3]=1.0f;

  t = clock ();
  fprintf (stdout, "Getting gradients...\0");

  for (i=0; i<scaleCount; i++)
  {
    fprintf (stdout, "\rGradient %03i/%03i\0", i, scaleCount);

    glBindTexture (GL_TEXTURE_2D, scaleTextures[i]);
    glGetTexImage (GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, GL_FLOAT, img);

    memset (emg, 0, w*h*2*sizeof (float));

    // compute the gradient
    for (y=1; y<h-1; y++)
    {
      for (x=1; x<w-1; x++)
      {
        // get the data
        for (j=0,yy=y-1;yy<y+2;yy++)
        {
          for (xx=x-1;xx<x+2;xx++,j++)
          {
            box[j] = img[(yy*w*2)+(xx*2)+0];
          }
        }

        // compute the sobel edge
        he = box[2]+(2.0*box[5])+box[8] - (box[0]+(2.0*box[3])+box[6]);
        ve = box[0]+(2.0*box[1])+box[2] - (box[6]+(2.0*box[7])+box[8]);
        me = sqrt (he*he + ve*ve);

        emg[(y*w*2)+(x*2)+0] = img[(y*w*2)+(x*2)+0];
        emg[(y*w*2)+(x*2)+1] = me;
      }
    }

    ImageUtils::getMinmax<float> (emg, w, h, 2, &frng[0], &frng[2]);
    ImageUtils::normalise<float> (emg, w, h, 2, &frng[0], &frng[2], &nrng[0], &nrng[2]);

    glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, w, h, GL_LUMINANCE_ALPHA, GL_FLOAT, emg);
  }

  fprintf (stdout, "\rComputed gradients in %0.2fs\n\0", (float)(clock ()-t)/CLOCKS_PER_SEC);

  delete[] img;
}
void buildScaleTextures (const unsigned int texid, unsigned int *scaleTextures, const unsigned int scaleCount)
{
  int        w, h, i, j;
  float      *tex, *ttx, frng[2], fnrng[2];
  double      *img, drng[2], dnrng[2];
  Diffusion    *diff;
  unsigned int  t;

  fprintf (stdout, "Construcing scales...\0");
  t = clock ();

  // get the primal image
  glBindTexture (GL_TEXTURE_2D, texid);
  glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

  tex = new float[w*h];
  img = new double[w*h];
  ttx = new float[w*h*2];

  glGetTexImage (GL_TEXTURE_2D, 0, GL_LUMINANCE, GL_FLOAT, tex);

  fnrng[0]=0.0f; fnrng[1]=1.0f;
  dnrng[0]=0.0; dnrng[1]=1.0;
  ImageUtils::getMinmax<float> (tex, w, h, 1, &frng[0], &frng[1]);
//  ImageUtils::normalise<float> (tex, w, h, 1, &frng[0], &frng[1], &fnrng[0], &fnrng[1]);
  ImageUtils::convert<float, double> (tex, img, w, h, 1);

  for (i=0; i<w*h; i++)
  {
    ttx[(i*2)+0] = tex[i];
    ttx[(i*2)+1] = tex[i]*tex[i]*tex[i];
  }

  // create the scales
  glGenTextures (scaleCount, scaleTextures);

  // use mipmaps
//  glHint (GL_GENERATE_MIPMAP_HINT, GL_NICEST);

  // upload the primal image to t=0
  glBindTexture (GL_TEXTURE_2D, scaleTextures[0]);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
//  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
//  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_LUMINANCE_ALPHA, GL_FLOAT, ttx);
//  glGenerateMipmapEXT (GL_TEXTURE_2D);

  // construct the lower scales
  diff = new AnisotropicDiffusion (w, h, 0.12, 0.02);
//  diff = new GaussianDiffusion (w, h, 5);

  for (i=0; i<scaleCount-1; i++)
  {
    fprintf (stdout, "\rScale %03i/%03i...\0", i+1, scaleCount);

    // compute the next level of diffusion
    diff->diffusionStep (img);

    ImageUtils::getMinmax<double> (img, w, h, 1, &drng[0], &drng[1]);
  //  ImageUtils::normalise<double> (img, w, h, 1, &drng[0], &drng[1], &dnrng[0], &dnrng[1]);

    fprintf (stdout, "mn=[%0.3f], mx=[%0.3f]\0", drng[0], drng[1]);

    // copy to float
    for (j=0; j<w*h; j++)
    {
      ttx[(j*2)+0] = img[j];
      ttx[(j*2)+1] = img[j]*img[j];
    }

    // upload the image
    glBindTexture (GL_TEXTURE_2D, scaleTextures[i+1]);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
//    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
//    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_LUMINANCE_ALPHA, GL_FLOAT, ttx);
//    glGenerateMipmapEXT (GL_TEXTURE_2D);

  //  GLUtils::writeTextureToFile ("test.png\0", GL_LUMINANCE);
  }

  fprintf (stdout, "\rConstructed scales in %0.2fs\n\0", (float)(clock ()-t)/CLOCKS_PER_SEC);

  // cleanup
  delete[] img;
  delete[] tex;
  delete[] ttx;
  delete diff;
}
void drawTexture (const unsigned int texid, const unsigned int w, const unsigned int h,
          const unsigned int *scaleTextures, const unsigned int scaleIndex)
{
  unsigned int  tw, th;

  tw = (float)w/(float)4;
  th = (float)w/(float)4;

  glEnable (GL_TEXTURE_2D);
  glBindTexture (GL_TEXTURE_2D, texid);
  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  glBegin (GL_QUADS);
  glTexCoord2f (0, 0);  glVertex2f (0, 0);
  glTexCoord2f (0, 1);  glVertex2f (0, th);
  glTexCoord2f (1, 1);  glVertex2f (tw, th);
  glTexCoord2f (1, 0);  glVertex2f (tw, 0);
  glEnd ();

  glBindTexture (GL_TEXTURE_2D, scaleTextures[scaleIndex]);

  glBegin (GL_QUADS);
  glTexCoord2f (0, 0);  glVertex2f (0, th);
  glTexCoord2f (0, 1);  glVertex2f (0, th*2);
  glTexCoord2f (1, 1);  glVertex2f (tw, th*2);
  glTexCoord2f (1, 0);  glVertex2f (tw, th);
  glEnd ();

  glDisable (GL_TEXTURE_2D);
}
void drawScaleCube (const unsigned int *texid, const unsigned int scaleCount, const unsigned int scaleIndex,
          const float aspectRatio, GLUtils::Camera *camera)
{
  unsigned int  i;
  float      t;

  // set the new projections
  glMatrixMode (GL_PROJECTION);  glPushMatrix ();  glLoadIdentity ();  gluPerspective (40.0, aspectRatio, 1, 1000);
  glMatrixMode (GL_MODELVIEW);  glPushMatrix ();  glLoadIdentity ();  camera->rotate ();

  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable (GL_DEPTH_TEST);

  glEnable (GL_TEXTURE_2D);

  // draw the scales
  for (i=0; i<scaleCount; i++)
  {
    t = -1.0 + (2.0*((float)i/(float)(scaleCount-1)));

    if (i==scaleIndex)
      glDisable (GL_BLEND);

    glBindTexture (GL_TEXTURE_2D, texid[i]);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glBegin (GL_QUADS);
    glColor4f (1,0,0.5, 1.0f/(float)(scaleCount));
    glTexCoord2f (0, 0);  glVertex3f (-1,t,-1);
    glTexCoord2f (0, 1);  glVertex3f (-1,t, 1);
    glTexCoord2f (1, 1);  glVertex3f ( 1,t, 1);
    glTexCoord2f (1, 0);  glVertex3f ( 1,t,-1);
    glEnd ();

    if (i==scaleIndex)
      glEnable (GL_BLEND);
  }

  glDisable (GL_TEXTURE_2D);
  glDisable (GL_DEPTH_TEST);
  glDisable (GL_BLEND);

  // cleanup
  glMatrixMode (GL_PROJECTION);  glPopMatrix ();
  glMatrixMode (GL_MODELVIEW);  glPopMatrix ();
}
LRESULT CALLBACK MainProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  static GLUtils::Camera    camera;
  static GLUtils::MouseCameraControl  mcc;
  static unsigned int  texid;
  static unsigned int  noMipmap;
  static unsigned int  scaleTextures[SCALE_TEX_CNT];
  static unsigned int  selectedScale;

  switch (message)
  {
    case WM_CREATE:      // creation
    {
      GLUtils::setupGLContext (hwnd);

      glewInit ();

      // setup the fonts
      GLUtils::Text::createFonts (GetDC (hwnd), 12, "Verdana\0");

      // setup the camera
      float  defaultViewAngle[4] = {30, 150.0, 0.0, -5.0};
      camera.setDefaultViewAngle (defaultViewAngle);
      camera.reset ();

      mcc.addCamera (&camera);

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
      float  ar;
      RECT  r;
      GetClientRect (hwnd, &r);

      ar = (float)r.right/(float)r.bottom;

      glClearColor (1,1,1,0);
      glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

      glMatrixMode (GL_PROJECTION);  glLoadIdentity ();  glOrtho (0, r.right, r.bottom, 0, 0, 1);
      glMatrixMode (GL_MODELVIEW);  glLoadIdentity ();

      if (glIsTexture (texid) == GL_TRUE)
      {
        drawTexture (texid, r.right, r.bottom, scaleTextures, selectedScale);
        drawScaleCube (scaleTextures, SCALE_TEX_CNT, selectedScale, ar, &camera);
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

          break;
        }
        case FILE_CLOSE:
        {
          break;
        }
        case FILE_EXIT:
        {
          SendMessage (hwnd, WM_COMMAND, FILE_CLOSE, 0);
          break;
        }
        case BUILD_SCALE_TEXTURES:
        {
          SendMessage (hwnd, WM_COMMAND, CLEAN_SCALE_TEXTURES, 0);

          if (glIsTexture (texid) == GL_FALSE)
            break;

          buildScaleTextures (texid, scaleTextures, SCALE_TEX_CNT);
          setGradients (scaleTextures, SCALE_TEX_CNT);
          break;
        }
        case CLEAN_SCALE_TEXTURES:
        {
          glDeleteTextures (SCALE_TEX_CNT, scaleTextures);
          break;
        }
        case SAVE_SCALE_IMAGE:
        {
          char  buf[MAX_PATH];

          sprintf_s (buf, MAX_PATH, "scale_%03i.png\0", selectedScale);
          glBindTexture (GL_TEXTURE_2D, scaleTextures[selectedScale]);
          GLUtils::writeTextureToFile (buf, GL_LUMINANCE);
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
        case VK_RETURN:  // reload shaders
        {
          SendMessage (hwnd, WM_COMMAND, CLEAN_SCALE_TEXTURES, 0);
          SendMessage (hwnd, WM_COMMAND, BUILD_SCALE_TEXTURES, 0);
          PostMessage (hwnd, WM_PAINT, 0, 0);
          break;
        }
        case VK_ESCAPE:
        {
          camera.reset ();
          PostMessage (hwnd, WM_PAINT, 0, 0);
          break;
        }
        case VK_UP:
        {
          if (selectedScale<SCALE_TEX_CNT-1)
            selectedScale++;
          PostMessage (hwnd, WM_PAINT, 0, 0);
          break;
        }
        case VK_DOWN:
        {
          if (selectedScale>0)
            selectedScale--;
          PostMessage (hwnd, WM_PAINT, 0, 0);
          break;
        }
        case VK_F12:
        {
          PostMessage (hwnd, WM_COMMAND, SAVE_SCALE_IMAGE, 0);
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
    default:
    {
      if (mcc.dispatch (hwnd, message, wParam, lParam) == TRUE)
        PostMessage (hwnd, WM_PAINT, 0, 0);
      break;
    }
  }
  return DefWindowProc (hwnd, message, wParam, lParam);
}

#define CLASS_NAME  "ScaleTree\0"
#define APP_NAME  "Scale Tree\0"

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