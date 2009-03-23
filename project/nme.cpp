/*
 * Copyright (c) 2006, Lee McColl Sylvester - www.designrealm.co.uk
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE HAXE PROJECT CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE HAXE PROJECT CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */
 
#include "nsdl.h"
#include "nme.h"
#include "Scrap.h"
#include <stack>
#include <string.h>
#include <SDL_ttf.h>
#include <SDL_rwops.h>

#ifdef WIN32
#include <windows.h>
#endif
#include <SDL_opengl.h>

#ifndef empty_object
#define empty_object alloc_object(0)
#endif


int nme_resize_id = 0;

using namespace std;

// helper functions


DEFINE_KIND( k_surf );
DEFINE_KIND( k_snd );
DEFINE_KIND( k_mus );


// As opposed to opengl hardware ..
static bool sUseSystemHardware = false;

SDL_Surface *ConvertToPreferredFormat(SDL_Surface *inSurface)
{
   unsigned int flags = sUseSystemHardware ?SDL_HWSURFACE:SDL_SWSURFACE;

   SDL_PixelFormat fmt;
   memset(&fmt,0,sizeof(fmt));
   fmt.BitsPerPixel = 32;
   fmt.BytesPerPixel = 4;

   if (inSurface->flags & SDL_SRCALPHA)
   {
      flags |= SDL_SRCALPHA;
      fmt.Amask = 0xff000000;
      fmt.Ashift = 24;
   }
   fmt.Rmask = 0xff0000;
   fmt.Rshift = 16;
   fmt.Gmask = 0xff00;
   fmt.Gshift = 8;
   fmt.Bmask = 0xff;
   fmt.Bshift = 0;

   fmt.alpha = 255;
   fmt.colorkey = 0xff000000;


   return SDL_ConvertSurface(inSurface,&fmt,flags);
}


SDL_Surface* nme_loadimage( value file )
{
   val_check( file, string );

   SDL_Surface* surf;
   surf = IMG_Load( val_string( file ) );
   if ( !surf )
     	surf = SDL_LoadBMP( val_string( file ) );
   if ( !surf )
     	return NULL;
   SDL_Surface *surface = ConvertToPreferredFormat( surf );
  	SDL_FreeSurface( surf );
   return surface;
}

struct MyRWOps : SDL_RWops
{
   MyRWOps(const char *inItems,int inLen)
   {
      mItems = (unsigned char *)inItems;
      mLen = inLen;
      mPos = 0;

      SDL_RWops::seek = &MyRWOps::s_seek;
      SDL_RWops::read = &MyRWOps::s_read;
      SDL_RWops::write = &MyRWOps::s_write;
      SDL_RWops::close = &MyRWOps::s_close;
   }

   int seek(int offset,int whence)
   {
      switch(whence)
      {
         case SEEK_SET: mPos = offset; return mPos;
         case SEEK_CUR: mPos += offset; return mPos;
         case SEEK_END: mPos = mLen-offset; return mPos;
      }
      return 0;
   }

/* Read up to 'num' objects each of size 'objsize' from the data
   source to the area pointed at by 'ptr'.
   Returns the number of objects read, or -1 if the read failed.
 */
   int read(void *ptr, int size, int maxnum)
   {
      unsigned char *p = (unsigned char *)ptr;
      int bytes = size*maxnum;
      int i;
      for(i=0;i<bytes;i++)
      {
         *p++ = mItems[mPos++];
      }
      return maxnum;
   }

/* Write exactly 'num' objects each of size 'objsize' from the area
   pointed at by 'ptr' to data source.
   Returns 'num', or -1 if the write failed.
 */
   int write(const void *ptr, int size, int num)
   {
      return 0;
   }

   int close()
   {
      return 1;
   }



   static int s_seek(struct SDL_RWops *context, int offset, int whence)
   {
      MyRWOps *ops = (MyRWOps *)context;
      return ops->seek(offset,whence);
   }

   static int s_read(struct SDL_RWops *context, void *ptr, int size, int maxnum)
   {
      MyRWOps *ops = (MyRWOps *)context;
      return ops->read(ptr,size,maxnum);
   }

