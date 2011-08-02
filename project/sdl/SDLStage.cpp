#include <Display.h>
#include <Utils.h>
#include <SDL.h>
#include <Surface.h>
#include <ExternalInterface.h>
#include <KeyCodes.h>
#include <map>

#ifdef WEBOS
#include "PDL.h"
#include <syslog.h>
#endif

#ifdef NME_MIXER
#include <SDL_mixer.h>
#endif

namespace nme
{

void MainLoop();

static int sgDesktopWidth = 0;
static int sgDesktopHeight = 0;

class SDLSurf : public Surface
{
public:
   SDLSurf(SDL_Surface *inSurf,bool inDelete) : mSurf(inSurf)
   {
      mDelete = inDelete;
   }
   ~SDLSurf()
   {
      if (mDelete)
         SDL_FreeSurface(mSurf);
   }

   int Width() const  { return mSurf->w; }
   int Height() const  { return mSurf->h; }
   PixelFormat Format()  const
   {
      if (mSurf->flags & SDL_SRCALPHA)
          return pfARGB;
      return pfXRGB;
   }
   const uint8 *GetBase() const { return (const uint8 *)mSurf->pixels; }
   int GetStride() const { return mSurf->pitch; }

   void Clear(uint32 inColour,const Rect *inRect)
   {
      SDL_Rect r;
      SDL_Rect *rect_ptr = 0;
      if (inRect)
      {
         rect_ptr = &r;
         r.x = inRect->x;
         r.y = inRect->y;
         r.w = inRect->w;
         r.h = inRect->h;
      }

      SDL_FillRect(mSurf,rect_ptr,SDL_MapRGBA(mSurf->format,
            inColour>>16, inColour>>8, inColour, inColour>>24 )  );
   }

   RenderTarget BeginRender(const Rect &inRect)
   {
      if (SDL_MUSTLOCK(mSurf) )
         SDL_LockSurface(mSurf);
      return RenderTarget(Rect(Width(),Height()), Format(),
         (uint8 *)mSurf->pixels, mSurf->pitch);
   }
   void EndRender()
   {
      if (SDL_MUSTLOCK(mSurf) )
         SDL_UnlockSurface(mSurf);
   }

   void BlitTo(const RenderTarget &outTarget,
               const Rect &inSrcRect,int inPosX, int inPosY,
               BlendMode inBlend, const BitmapCache *inMask,
               uint32 inTint=0xffffff ) const
   {
   }
	void BlitChannel(const RenderTarget &outTarget, const Rect &inSrcRect,
									 int inPosX, int inPosY,
									 int inSrcChannel, int inDestChannel ) const
	{
	}

   void StretchTo(const RenderTarget &outTarget,
          const Rect &inSrcRect, const DRect &inDestRect) const
   {
   }

   SDL_Surface *mSurf;
   bool  mDelete;
};

SDL_Surface *SurfaceToSDL(Surface *inSurface)
{
   int swap =  (gC0IsRed!=(bool)(inSurface->Format()&pfSwapRB)) ? 0xff00ff : 0;
   return SDL_CreateRGBSurfaceFrom((void *)inSurface->Row(0),
             inSurface->Width(), inSurface->Height(),
             32, inSurface->Width()*4,
             0x00ff0000^swap, 0x0000ff00,
             0x000000ff^swap, 0xff000000 );
}


SDL_Cursor *CreateCursor(const char *image[],int inHotX,int inHotY)
{
  int i, row, col;
  Uint8 data[4*32];
  Uint8 mask[4*32];

  i = -1;
  for ( row=0; row<32; ++row ) {
    for ( col=0; col<32; ++col ) {
      if ( col % 8 ) {
        data[i] <<= 1;
        mask[i] <<= 1;
      } else {
        ++i;
        data[i] = mask[i] = 0;
      }
      switch (image[row][col]) {
        case 'X':
          data[i] |= 0x01;
          mask[i] |= 0x01;
          break;
        case '.':
          mask[i] |= 0x01;
          break;
        case ' ':
          break;
      }
    }
  }
  return SDL_CreateCursor(data, mask, 32, 32, inHotX, inHotY);
}

SDL_Cursor *sDefaultCursor = 0;
SDL_Cursor *sTextCursor = 0;
SDL_Cursor *sHandCursor = 0;



class SDLStage : public Stage
{
public:
   SDLStage(SDL_Surface *inSurface,uint32 inFlags,bool inIsOpenGL,
       int inWidth, int inHeight)
   {
      mWidth = inWidth;
      mHeight = inHeight;

      mIsOpenGL = inIsOpenGL;
      mSDLSurface = inSurface;
      mFlags = inFlags;
      mShowCursor = true;
      mCurrentCursor = curPointer;

      mIsFullscreen =  (mFlags & SDL_FULLSCREEN);
      if (mIsFullscreen)
         displayState = sdsFullscreenInteractive;

      if (mIsOpenGL)
      {
         mOpenGLContext = HardwareContext::CreateOpenGL(0,0);
         mOpenGLContext->IncRef();
         mOpenGLContext->SetWindowSize(inSurface->w, inSurface->h);
         mPrimarySurface = new HardwareSurface(mOpenGLContext);
      }
      else
      {
         mOpenGLContext = 0;
         mPrimarySurface = new SDLSurf(inSurface,inIsOpenGL);
      }
      mPrimarySurface->IncRef();
   }

