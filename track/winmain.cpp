#pragma comment(lib, "../../Image/PNG/libpng13.lib")  // libpng

#include <iostream>
#include <list>

#include "../../Utils/Patterns/Persistable.h"
#include "../../Utils/Structures/ParameterSet.h"
#define NOMINMAX
#include <windows.h>                  // windows
#include "../../Utils/Win/Utils.h"
#include "../../Utils/Win/Console.h"
#include "../../Utils/Win/Menu.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <assert.h>

// opengl
#define _GLUTILS_DEF_LIBS_
#define _GLUTILS_USE_GLEW_
#include "../../Utils/GL/GLUtils.h"

#include "../../Image/pngreader.h"            // libpng

#define  FILE_OPEN_IMAGE    1
#define FILE_OPEN_PATH    2
#define FILE_SAVE_PATH    3
#define  FILE_CLOSE      4
#define FILE_CLOSE_IMAGE  5
#define FILE_CLOSE_PATH    6
#define FILE_EXIT      7
#define SETTING_SIZE_UP    10
#define SETTING_SIZE_DOWN  11
#define SETTING_FILL    12
#define SETTING_STROKE    13
#define SETTING_SHOW_STATS  14
#define SELECT_NEXT_CHAIN  20
#define SELECT_PREV_CHAIN  21
#define SELECT_LESS      22
#define SELECT_MORE      23
#define SELECT_DELETE_CHAIN  24
#define SELECT_MERGE_CHAIN  25
#define SET_SCROLL_PARAMS  100
#define SET_USER_CONTEXT  101
#define SAVE_USER_CONTEXT  102

typedef std::pair<unsigned int, unsigned int>  Vert;
typedef  std::list<Vert>            VertList;
typedef std::pair<VertList::iterator, VertList::iterator>  VertSelection;

#define TEST_IMAGE    "C:\\Users\\csed\\Documents\\Projects\\data\\Images\\lenna.png"
#define STIPPLE_TIMER  2