   static int s_write(struct SDL_RWops *context, const void *ptr, int size, int num)
   {
      MyRWOps *ops = (MyRWOps *)context;
      return ops->write(ptr,size,num);
   }

   static int s_close(struct SDL_RWops *context)
   {
      MyRWOps *ops = (MyRWOps *)context;
      return ops->close();
   }

   unsigned char *mItems;
   int   mLen;
   int   mPos;
};


SDL_Surface* nme_loadimage_from_bytes( value inBytes, value inLen, value inType,
 value inAlpha, value inAlphaLen)
{
	val_check( inBytes, string );
	val_check( inType, string );
	val_check( inLen, int );
	val_check( inAlpha, string );
	val_check( inAlphaLen, int );

        int len = val_int(inLen);
        const char *items = val_string(inBytes);
        const char *type = val_string(inType);


        MyRWOps rw_ops(items,len);

	SDL_Surface* surf;
	surf = IMG_LoadTyped_RW(&rw_ops,0,(char *)type);
	if ( !surf )
	   return NULL;

   SDL_Surface *surface = ConvertToPreferredFormat( surf );
  	SDL_FreeSurface( surf );
   return surface;

   if (inAlphaLen>0)
   {
      // TODO: Need a test image to test this...
   }


	return surf;
}


// creates a surface with alpha channel
value nme_create_image_32( value flags, value width, value height )
{
	val_check( flags, int );
	val_check( width, int );
	val_check( height, int );

//	return alloc_abstract( k_surf, sge_CreateAlphaSurface( val_int( flags ), val_int( width ), val_int( height ) ) );
	return alloc_int(0);
}

value nme_copy_surface( value surf )
{
	return alloc_int(0);
	/*
	val_check_kind( surf, k_surf );
	
	SDL_Surface* scr = SURFACE(surf);

	return alloc_abstract( k_surf, sge_copy_surface( scr ) );
	*/
}



// surface relative functions