   ~SDLStage()
   {
      if (!mIsOpenGL)
         SDL_FreeSurface(mSDLSurface);
      else
         mOpenGLContext->DecRef();
      mPrimarySurface->DecRef();
   }

   void Resize(int inWidth,int inHeight)
   {
      #ifndef __APPLE__
	  #ifndef WEBOS
      if (mIsOpenGL)
      {
         // Little hack to help windows
         mSDLSurface->w = inWidth;
         mSDLSurface->h = inHeight;
         mOpenGLContext->SetWindowSize(inWidth,inHeight);
      }
      else
	  #endif
      #endif
      {
         // Calling this recreates the gl context and we loose all our textures and
         // display lists. So Work around it.
         gTextureContextVersion++;
 
         mSDLSurface = SDL_SetVideoMode(inWidth, inHeight, 32, mFlags );
  
         if (mIsOpenGL)
         {
            //nme_resize_id ++;
            mOpenGLContext->DecRef();
            mOpenGLContext = HardwareContext::CreateOpenGL(0,0);
            mOpenGLContext->SetWindowSize(inWidth, inHeight);
            mOpenGLContext->IncRef();
            mPrimarySurface->DecRef();
            mPrimarySurface = new HardwareSurface(mOpenGLContext);
         }
         else
         {
            mPrimarySurface->DecRef();
            mPrimarySurface = new SDLSurf(mSDLSurface,mIsOpenGL);
         }
         mPrimarySurface->IncRef();
      }
   }

   void SetFullscreen(bool inFullscreen)
   {
      if (inFullscreen != mIsFullscreen)
      {
         mIsFullscreen = inFullscreen;
         //printf("SetFullscreen %d\n",inFullscreen);

         // Calling this recreates the gl context and we loose all our textures and
         // display lists. So Work around it.
         gTextureContextVersion++;

         int w = mIsFullscreen ? sgDesktopWidth : mWidth;
         int h = mIsFullscreen ? sgDesktopHeight : mHeight;
         if (mIsFullscreen)
            mFlags |= SDL_FULLSCREEN;
         else
            mFlags &= ~SDL_FULLSCREEN;

         //printf("Set %dx%d %d\n", w,h,mFlags & SDL_FULLSCREEN);
         mSDLSurface = SDL_SetVideoMode(w, h, 32, mFlags);


         w = mSDLSurface->w;
         h = mSDLSurface->h;
         if (mIsOpenGL)
         {
            //nme_resize_id ++;
            mOpenGLContext->DecRef();
            mOpenGLContext = HardwareContext::CreateOpenGL(0,0);
            mOpenGLContext->SetWindowSize(w, h);
            mOpenGLContext->IncRef();
            mPrimarySurface->DecRef();
            mPrimarySurface = new HardwareSurface(mOpenGLContext);
         }
         else
         {
            mPrimarySurface->DecRef();
            mPrimarySurface = new SDLSurf(mSDLSurface,mIsOpenGL);
         }
         mPrimarySurface->IncRef();


         Event resize(etResize,w,h);
         ProcessEvent(resize);
      }
   }