class Context
{
protected:
  unsigned int  srcImage;
  VertList    verts;
  Vert      currentPoint;
  VertSelection  sel;
  unsigned short  stipPat;

public:
  IntegerSet    ucx;

public:
  Context ()
    : srcImage (0), currentPoint (nullPoint ()), stipPat (initialStipplePattern ())
  {}
  // image
  bool readImage (const char *buf)
  {
    clearImage ();

    glGenTextures (1, &srcImage);

    if (!GLUtils::readImageToTexture (buf, srcImage))
    {
      fprintf (stdout, "ERR: Unable to read '%s'\\n\\0", buf);
      return false;
    }

    return true;
  }
  const unsigned int image () const
  {
    return srcImage;
  }
  const bool hasImage () const
  {
    return (image ()!=0);
  }
  void clearImage ()
  {
    if (glIsTexture (srcImage) == GL_TRUE)
    {
      glDeleteTextures (1, &srcImage);
      srcImage = 0;
    }
  }
  // path
  static Vert nullPoint ()
  {
    return Vert (0xFFFFFFFF, 0xFFFFFFFF);
  }
  void clearPath ()
  {
    verts.clear ();
    verts.push_back (nullPoint ());
    sel.first=verts.begin ();
    sel.second=verts.begin ();
  }
  void setHotpoint (const unsigned int x, const unsigned int y)
  {
    currentPoint.first = x;
    currentPoint.second = y;
  }
  Vert hotPoint () const
  {
    return currentPoint;
  }
  void clearHotpoint ()
  {
    currentPoint = nullPoint ();
  }
  // chain
  void selectNextChain ()
  {
    if ((sel.second==verts.end ()) || (std::distance (sel.second, verts.end ()) == 1))
    {
      sel.first = verts.begin ();
    }
    else
    {
      sel.first = sel.second++;
    }

    std::advance (sel.first, 1);
    sel.second = verts.end ();
    sel.second = std::find (sel.first, sel.second, nullPoint ());

    setStipplePattern (initialStipplePattern ());
  }
  void selectNone ()
  {
    sel.first = verts.begin ();
    sel.second = verts.begin ();
  }
  void chainAdd (Vert v)
  {
    verts.push_back (v);
  }
  void chainAdd (const unsigned int x, const unsigned int y)
  {
    chainAdd (Vert (x, y));
  }
  void chainEnd ()
  {
    verts.push_back (nullPoint ());
  }
  // settings
  void load ()
  {
    ucx.set ("glyphSize\0", 4);
    ucx.set ("fill\0", RGB(0,0,0));
    ucx.set ("stroke\0", RGB(255,255,255));
  }
  void save ()
  {
  }
  void setStipplePattern (const unsigned short nstp)
  {
    stipPat = nstp;
  }
  const unsigned short stipplePattern () const
  {
    return stipPat;
  }
  static const unsigned short initialStipplePattern ()
  {
    return 0x00FF;
  }
  void nextStipplePattern ()
  {
    setStipplePattern ((stipplePattern ()<<15)|(stipplePattern ()>>1));

    glLineStipple (1, stipplePattern ());
  }
  // drawing
  static void drawList (VertList::const_iterator beg, VertList::const_iterator end, GLenum mode)
  {
    VertList::const_iterator  pl;
    const Vert  np = Context::nullPoint ();

    glBegin (mode);
    for (;beg!=end;beg++)
    {
      if ((beg->first == np.first) && (beg->second == np.second))
      {
        glEnd ();
        glBegin (mode);
        continue;
      }
      glVertex2i (beg->first, beg->second);
    }
    glEnd ();
  }
  static void drawListR (VertList::const_iterator beg, VertList::const_iterator end, GLenum mode)
  {
    VertList::const_iterator  pl;
    const Vert  np = Context::nullPoint ();

    glBegin (mode);
    for (;beg!=end;beg--)
    {
      if ((beg->first == np.first) && (beg->second == np.second))
      {
        glEnd ();
        glBegin (mode);
        continue;
      }
      glVertex2i (beg->first, beg->second);
    }
    glEnd ();
  }
  void drawVerts ()
  {
    COLORREF  fill, strk;

    fill = ucx["fill\0"];
    strk = ucx["stroke\0"];

    glLineWidth (2.0f);

    // draw the point list
    if (verts.size () > 0)
    {
      glPointSize (static_cast<float>(ucx["size\0"]+2));
      glColor4ub (GetRValue (strk), GetGValue (strk), GetBValue (strk), 25);
      drawList (verts.begin (), verts.end (), GL_LINE_STRIP);
      drawList (verts.begin (), verts.end (), GL_POINTS);
      glPointSize (static_cast<float>(ucx["size\0"]));
      glColor4ub (GetRValue (fill), GetGValue (fill), GetBValue (fill), 25);
      drawList (verts.begin (), verts.end (), GL_LINE_STRIP);
      drawList (verts.begin (), verts.end (), GL_POINTS);
    }

    glLineWidth (1.0f);
    glPointSize (1.0f);
  }
  void drawHotpoint ()
  {
    COLORREF  fill, strk;
    const Vert&  np = nullPoint ();
    const Vert&  hp = hotPoint ();

    fill = ucx["fill\0"];
    strk = ucx["stroke\0"];

    if ((hp.first != np.first) && (hp.second != np.second))
    {
      glPointSize (static_cast<float>(ucx["size\0"]+6));
      glColor4ub (GetRValue (strk), GetGValue (strk), GetBValue (strk), 25);
      glBegin (GL_POINTS);
      glVertex2i (hp.first, hp.second);
      glEnd ();

      glPointSize (static_cast<float>(ucx["size\0"]+4));
      glColor4ub (GetRValue (fill), GetGValue (fill), GetBValue (fill), 25);
      glBegin (GL_POINTS);
      glVertex2i (hp.first, hp.second);
      glEnd ();
    }
  }
  void drawSelection ()
  {
    if (std::distance (sel.first, sel.second) > 0)
    {
      VertList::const_iterator  l = sel.second;
      l--;
      glLineWidth (static_cast<float>(ucx["size\0"]+2));
      glColor4f (1.0f, 0.0f, 0.0f, 0.4f);
#if 0
      drawList (sel.first, sel.second, GL_LINE_STRIP);
      drawList (sel.first, sel.second, GL_POINTS);
#else
      glLineStipple (1, ~stipplePattern ());
      glColor4f (0.0f, 0.0f, 0.0f, 0.4f);
      glEnable (GL_LINE_STIPPLE);
      drawListR (sel.second, sel.first, GL_LINE_STRIP);
      glDisable (GL_LINE_STIPPLE);

      glLineStipple (1, stipplePattern ());
      glColor4f (1.0f, 0.0f, 0.0f, 0.4f);
      glEnable (GL_LINE_STIPPLE);
      drawListR (sel.second, sel.first, GL_LINE_STRIP);
      glDisable (GL_LINE_STIPPLE);
#endif
      glEnable (GL_LINE_STIPPLE);
      glBegin (GL_LINES);
      glVertex2i (l->first, l->second);
      glVertex2i (sel.first->first, sel.first->second);
      glEnd ();
      glDisable (GL_LINE_STIPPLE);

    //  if (ucx["showStats\0"] == TRUE)
      {
        Vert  mp;
        float  x[2], y[2];
        float  dx, dy, gr;  // gradient

        GLUtils::Text::setHorizontalAlignment (GLUtils::Text::LEFT);
        GLUtils::Text::setVerticalAlignment (GLUtils::Text::TOP);

        glPushMatrix ();
        glTranslatef (sel.first->first, sel.first->second, 0.0f);
        glScalef (30.0f, -30.0f, 1.0f);
        GLUtils::Text::glPrintf ("Start\0");
        glPopMatrix ();

        glPushMatrix ();
        glTranslatef (l->first, l->second, 0.0f);
        glScalef (30.0f, -30.0f, 1.0f);
        GLUtils::Text::glPrintf ("End\0");
        glPopMatrix ();

        GLUtils::Text::setHorizontalAlignment (GLUtils::Text::CENTER);
        GLUtils::Text::setVerticalAlignment (GLUtils::Text::BOTTOM);

        x[0] = sel.first->first;
        x[1] = l->first;
        y[0] = sel.first->second;
        y[1] = l->second;
        mp.first = x[0]+((x[1]-x[0])/2);
        mp.second = y[0]+((y[1]-y[0])/2);
        dx = (x[1]-x[0]);
        dy = (y[1]-y[0]);
        gr = dy/dx;
        gr = atan (gr) * (180.0/M_PI);

        glPushMatrix ();
        glTranslatef (mp.first, mp.second, 0.0f);
        glRotatef (gr, 0.0f, 0.0f, 1.0f);
        glScalef (30.0f, -30.0f, 1.0f);
        GLUtils::Text::glPrintf ("Tort\0");
        glPopMatrix ();
      }
    }

    glLineWidth (1.0f);
  }
};