static value nme_surface_clear( value surf, value c )
{
	val_check_kind( surf, k_surf );
	
	val_check( c, int );
	SDL_Surface* scr = SURFACE(surf);

	Uint8 r = RRGB( c );
	Uint8 g = GRGB( c );
	Uint8 b = BRGB( c );

        if (IsOpenGLScreen(scr))
        {
           int w = scr->w;
           int h = scr->h;
           glDisable(GL_CLIP_PLANE0);
           glViewport(0,0,w,h);
           glMatrixMode(GL_PROJECTION);
           glLoadIdentity();
           glOrtho(0,w, h,0, -1000,1000);
           glMatrixMode(GL_MODELVIEW);
           glLoadIdentity();
           glClearColor((GLclampf)(r/255.0),
                        (GLclampf)(g/255.0),
                        (GLclampf)(b/255.0),
                        (GLclampf)1.0 );
           glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        else
        {
	   SDL_FillRect( scr, NULL, SDL_MapRGB( scr->format, r, g, b ) );
        }

	return alloc_int( 0 );
	
}

value nme_surface_width( value surface )
{
	val_check_kind( surface, k_surf );

	SDL_Surface* surf = SURFACE(surface);
        return alloc_int(surf->w);
}

value nme_surface_height( value surface )
{
	val_check_kind( surface, k_surf );

	SDL_Surface* surf = SURFACE(surface);
        return alloc_int(surf->h);
}


void nme_surface_free( value surface )
{
	if ( val_is_kind( surface, k_surf ) )
	{
		val_gc( surface, NULL );

		SDL_Surface* surf = SURFACE( surface );
		SDL_FreeSurface( surf );
	}
}

value nme_surface_colourkey( value surface, value r, value g, value b )
{
	val_check_kind( surface, k_surf );
	val_check( r, int );
	val_check( g, int );
	val_check( b, int );

	SDL_Surface* surf = SURFACE( surface );
	if( !surf )
		return alloc_bool( false );
	unsigned int key = SDL_MapRGB( surf->format, val_int( r ), val_int( g ), val_int( b ) );
	SDL_SetColorKey( surf, SDL_RLEACCEL | SDL_SRCCOLORKEY, key );
	return alloc_bool( true );
}




// screen relative functions

static bool sOpenGL = false;
SDL_Surface *sOpenGLScreen = 0;
SDL_Surface *gCurrentScreen = 0;
static bool sDoScissor;
SDL_Rect sScissorRect;
static Uint32 sFlags = 0;

bool IsOpenGLMode() { return sOpenGL; }
bool IsOpenGLScreen(SDL_Surface *inSurface) { return sOpenGL && inSurface==sOpenGLScreen; }


value nme_delay( value period )
{
	val_check( period, int );
	SDL_Delay( val_int( period ) );
	return alloc_int( 0 );
}

value nme_flipbuffer( value buff )
{
   if (sOpenGL)
   {
	SDL_GL_SwapBuffers();
   }
   else
   {
	val_check_kind( buff, k_surf );

	SDL_Surface* srf = SURFACE( buff );
	SDL_Flip( srf );
	SDL_Delay( 1 );
   }

    return alloc_int( 0 );
}

value nme_swapbuffer()
{
	return alloc_int( 0 );
}


/*
value nme_screen_flip()
{
	value o = val_this();
	val_check( o, object );
	nme_flipbuffer( val_field( o, val_id( "screen" ) ) );
	return alloc_int( 0 );
}
*/

value AllocRect(const SDL_Rect &inRect)
{
   value r = empty_object;
   alloc_field( r, val_id( "x" ), alloc_float( inRect.x ) );
   alloc_field( r, val_id( "y" ), alloc_float( inRect.y ) );
   alloc_field( r, val_id( "w" ), alloc_float( inRect.w ) );
   alloc_field( r, val_id( "h" ), alloc_float( inRect.h ) );
   return r;
}

value nme_set_clip_rect(value inSurface, value inRect)
{
   SDL_Rect rect;
   if (!val_is_null(inRect))
   {
      rect.x = (int)val_number( val_field(inRect, val_id("x")) );
      rect.y = (int)val_number( val_field(inRect, val_id("y")) );
      rect.w = (int)val_number( val_field(inRect, val_id("width")) );
      rect.h = (int)val_number( val_field(inRect, val_id("height")) );

   }
   else
      memset(&rect,0,sizeof(rect));

   if (val_is_kind(inSurface,k_surf))
   {
      SDL_Surface *surface = SURFACE(inSurface);

      if (IsOpenGLScreen(surface))
      {
         if (val_is_null(inRect))
         {
            sDoScissor = false;
            glDisable(GL_SCISSOR_TEST);
         }
         else
         {
            sDoScissor = true;
            glEnable(GL_SCISSOR_TEST);
            sScissorRect = rect;
            glScissor(sScissorRect.x,sScissorRect.y,
                      sScissorRect.w,sScissorRect.h);
         }
      }
      else
      {
         if (val_is_null(inRect))
         {
            SDL_SetClipRect(surface,0);
            SDL_GetClipRect(surface,&rect);
         }
         else
         {
            SDL_SetClipRect(surface,&rect);
         }
      }
   }

   return AllocRect(rect);
}

value nme_get_clip_rect(value inSurface)
{
   SDL_Rect rect;
   memset(&rect,0,sizeof(rect));

   if (val_is_kind(inSurface,k_surf))
   {
      SDL_Surface *surface = SURFACE(inSurface);

      if (IsOpenGLScreen(surface))
      {
         if (sDoScissor)
            rect = sScissorRect;
         else
         {
            rect.w = sOpenGLScreen->w;
            rect.h = sOpenGLScreen->h;
         }
      }
      else
      {
         SDL_GetClipRect(surface,&rect);
      }
   }

   return AllocRect(rect);
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

static const char *sTextCursorData[] = {
  "                                ",
  "                                ",
  "XX XX                           ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "  X                             ",
  "XX XX                           ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
};



#define CURSOR_NONE   0
#define CURSOR_NORMAL 1
#define CURSOR_TEXT   2

SDL_Cursor *sDefaultCursor = 0;
SDL_Cursor *sTextCursor = 0;


value nme_set_cursor(value inCursor)
{
   val_check(inCursor,int);

   if (sDefaultCursor==0)
      sDefaultCursor = SDL_GetCursor();

   int c = val_int(inCursor);

   if (c==CURSOR_NONE)
      SDL_ShowCursor(false);
   else
   {
      SDL_ShowCursor(true);

      if (c==CURSOR_NORMAL)
         SDL_SetCursor(sDefaultCursor);
      else
      {
         if (sTextCursor==0)
            sTextCursor = CreateCursor(sTextCursorData,1,13);
         SDL_SetCursor(sTextCursor);
      }
   }

   return alloc_int(0);
}

value nme_get_mouse_position()
{
   int x,y;

   SDL_GetMouseState(&x,&y);

	value pos = empty_object;
   alloc_field( pos, val_id( "x" ), alloc_int( x ) );
   alloc_field( pos, val_id( "y" ), alloc_int( y ) );
   return alloc_object( pos );
}


#define NME_FULLSCREEN 0x0001
#define NME_OPENGL     0x0002
#define NME_RESIZABLE  0x0004
#define NME_HWSURF     0x0008

#ifdef __APPLE__

extern "C" void MacBoot( /*void (*)()*/ );

#endif

value nme_screen_init( value width, value height, value title, value in_flags, value icon )
{
#ifdef __APPLE__
   MacBoot();
#endif

   val_check( in_flags, int );

   int flags = val_int(in_flags);

   bool fullscreen = (flags & NME_FULLSCREEN) != 0;
   bool opengl = (flags & NME_OPENGL) != 0;
   bool resizable = (flags & NME_RESIZABLE) != 0;

   Uint32 init_flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;
   if (opengl)
      init_flags |= SDL_OPENGL;

   sUseSystemHardware = (!opengl) && (val_int(in_flags) & NME_HWSURF);

   if ( SDL_Init( init_flags ) == -1 )
      failure( SDL_GetError() );

   SDL_EnableUNICODE(1);
   SDL_EnableKeyRepeat(500,30);

   val_check( width, int );
   val_check( height, int );
   val_check( title, string );

   int w = val_int(width);
   int h = val_int(height);

   sFlags = SDL_HWSURFACE;

   if ( resizable )
      sFlags |= SDL_RESIZABLE;

   if ( fullscreen )
      sFlags |= SDL_FULLSCREEN;

   if ( val_is_string( icon ) )
   {
      SDL_Surface *icn = nme_loadimage( icon );
      if ( icn != NULL )
         SDL_WM_SetIcon( icn, NULL );
   }

   SDL_Surface* screen = 0;
   if (opengl)
   {
      /* Initialize the display */
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE,  8 );
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8 );
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8 );
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

      sFlags |= SDL_OPENGL;
      if (!(screen = SDL_SetVideoMode( w, h, 32, sFlags | SDL_OPENGL)))
      {
         sFlags &= ~SDL_OPENGL;
         fprintf(stderr, "Couldn't set OpenGL mode: %s\n", SDL_GetError());
      }
      else
        sOpenGL = true;

      sOpenGLScreen = screen;
   }


   if (!screen)
   {
      sFlags |= SDL_DOUBLEBUF;
      screen = SDL_SetVideoMode( w, h, 32, sFlags );
      if (!screen)
         failure( SDL_GetError() );
   }

   if ( TTF_Init() != 0 )
      printf("unable to initialize the truetype font support\n");

   SDL_WM_SetCaption( val_string( title ), 0 );

   if ( Mix_OpenAudio( MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS,4096 )!= 0 )
      printf("unable to initialize the sound support\n");

   gCurrentScreen = screen;
   return alloc_abstract( k_surf, screen );
}