   bool isOpenGL() const { return mOpenGLContext; }

   void ProcessEvent(Event &inEvent)
   {
      HandleEvent(inEvent);
   }


   void Flip()
   {
      if (mIsOpenGL)
      {
         SDL_GL_SwapBuffers();
      }
      else
      {
         SDL_Flip( mSDLSurface );
      }
   }
   void GetMouse()
   {
   }

   void SetCursor(Cursor inCursor)
   {
	  #ifndef WEBOS
      if (sDefaultCursor==0)
         sDefaultCursor = SDL_GetCursor();

      mCurrentCursor = inCursor;
   
      if (inCursor==curNone || !mShowCursor)
         SDL_ShowCursor(false);
      else
      {
         SDL_ShowCursor(true);
   
         if (inCursor==curPointer)
            SDL_SetCursor(sDefaultCursor);
         else if (inCursor==curHand)
         {
            if (!sHandCursor)
               sHandCursor = CreateCursor(sHandCursorData,1,13);
            SDL_SetCursor(sHandCursor);
         }
         else
         {
            // TODO: Rotated
            if (sTextCursor==0)
               sTextCursor = CreateCursor(sTextCursorData,2,13);
            SDL_SetCursor(sTextCursor);
         }
      }
	  #endif
   }

   void ShowCursor(bool inShow)
   {
      if (inShow!=mShowCursor)
      {
         mShowCursor = inShow;
         this->SetCursor(mCurrentCursor);
      }
   }
   
   
   bool getMultitouchSupported() { 
	   #ifdef WEBOS
	   return true;
	   #else
	   return false;
	   #endif
   }
   void setMultitouchActive(bool inActive) { mMultiTouch = inActive; }
   bool getMultitouchActive() {
	   #ifdef WEBOS
	   return mMultiTouch;
	   #else
	   return false;
	   #endif
   }
   
   bool mMultiTouch;
   int  mSingleTouchID;
   

   Surface *GetPrimarySurface()
   {
      return mPrimarySurface;
   }

   HardwareContext *mOpenGLContext;
   SDL_Surface *mSDLSurface;
   Surface     *mPrimarySurface;
   double       mFrameRate;
   bool         mIsOpenGL;
   Cursor       mCurrentCursor;
   bool         mShowCursor;
   bool         mIsFullscreen;
   unsigned int mFlags;
   int          mWidth;
   int          mHeight;
};


class SDLFrame : public Frame
{
public:
   SDLFrame(SDL_Surface *inSurface, uint32 inFlags, bool inIsOpenGL,int inW,int inH)
   {
      mFlags = inFlags;
      mIsOpenGL = inIsOpenGL;
      mStage = new SDLStage(inSurface,mFlags,inIsOpenGL,inW,inH);
      mStage->IncRef();
      // SetTimer(mHandle,timerFrame, 10,0);
   }
   ~SDLFrame()
   {
      mStage->DecRef();
   }

   void ProcessEvent(Event &inEvent)
   {
      mStage->ProcessEvent(inEvent);
   }
   void Resize(int inWidth,int inHeight)
   {
      mStage->Resize(inWidth,inHeight);
   }

  // --- Frame Interface ----------------------------------------------------

   void SetTitle()
   {
   }
   void SetIcon()
   {
   }
   Stage *GetStage()
   {
      return mStage;
   }


   SDLStage *mStage;
   bool   mIsOpenGL;
   uint32 mFlags;
   