void scrollState (HWND hwnd, SCROLLINFO *si)
{
  ZeroMemory (&si[0], sizeof (SCROLLINFO));
  ZeroMemory (&si[1], sizeof (SCROLLINFO));

  si[0].cbSize = sizeof (SCROLLINFO);
  si[1].cbSize = sizeof (SCROLLINFO);
  si[0].fMask = SIF_ALL;
  si[1].fMask = SIF_ALL;
  GetScrollInfo (hwnd, SB_HORZ, &si[0]);
  GetScrollInfo (hwnd, SB_VERT, &si[1]);
}
LRESULT CALLBACK MainProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  static Context      ctx;
  static WinUtils::Menu  menu;

  switch (message)
  {
    case WM_CREATE:      // creation
    {
      GLUtils::setupGLContext (hwnd);

      glewInit ();

      // setup the fonts
      GLUtils::Text::createFonts (GetDC (hwnd), 12, "verdana\0");

      // create the menu
      {
        WinUtils::Menu  size;

        menu.add ("File\0", "Open image...\tCtrl+O\0", FILE_OPEN_IMAGE);
        menu.add ("File\0", "Open path...\0", FILE_OPEN_PATH);
        menu.sep ("File\0");
        menu.add ("File\0", "Save path...\0", FILE_SAVE_PATH);
        menu.sep ("File\0");
        menu.add ("File\0", "Close\0", FILE_CLOSE);
        menu.add ("File\0", "Exit\0", FILE_EXIT);

        size.add ("Size\0", "Up\t+\0", SETTING_SIZE_UP);
        size.add ("Size\0", "Down\t-\0", SETTING_SIZE_DOWN);
        menu.add ("Settings\0", size);
        menu.add ("Settings\0", "Fill...\0", SETTING_FILL);
        menu.add ("Settings\0", "Stroke...\0", SETTING_STROKE);
        menu.sep ("Settings\0");
        menu.add ("Settings\0", "Show stats\0", SETTING_SHOW_STATS);

        menu.add ("Select\0", "Next\0", SELECT_NEXT_CHAIN);
        menu.add ("Select\0", "Previous\0", SELECT_PREV_CHAIN);
        menu.sep ("Select\0");
        menu.add ("Select\0", "Merge\0", SELECT_MERGE_CHAIN);
        menu.add ("Select\0", "Delete\0", SELECT_DELETE_CHAIN);

        menu (hwnd);
      }

      PostMessage (hwnd, WM_COMMAND, FILE_OPEN_IMAGE, 0);
      return 1;
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
    case WM_SIZE:      // Resizing
    {
      RECT  r;
      GetClientRect (hwnd, &r);

      glViewport (0, 0, r.right-r.left, r.bottom-r.top);

      SendMessage (hwnd, WM_COMMAND, SET_SCROLL_PARAMS, 0);
      return 1;
    }
    case WM_VSCROLL:
    case WM_HSCROLL:
    {
      SCROLLINFO    si[2] = {0};

      scrollState (hwnd, si);
      si[0].nPos = si[0].nTrackPos;
      si[1].nPos = si[1].nTrackPos;
      SetScrollInfo (hwnd, SB_HORZ, &si[0], TRUE);
      SetScrollInfo (hwnd, SB_VERT, &si[1], TRUE);
      PostMessage (hwnd, WM_PAINT, 0, 0);
      break;
    }
    case WM_TIMER:
    {
      if (wParam == STIPPLE_TIMER)
      {
        ctx.nextStipplePattern ();
      }
    }  /// intentional fall through
    case WM_PAINT:      // Drawing (WM_TIMER and WM_PAINT are combined)
    {
      SCROLLINFO    si[2]={0};
      RECT      r;
      unsigned int  w, h;
      unsigned int  bbox[4];

      scrollState (hwnd, si);

      glClearColor (1,1,1,0);
      glClear (GL_COLOR_BUFFER_BIT);

      if (glIsTexture (ctx.image ()) == FALSE)
      {
        SwapBuffers (GetDC (hwnd));
        break;
      }

      GetClientRect (hwnd, &r);

      glBindTexture (GL_TEXTURE_2D, ctx.image ());
      w = GLUtils::textureWidth ();
      h = GLUtils::textureHeight ();

      if (r.right-r.left>w)
      {
        bbox[0]=0;bbox[2]=r.right-r.left;
      }
      else
      {
        bbox[0]=si[0].nPos;bbox[2]=si[0].nPos+si[0].nPage;
      }

      if (r.bottom-r.top>h)
      {
        bbox[1]=0;bbox[3]=r.bottom-r.top;
      }
      else
      {
        bbox[1]=si[1].nPos;bbox[3]=si[1].nPos+si[1].nPage;
      }
      glMatrixMode (GL_PROJECTION);
      glLoadIdentity ();
      glOrtho (bbox[0], bbox[2], bbox[3], bbox[1], 0, 1);
      glMatrixMode (GL_MODELVIEW);  glLoadIdentity ();

      // set the state
      glEnable (GL_POINT_SMOOTH);
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // draw the image
      {
      glEnable (GL_TEXTURE_2D);
      glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
      glBegin (GL_QUADS);
      glTexCoord2i (0, 0);  glVertex2i (0, 0);
      glTexCoord2i (1, 0);  glVertex2i (w, 0);
      glTexCoord2i (1, 1);  glVertex2i (w, h);
      glTexCoord2i (0, 1);  glVertex2i (0, h);
      glEnd ();
      glDisable (GL_TEXTURE_2D);
      }
      // get the draw settings
      ctx.drawVerts ();
      // draw the current point
      ctx.drawHotpoint ();
      // draw the selection
      ctx.drawSelection ();

      // restore the state
      glDisable (GL_BLEND);
      glDisable (GL_POINT_SMOOTH);

      SwapBuffers (GetDC (hwnd));
      break;
    }
    case WM_COMMAND:    // Input commands
    {
      switch (LOWORD(wParam))
      {
        // USER COMMANDS
        case FILE_OPEN_IMAGE:
        {
#if 0
          char  buf[MAX_PATH];

          if (!WinUtils::getOpenFileName (hwnd, buf, MAX_PATH, "*.png\\0"))
            break;
#else
          char  *buf = TEST_IMAGE;
#endif
          SendMessage (hwnd, WM_COMMAND, FILE_CLOSE, 0);

          if (ctx.readImage (buf) == true)
          {
            PostMessage (hwnd, WM_COMMAND, SET_SCROLL_PARAMS, 0);
          }
          else
          {
            SendMessage (hwnd, WM_COMMAND, FILE_CLOSE, 0);
          }
          break;
        }
        case FILE_OPEN_PATH:
        {
          SendMessage (hwnd, WM_COMMAND, FILE_CLOSE_PATH, 0);
          break;
        }
        case FILE_CLOSE_IMAGE:
        {
          // FIXME: test for save file
          ctx.clearImage ();
          break;
        }
        case FILE_CLOSE_PATH:
        {
          ctx.clearPath ();
          break;
        }
        case FILE_CLOSE:
        {
          SendMessage (hwnd, WM_COMMAND, FILE_CLOSE_PATH, 0);
          SendMessage (hwnd, WM_COMMAND, FILE_CLOSE_IMAGE, 0);
          break;
        }
        case FILE_EXIT:
        {
          SendMessage (hwnd, WM_COMMAND, FILE_CLOSE, 0);
          break;
        }
        case SETTING_SIZE_UP:
        {
          int  mxsz[2]={0};

          glGetIntegerv (GL_ALIASED_LINE_WIDTH_RANGE, mxsz);

          if (ctx.ucx["size\0"] <  mxsz[1]-2)
            ctx.ucx.set ("size\0", ctx.ucx["size\0"]+1);
          break;
        }
        case SETTING_SIZE_DOWN:
        {
          if (ctx.ucx["size\0"] > 1)
            ctx.ucx.set ("size\0", ctx.ucx["size\0"]-1);
          break;
        }
        case SETTING_FILL:
        {
          COLORREF  c, n;

          c = ctx.ucx["fill\0"];
          WinUtils::getColour (hwnd, c, n);
          ctx.ucx.set ("fill\0", n);
          break;
        }
        case SETTING_STROKE:
        {
          COLORREF  c, n;

          c = ctx.ucx["stroke\0"];
          WinUtils::getColour (hwnd, c, n);
          ctx.ucx.set ("stroke\0", n);
          break;
        }
        case SETTING_SHOW_STATS:
        {
          if (ctx.ucx["showStats\0"] == TRUE)
          {
            ctx.ucx["showStats\0"] = FALSE;
            menu.uncheck ("Settings\0", SETTING_SHOW_STATS);
          }
          else
          {
            ctx.ucx["showStats\0"] = TRUE;
            menu.check ("Settings\0", SETTING_SHOW_STATS);
          }
          break;
        }
        case SELECT_NEXT_CHAIN:
        {
          ctx.selectNextChain ();
          SetTimer (hwnd, STIPPLE_TIMER, 1000/20, NULL);
          break;
        }
      /*  case SELECT_PREV_CHAIN:
        {
          std::reverse_iterator<VertList::const_iterator>  rb (sel.first);
          std::reverse_iterator<VertList::const_iterator>  rn (sel.second);

          if (rb.base ()==verts.begin ())
          {
            rn.base () = verts.end ()-1;
          }
          else
          {
            rn = rb--;
          }

          rb.base () = verts.begin ();
          rb=std::find (rn, rb, NullPoint);
          break;
        }*/
        // INTERNAL COMMANDS
        case SET_SCROLL_PARAMS:
        {
          unsigned int  w, h;
          RECT      r;
          SCROLLINFO    si = {0};

          GetClientRect (hwnd, &r);

          if (ctx.hasImage () == false)
          {
            w=h=0;
          }
          else
          {
            glBindTexture (GL_TEXTURE_2D, ctx.image ());
            w = GLUtils::textureWidth ();
            h = GLUtils::textureHeight ();
          }

          si.cbSize = sizeof (SCROLLINFO);
          si.fMask = SIF_PAGE|SIF_RANGE;
          si.nMin = 0;
          si.nMax = w;
          si.nPage = r.right-r.left;
          SetScrollInfo (hwnd, SB_HORZ, &si, TRUE);

          si.fMask = SIF_PAGE|SIF_RANGE;
          si.nMin = 0;
          si.nMax = h;
          si.nPage = r.bottom-r.top;
          SetScrollInfo (hwnd, SB_VERT, &si, TRUE);

          break;
        }
        case SET_USER_CONTEXT:
        {
          ctx.load ();
          break;
        }
        case SAVE_USER_CONTEXT:
        {
          ctx.save ();
          break;
        }
        default:
        {
          break;
        }
      }
      PostMessage (hwnd, WM_PAINT, 0, 0);
      break;
    }
    case WM_KEYDOWN:    // Keyboard input
    {
      switch (wParam)
      {
        case VK_RIGHT:
        {
          SendMessage (hwnd, WM_COMMAND, SELECT_NEXT_CHAIN, 0);
          break;
        }
        case VK_LEFT:
        {
          SendMessage (hwnd, WM_COMMAND, SELECT_PREV_CHAIN, 0);
          break;
        }
        case VK_UP:
        {
          break;
        }
        case VK_DOWN:
        {
          break;
        }
        case VK_ADD:
        {
          SendMessage (hwnd, WM_COMMAND, SETTING_SIZE_UP, 0);
          break;
        }
        case VK_SUBTRACT:
        {
          SendMessage (hwnd, WM_COMMAND, SETTING_SIZE_DOWN, 0);
          break;
        }
        case VK_RETURN:  // reload shaders
        {
          break;
        }
        case VK_ESCAPE:
        {
          KillTimer (hwnd, STIPPLE_TIMER);
          ctx.clearHotpoint ();
          ctx.selectNone ();
          break;
        }
        default:
        {
          break;
        }
      }

      PostMessage (hwnd, WM_PAINT, 0, 0);
      break;
    }

    case WM_LBUTTONDOWN:  // mouse input
    {
      SCROLLINFO  si[2]={0};
      POINTS    pt;

      pt = MAKEPOINTS (lParam);

      scrollState (hwnd, si);
      ctx.setHotpoint (pt.x + si[0].nPos, pt.y + si[1].nPos);
      SetCapture (hwnd);
      PostMessage (hwnd, WM_PAINT, 0, 0);
      break;
    }
    case WM_LBUTTONUP:
    {
      ctx.chainEnd ();
      ReleaseCapture ();
      PostMessage (hwnd, WM_PAINT, 0, 0);
      break;
    }
    case WM_MOUSEMOVE:
    {
      if (wParam & MK_LBUTTON)
      {
        SCROLLINFO  si[2]={0};
        POINTS    pt;

        pt = MAKEPOINTS(lParam);
        scrollState (hwnd, si);
        ctx.chainAdd (ctx.hotPoint ());
        ctx.setHotpoint (pt.x + si[0].nPos, pt.y + si[1].nPos);
        PostMessage (hwnd, WM_PAINT, 0, 0);
      }

      break;
    }
  }
  return DefWindowProc (hwnd, message, wParam, lParam);
}

#define CLASS_NAME  "TrackWinCls\\0"
#define APP_NAME  "Track\0"

int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow)
{
  WinUtils::Console  c;
    HWND  hwnd;

  WinUtils::createWindowClass (hInstance, MainProc, CLASS_NAME);
  hwnd = WinUtils::createWindow (APP_NAME, CLASS_NAME, WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL);

  WinUtils::showWindow (hwnd);
  WinUtils::messagePump (hwnd);
  WinUtils::destroyWindow (hwnd);

  return 1;
}