value nme_resize_surface(value inW, value inH)
{
   val_check( inW, int );
   val_check( inH, int );
   int w = val_int(inW);
   int h = val_int(inH);
   SDL_Surface *screen = gCurrentScreen;

   #ifndef __APPLE__
   if (sOpenGL)
   {
      // Little hack to help windows
      screen->w = w;
      screen->h = h;
   }
   else
   #endif
   {
      nme_resize_id ++;
      // Calling this recreates the gl context and we loose all our textures and
      // display lists. So Work around it.
      gCurrentScreen = screen = SDL_SetVideoMode(w, h, 32, sFlags );
   }

   return alloc_abstract( k_surf, screen );
}





value nme_event()
{
	SDL_Event event;
	value evt = empty_object;
	
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
		{
			alloc_field( evt, val_id( "type" ), alloc_int( et_quit ) );
			return alloc_object( evt );
		}
		if (event.type == SDL_KEYDOWN)
		{
			alloc_field( evt, val_id( "type" ), alloc_int( et_keydown ) );
			alloc_field( evt, val_id( "key" ), alloc_int( event.key.keysym.sym ) );
			alloc_field( evt, val_id( "char" ), alloc_int( event.key.keysym.unicode ) );
			alloc_field( evt, val_id( "shift" ), alloc_bool( event.key.keysym.mod & KMOD_SHIFT ) );
			alloc_field( evt, val_id( "ctrl" ), alloc_bool( event.key.keysym.mod & KMOD_CTRL ) );
			alloc_field( evt, val_id( "alt" ), alloc_bool( event.key.keysym.mod & KMOD_ALT ) );
			return alloc_object( evt );
		}
		if (event.type == SDL_KEYUP)
		{
			alloc_field( evt, val_id( "type" ), alloc_int( et_keyup ) );
			alloc_field( evt, val_id( "key" ), alloc_int( event.key.keysym.sym ) );
			alloc_field( evt, val_id( "char" ), alloc_int( event.key.keysym.unicode ) );
			alloc_field( evt, val_id( "shift" ), alloc_bool( event.key.keysym.mod & KMOD_SHIFT ) );
			alloc_field( evt, val_id( "ctrl" ), alloc_bool( event.key.keysym.mod & KMOD_CTRL ) );
			alloc_field( evt, val_id( "alt" ), alloc_bool( event.key.keysym.mod & KMOD_ALT ) );
			return alloc_object( evt );
		}
		if (event.type == SDL_MOUSEMOTION)
		{
			alloc_field( evt, val_id( "type" ), alloc_int( et_motion ) );
			alloc_field( evt, val_id( "state" ), alloc_int( event.motion.state ) );
			alloc_field( evt, val_id( "x" ), alloc_int( event.motion.x ) );
			alloc_field( evt, val_id( "y" ), alloc_int( event.motion.y ) );
			alloc_field( evt, val_id( "xrel" ), alloc_int( event.motion.xrel ) );
			alloc_field( evt, val_id( "yrel" ), alloc_int( event.motion.yrel ) );
			return alloc_object( evt );
		}
		if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
		{
			alloc_field( evt, val_id( "type" ), alloc_int( event.type == SDL_MOUSEBUTTONUP ?
                                                             et_button_up : et_button_down ) );
			alloc_field( evt, val_id( "state" ), alloc_int( event.button.state ) );
			alloc_field( evt, val_id( "x" ), alloc_int( event.button.x ) );
			alloc_field( evt, val_id( "y" ), alloc_int( event.button.y ) );
			alloc_field( evt, val_id( "which" ), alloc_int( event.button.which ) );
			alloc_field( evt, val_id( "button" ), alloc_int( event.button.button ) );
			return alloc_object( evt );
		}
		if (event.type == SDL_JOYAXISMOTION)
		{
			alloc_field( evt, val_id( "type" ), alloc_int( et_jaxis ) );
			alloc_field( evt, val_id( "axis" ), alloc_int( event.jaxis.axis ) );
			alloc_field( evt, val_id( "value" ), alloc_int( event.jaxis.value ) );
			alloc_field( evt, val_id( "which" ), alloc_int( event.jaxis.which ) );
			return alloc_object( evt );
		}
		if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP)
		{
			alloc_field( evt, val_id( "type" ), alloc_int( et_jbutton ) );
			alloc_field( evt, val_id( "button" ), alloc_int( event.jbutton.button ) );
			alloc_field( evt, val_id( "state" ), alloc_int( event.jbutton.state ) );
			alloc_field( evt, val_id( "which" ), alloc_int( event.jbutton.which ) );
			return alloc_object( evt );
		}
		if (event.type == SDL_JOYHATMOTION)
		{
			alloc_field( evt, val_id( "type" ), alloc_int( et_jhat ) );
			alloc_field( evt, val_id( "button" ), alloc_int( event.jhat.hat ) );
			alloc_field( evt, val_id( "value" ), alloc_int( event.jhat.value ) );
			alloc_field( evt, val_id( "which" ), alloc_int( event.jhat.which ) );
			return alloc_object( evt );
		}
		if (event.type == SDL_JOYBALLMOTION)
		{
			alloc_field( evt, val_id( "type" ), alloc_int( et_jball ) );
			alloc_field( evt, val_id( "ball" ), alloc_int( event.jball.ball ) );
			alloc_field( evt, val_id( "xrel" ), alloc_int( event.jball.xrel ) );
			alloc_field( evt, val_id( "yrel" ), alloc_int( event.jball.yrel ) );
			alloc_field( evt, val_id( "which" ), alloc_int( event.jball.which ) );
			return alloc_object( evt );
		}

      if (event.type==SDL_VIDEORESIZE)
      {
			alloc_field( evt, val_id( "type" ), alloc_int( et_resize ) );
			alloc_field( evt, val_id( "width" ), alloc_int( event.resize.w ) );
			alloc_field( evt, val_id( "height" ), alloc_int( event.resize.h ) );
			return alloc_object( evt );
      }
	}
	alloc_field( evt, val_id( "type" ), alloc_int( et_noevent ) );
	return evt;
}