   double mAccX;
   double mAccY;
   double mAccZ;
};


// --- When using the simple window class -----------------------------------------------

extern "C" void MacBoot( /*void (*)()*/ );


SDLFrame *sgSDLFrame = 0;
SDL_Joystick *joystick = 0;

void CreateMainFrame(FrameCreationCallback inOnFrame,int inWidth,int inHeight,
   unsigned int inFlags, const char *inTitle, const char *inPackage, Surface *inIcon )
{
#ifdef HX_MACOS
   MacBoot();
#endif
#ifdef WEBOS
   openlog (inPackage, 0, LOG_USER);
   if (PDL_GetPDKVersion () >= 100)
	  PDL_Init(0);
#endif
	
   unsigned int sdl_flags = 0;
   bool fullscreen = (inFlags & wfFullScreen) != 0;
   bool opengl = (inFlags & wfHardware) != 0;
   bool resizable = (inFlags & wfResizable) != 0;

   Rect r(100,100,inWidth,inHeight);

   Uint32 init_flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;
   if (opengl)
      init_flags |= SDL_OPENGL;

   //  SDL_GL_DEPTH_SIZE = 0;

   init_flags |= SDL_INIT_JOYSTICK;

   int err =  SDL_Init( init_flags );
   if ( err == -1 )
   {
      fprintf(stderr,"Couble not initialize SDL : %s\n", SDL_GetError());
      inOnFrame(0);
      // SDL_GetError()
      return;
   }

   SDL_EnableUNICODE(1);
   SDL_EnableKeyRepeat(500,30);

   gSDLIsInit = true;

   #ifdef NME_MIXER
   
   #ifdef WEBOS
   int chunksize = 256;
   #else
   int chunksize = 4096;
   #endif
   
   if ( Mix_OpenAudio( MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, chunksize)!= 0 )
   {
      fprintf(stderr,"Couble not open sound: %s\n", Mix_GetError());
      gSDLIsInit = false;
   }
   #endif


   const SDL_VideoInfo *info  = SDL_GetVideoInfo();
   sgDesktopWidth = info->current_w;
   sgDesktopHeight = info->current_h;

   sdl_flags = SDL_HWSURFACE;

   if ( resizable )
      sdl_flags |= SDL_RESIZABLE;

   if ( fullscreen )
   {
      sdl_flags |= SDL_FULLSCREEN;
   }

   int use_w = fullscreen ? 0 : inWidth;
   int use_h = fullscreen ? 0 : inHeight;

#ifdef IPHONE
   sdl_flags |= SDL_NOFRAME;
#else
   if (inIcon)
   {
      SDL_Surface *sdl = SurfaceToSDL(inIcon);
      SDL_WM_SetIcon( sdl, NULL );
   }
#endif


   #ifndef IPHONE
   #ifndef WEBOS
   SDL_WM_SetCaption( inTitle, 0 );
   #endif
   #endif

   SDL_Surface* screen = 0;
   bool is_opengl = false;
   int  aa_tries = (inFlags & wfHW_AA) ? ( (inFlags & wfHW_AA_HIRES) ? 2 : 1 ) : 0;

   if (opengl)
   {
      for(int pass=0;pass<3;pass++)
      {
         #ifdef WEBOS
         pass = 2;
         #endif
         /* Initialize the display */
         for(int aa_pass = aa_tries; aa_pass>=0; --aa_pass)
         {
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE,  8 );
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8 );
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8 );
			#ifdef WEBOS
		 	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
         	#endif
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  32 - pass*8 );
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, aa_pass>0);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  1<<aa_pass );

            if ( inFlags & wfVSync )
            {
               SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
            }

            sdl_flags |= SDL_OPENGL;
            if (!(screen = SDL_SetVideoMode( use_w, use_h, 32, sdl_flags | SDL_OPENGL)))
            {
               if (pass==2 && aa_pass==0)
               {
                  sdl_flags &= ~SDL_OPENGL;
                  fprintf(stderr, "Couldn't set OpenGL mode: %s\n", SDL_GetError());
               }
            }
            else
            {
              is_opengl = true;
              break;
            }
         }
      }
   }


   if (!screen)
   {
      sdl_flags |= SDL_DOUBLEBUF;
      screen = SDL_SetVideoMode( use_w, use_h, 32, sdl_flags );
      //printf("Flags %p\n",sdl_flags);
      if (!screen)
      {
         fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
         inOnFrame(0);
         gSDLIsInit = false;
         return;
      }
   }


   HintColourOrder( opengl || screen->format->Rmask==0xff );

   #ifdef WEBOS
   PDL_ScreenTimeoutEnable(PDL_TRUE);
   joystick = SDL_JoystickOpen(0);
   #endif

   sgSDLFrame =  new SDLFrame( screen, sdl_flags, is_opengl, inWidth, inHeight );

   inOnFrame(sgSDLFrame);

   MainLoop();
}