value nme_screen_close()
{
	Mix_CloseAudio();
	//sge_TTF_Quit();
	SDL_Quit();
	return alloc_int( 0 );
}

static void init_scrap_once()
{
   static bool init=false;
   if (!init)
   {
      init_scrap();
      init = true;
   }
}

value nme_get_clipboard()
{
   init_scrap_once();

   char *data = 0;
   int len = 0;
   get_scrap( TYPE('T','E','X','T'), &len, &data );
   if (len==0 || data==0)
      data = "";

   value result = empty_object;
   alloc_field( result, val_id("__s"), alloc_string(data));
   alloc_field( result, val_id("length"), alloc_int((int)strlen(data)));

   return alloc_string(data);
   return result;
}

value nme_set_clipboard(value inVal)
{
   val_check(inVal,string);

   init_scrap_once();

   const char *str = val_string(inVal);
   put_scrap( TYPE('T','E','X','T'), (int)strlen(str), (char *)str );
   return alloc_int(0);
}


DEFINE_PRIM(nme_event, 0);
DEFINE_PRIM(nme_delay, 1);
DEFINE_PRIM(nme_flipbuffer, 1);

DEFINE_PRIM(nme_get_clipboard,0);
DEFINE_PRIM(nme_set_clipboard, 1);

DEFINE_PRIM(nme_create_image_32,3);
DEFINE_PRIM(nme_copy_surface,1);

DEFINE_PRIM(nme_screen_init, 5);
DEFINE_PRIM(nme_resize_surface, 2);
DEFINE_PRIM(nme_screen_close, 0);

DEFINE_PRIM(nme_surface_clear, 2);
DEFINE_PRIM(nme_surface_free, 1);
DEFINE_PRIM(nme_surface_width, 1);
DEFINE_PRIM(nme_surface_height, 1);
DEFINE_PRIM(nme_surface_colourkey, 4);
DEFINE_PRIM(nme_set_clip_rect, 2);
DEFINE_PRIM(nme_get_clip_rect, 1);
DEFINE_PRIM(nme_set_cursor, 1);
DEFINE_PRIM(nme_get_mouse_position, 0);