bool sgDead = false;

void TerminateMainLoop()
{
   #ifdef NME_MIXER
   Mix_CloseAudio();
   #endif
   #ifdef WEBOS
   closelog();
   PDL_Quit();
   #endif
   sgDead = true;
}

static SDL_TimerID  sgTimerID = 0;
bool sgTimerActive = false;

Uint32 OnTimer(Uint32 interval, void *)
{
    // Ping off an event - any event will force the frame check.
    SDL_Event event;
    SDL_UserEvent userevent;
    /* In this example, our callback pushes an SDL_USEREVENT event
    into the queue, and causes ourself to be called again at the
    same interval: */
    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = NULL;
    userevent.data2 = NULL;
    event.type = SDL_USEREVENT;
    event.user = userevent;
    sgTimerActive = false;
    sgTimerID = 0;
    SDL_PushEvent(&event);
    return 0;
}


void AddModStates(int &ioFlags,int inState = -1)
{
   int state = inState==-1 ? SDL_GetModState() : inState;
   if (state & KMOD_SHIFT) ioFlags |= efShiftDown;
   if (state & KMOD_CTRL) ioFlags |= efCtrlDown;
   if (state & KMOD_ALT) ioFlags |= efAltDown;
   if (state & KMOD_META) ioFlags |= efCommandDown;

   int m = SDL_GetMouseState(0,0);
   if ( m & SDL_BUTTON(1) ) ioFlags |= efLeftDown;
   if ( m & SDL_BUTTON(2) ) ioFlags |= efMiddleDown;
   if ( m & SDL_BUTTON(3) ) ioFlags |= efRightDown;

   ioFlags |= efNoNativeClick;
}

#define SDL_TRANS(x) case SDLK_##x: return key##x;

int SDLKeyToFlash(int inKey,bool &outRight)
{
   outRight = (inKey==SDLK_RSHIFT || inKey==SDLK_RCTRL ||
               inKey==SDLK_RALT || inKey==SDLK_RMETA || inKey==SDLK_RSUPER);
   if (inKey>=keyA && inKey<=keyZ)
      return inKey;
   if (inKey>=SDLK_0 && inKey<=SDLK_9)
      return inKey - SDLK_0 + keyNUMBER_0;
   if (inKey>=SDLK_KP0 && inKey<=SDLK_KP9)
      return inKey - SDLK_KP0 + keyNUMPAD_0;

   if (inKey>=SDLK_F1 && inKey<=SDLK_F15)
      return inKey - SDLK_F1 + keyF1;


   switch(inKey)
   {
      case SDLK_RALT:
      case SDLK_LALT:
         return keyALTERNATE;
      case SDLK_RSHIFT:
      case SDLK_LSHIFT:
         return keySHIFT;
      case SDLK_RCTRL:
      case SDLK_LCTRL:
         return keyCONTROL;
      case SDLK_LMETA:
      case SDLK_RMETA:
         return keyCOMMAND;

      case SDLK_CAPSLOCK: return keyCAPS_LOCK;
      case SDLK_PAGEDOWN: return keyPAGE_DOWN;
      case SDLK_PAGEUP: return keyPAGE_UP;
      case SDLK_EQUALS: return keyEQUAL;
      case SDLK_RETURN:
      case SDLK_KP_ENTER:
         return keyENTER;

      SDL_TRANS(BACKQUOTE)
      SDL_TRANS(BACKSLASH)
      SDL_TRANS(BACKSPACE)
      SDL_TRANS(COMMA)
      SDL_TRANS(DELETE)
      SDL_TRANS(DOWN)
      SDL_TRANS(END)
      SDL_TRANS(ESCAPE)
      SDL_TRANS(HOME)
      SDL_TRANS(INSERT)
      SDL_TRANS(LEFT)
      SDL_TRANS(LEFTBRACKET)
      SDL_TRANS(MINUS)
      SDL_TRANS(PERIOD)
      SDL_TRANS(QUOTE)
      SDL_TRANS(RIGHT)
      SDL_TRANS(RIGHTBRACKET)
      SDL_TRANS(SEMICOLON)
      SDL_TRANS(SLASH)
      SDL_TRANS(SPACE)
      SDL_TRANS(TAB)
      SDL_TRANS(UP)
   }

   return inKey;
}

std::map<int,wchar_t> sLastUnicode;


void ProcessEvent(SDL_Event &inEvent)
{

  switch(inEvent.type)
   {
      case SDL_QUIT:
      {
         Event close(etQuit);
         sgSDLFrame->ProcessEvent(close);
         break;
      }
	   case SDL_ACTIVEEVENT:
      {
         if (inEvent.active.state & SDL_APPINPUTFOCUS)
         {
            Event activate( inEvent.active.gain ? etGotInputFocus : etLostInputFocus );
            sgSDLFrame->ProcessEvent(activate);
         }
	
         if (inEvent.active.state & SDL_APPACTIVE)
         {
            Event activate( inEvent.active.gain ? etActivate : etDeactivate );
            sgSDLFrame->ProcessEvent(activate);
         }
		   break;
	   }
      case SDL_MOUSEMOTION:
      {
         Event mouse(etMouseMove,inEvent.motion.x,inEvent.motion.y);
         AddModStates(mouse.flags);
		 #ifdef WEBOS
		 mouse.value = inEvent.motion.which;
		 #endif
         sgSDLFrame->ProcessEvent(mouse);
         break;
      }
      case SDL_MOUSEBUTTONDOWN:
      {
         Event mouse(etMouseDown,inEvent.button.x,inEvent.button.y,inEvent.button.button-1);
         AddModStates(mouse.flags);
		 #ifdef WEBOS
		 mouse.value = inEvent.button.which;
		 #endif
         sgSDLFrame->ProcessEvent(mouse);
         break;
      }
      case SDL_MOUSEBUTTONUP:
      {
         Event mouse(etMouseUp,inEvent.button.x,inEvent.button.y,inEvent.button.button-1);
         AddModStates(mouse.flags);
		 #ifdef WEBOS
		 mouse.value = inEvent.button.which;
		 #endif
         sgSDLFrame->ProcessEvent(mouse);
         break;
      }

      case SDL_KEYDOWN:
      case SDL_KEYUP:
      {
         Event key(inEvent.type==SDL_KEYDOWN ? etKeyDown : etKeyUp );
         bool right;
         key.value = SDLKeyToFlash(inEvent.key.keysym.sym,right);
         if (inEvent.type==SDL_KEYDOWN)
         {
            key.code = inEvent.key.keysym.unicode;
            sLastUnicode[inEvent.key.keysym.scancode] = key.code;
         }
         else
            // SDL does not provide unicode on key up, so remember it,
            //  keyed by scancode
            key.code = sLastUnicode[inEvent.key.keysym.scancode];

         AddModStates(key.flags,inEvent.key.keysym.mod);
         if (right)
            key.flags |= efLocationRight;
         sgSDLFrame->ProcessEvent(key);
         break;
      }

	  case SDL_VIDEOEXPOSE:
	  {
			Event poll(etPoll);
			sgSDLFrame->ProcessEvent(poll);
         break;
	  }
      case SDL_VIDEORESIZE:
      {
         Event resize(etResize,inEvent.resize.w,inEvent.resize.h);
         sgSDLFrame->Resize(inEvent.resize.w,inEvent.resize.h);
         sgSDLFrame->ProcessEvent(resize);
         break;
      }
   }
}


#ifdef WEBOS

bool GetAcceleration(double &outX, double &outY, double &outZ)
{
	outX = SDL_JoystickGetAxis(joystick, 0) / 32768.0;
	outY = SDL_JoystickGetAxis(joystick, 1) / 32768.0;
	outZ = SDL_JoystickGetAxis(joystick, 2) / 32768.0;
	return true;
}

#endif


void MainLoop()
{
   SDL_Event event;
   while(!sgDead)
   {
      while (!sgDead && SDL_PollEvent(&event) )
      {
         ProcessEvent(event);
         event.type = -1;
         if (sgDead) break;
      }

     
      if (sgDead)
         break;

      Event poll(etPoll);
      sgSDLFrame->ProcessEvent(poll);

      // Sleep if required...
      double next = sgSDLFrame->GetStage()->GetNextWake() - GetTimeStamp();
      if (next > 0.001)
      {
         int snooze = next*1000.0;
         sgTimerActive = true;
         sgTimerID = SDL_AddTimer(snooze, OnTimer, 0);

         event.type = -1;
         SDL_WaitEvent(&event);

         if (sgTimerActive && sgTimerID)
         {
            SDL_RemoveTimer(sgTimerID);
            sgTimerActive = false;
            sgTimerID = 0;
         }
         ProcessEvent(event);
      }
   }

   Event kill(etDestroyHandler);
   sgSDLFrame->ProcessEvent(kill);
   SDL_Quit();
}


/*
Frame *CreateTopLevelWindow(int inWidth,int inHeight,unsigned int inFlags, wchar_t *inTitle, wchar_t *inIcon )
{
   return 0;
}

*/

} // end namespace nme


         #if 0
         if (event.type == SDL_JOYAXISMOTION)
         {
       alloc_field( evt, val_id( "type" ), alloc_int( et_jaxis ) );
       alloc_field( evt, val_id( "axis" ), alloc_int( event.jaxis.axis ) );
       alloc_field( evt, val_id( "value" ), alloc_int( event.jaxis.value ) );
       alloc_field( evt, val_id( "which" ), alloc_int( event.jaxis.which ) );
       return evt;
         }
         if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP)
         {
       alloc_field( evt, val_id( "type" ), alloc_int( et_jbutton ) );
       alloc_field( evt, val_id( "button" ), alloc_int( event.jbutton.button ) );
       alloc_field( evt, val_id( "state" ), alloc_int( event.jbutton.state ) );
       alloc_field( evt, val_id( "which" ), alloc_int( event.jbutton.which ) );
       return evt;
         }
         if (event.type == SDL_JOYHATMOTION)
         {
       alloc_field( evt, val_id( "type" ), alloc_int( et_jhat ) );
       alloc_field( evt, val_id( "button" ), alloc_int( event.jhat.hat ) );
       alloc_field( evt, val_id( "value" ), alloc_int( event.jhat.value ) );
       alloc_field( evt, val_id( "which" ), alloc_int( event.jhat.which ) );
       return evt;
         }
         if (event.type == SDL_JOYBALLMOTION)
         {
       alloc_field( evt, val_id( "type" ), alloc_int( et_jball ) );
       alloc_field( evt, val_id( "ball" ), alloc_int( event.jball.ball ) );
       alloc_field( evt, val_id( "xrel" ), alloc_int( event.jball.xrel ) );
       alloc_field( evt, val_id( "yrel" ), alloc_int( event.jball.yrel ) );
       alloc_field( evt, val_id( "which" ), alloc_int( event.jball.which ) );
       return evt;
         }

         if (event.type==SDL_VIDEORESIZE)
         {
       alloc_field( evt, val_id( "type" ), alloc_int( et_resize ) );
       alloc_field( evt, val_id( "width" ), alloc_int( event.resize.w ) );
       alloc_field( evt, val_id( "height" ), alloc_int( event.resize.h ) );
       return evt;
         }
         #endif


#if 0
/*
 */



value nme_get_mouse_position()
{
   int x,y;

   #ifdef SDL13
   SDL_GetMouseState(0,&x,&y);
   #else
   SDL_GetMouseState(&x,&y);
   #endif

   value pos = alloc_empty_object();
   alloc_field( pos, val_id( "x" ), alloc_int( x ) );
   alloc_field( pos, val_id( "y" ), alloc_int( y ) );
   return pos;
}


#endif
