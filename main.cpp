#include <iostream>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <objLoader.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include "stb_image.h"



#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Uint32 rmask = 0xff000000;
    Uint32 gmask = 0x00ff0000;
    Uint32 bmask = 0x0000ff00;
    Uint32 amask = 0x000000ff;
#else
    Uint32 rmask = 0x000000ff;
    Uint32 gmask = 0x0000ff00;
    Uint32 bmask = 0x00ff0000;
    Uint32 amask = 0xff000000;
#endif


TTF_Font *font;

using namespace std;
objLoader *objData;
Uint32 globalTime = 0;
float frame = 0;
Uint32 timeMile = 0;
float framesToPrint = 0.0;
GLuint texture;

vector<char> getDigits(float fps) {
    int f = fps;
    vector<char> vec;
    stringstream str;
    char c;
    while (f != 0) {
        str << (f % 10);
        str.get(c);
        vec.push_back(c);
        f /= 10;
    }
    reverse(vec.begin(), vec.end());
    return vec;
}

void render_text() {

    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 500, 0.0, 500);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    GLuint text_texture;

    SDL_Surface *i_surface, *o_surface, *on_surface;

    SDL_PixelFormat *format;
    SDL_Color color={0,255,0};
    if (i_surface=TTF_RenderText_Blended(font,"Hello World!",color)) {
        o_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 128, 128, 32,
                                         rmask, gmask, bmask, amask);
        SDL_BlitSurface(i_surface, 0, o_surface, 0);
        glGenTextures(1, &text_texture);
        glBindTexture( GL_TEXTURE_2D, text_texture );
        glTexImage2D(GL_TEXTURE_2D, 0, 4, o_surface->w, o_surface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, o_surface->pixels);
// when texture area is small, bilinear filter the closest mipmap
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                         GL_LINEAR );
// when texture area is large, bilinear filter the original
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        // 	vector<char> digits = getDigits(framesToPrint);
// 	for(int i = 0; i < digits.size(); i++) {
// 		glRasterPos2i(i * 10 , 500 - 13);
// 		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, digits[i]);
// 	}
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(0.0f,0,-1);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(0.0f, 40, -1);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(100.0f, 40, -1);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(100.0f, 0.0f, -1);
        glEnd();
        glDeleteTextures(1, &text_texture);
        SDL_FreeSurface(i_surface);
        SDL_FreeSurface(o_surface);
        //SDL_FreeSurface(on_surface);

    } else
        cerr << "ne";
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

}

void load_font() {
    font = TTF_OpenFont("Ubuntu-R.ttf", 16);
    if (!font) {
        cerr << "TTF_OpenFont: \n" << TTF_GetError();
    }
}

void loadTexture() {
//load image

    int x,y,n;
    unsigned char *data = stbi_load("test.png", &x, &y, &n, 0);
    // ... process data if not NULL ...
    // ... x = width, y = height, n = # 8-bit components per pixel ...
    // ... replace '0' with '1'..'4' to force that many components per pixel



//gen texture

    glBindTexture( GL_TEXTURE_2D, texture );
    glTexImage2D(GL_TEXTURE_2D, 0, 3, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
// when texture area is small, bilinear filter the closest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                     GL_LINEAR );
// when texture area is large, bilinear filter the original
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    stbi_image_free(data);
}


void renderImage() {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 500, 0.0, 500);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();



//bind image to texture

//draw quad
    glColor3f(0.5, 0.6, 0.8);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindTexture( GL_TEXTURE_2D, texture );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0.0f,0,-1);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(0.0f, 255, -1);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(255.0f, 255, -1);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(255.0f, 0.0f, -1);


    glEnd();



    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}




void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //(Left, Right, Bottom, Top, Near, Far);
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 100.0);
    glMatrixMode(GL_MODELVIEW);
}




static void quit( int code )
{
    /*
     * Quit SDL so we can release the fullscreen
     * mode and restore the previous video settings,
     * etc.
     */
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    delete(objData);
    glDeleteTextures( 1,&texture );

    /* Exit program. */
    exit( code );
}



static void handle_key_down( SDL_keysym* keysym )
{

    /*
     * We're only interested if 'Esc' has
     * been presssed.
     *
     * EXERCISE:
     * Handle the arrow keys and have that change the
     * viewing position/angle.
     */
    switch ( keysym->sym ) {
    case SDLK_ESCAPE:
        quit( 0 );
        break;
    default:
        break;
    }

}


static void process_events( void )
{
    /* Our SDL event placeholder. */
    SDL_Event event;

    /* Grab all the events off the queue. */
    while ( SDL_PollEvent( &event ) ) {

        switch ( event.type ) {
        case SDL_KEYDOWN:
            /* Handle key presses. */
            handle_key_down( &event.key.keysym );
            break;
        case SDL_QUIT:
            /* Handle quit requests (like Ctrl-c). */
            quit( 0 );
            break;
        }

    }

}


static void draw_screen( void )
{

    globalTime = SDL_GetTicks();
    if (globalTime - timeMile > 100) {
        framesToPrint = frame/(globalTime - timeMile)*1000;
        timeMile = globalTime;
        frame = 0;
    }


    /* Clear the color and depth buffers. */
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    frame++;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //(Left, Right, Bottom, Top, Near, Far);
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 100.0);
    glMatrixMode(GL_MODELVIEW);

    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);


    glLoadIdentity();
    gluLookAt(0.0, 0.0, 50.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glTranslatef(timeMile/50 - 10,0,0);
// 	glutWireCube(2.0);
    glCallList(1);
    render_text();
    //renderImage();
    glFlush();

    /*
     * EXERCISE:
     * Draw text telling the user that 'Spc'
     * pauses the rotation and 'Esc' quits.
     * Do it using vetors and textured quads.
     */

    /*
     * Swap the buffers. This this tells the driver to
     * render the next frame from the contents of the
     * back-buffer, and to set all rendering operations
     * to occur on what was the front-buffer.
     *
     * Double buffering prevents nasty visual tearing
     * from the application drawing on areas of the
     * screen that are being updated at the same time.
     */
    SDL_GL_SwapBuffers( );
}



static void setup_opengl( int width, int height )
{


    glEnable( GL_TEXTURE_2D );


// allocate a texture name
    glGenTextures( 1, &texture );
    objData = new objLoader();
    objData->load("test.obj");
    cout << objData->faceCount;
    timeMile = 0.0;

    loadTexture();
    glNewList(1, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < objData->faceCount; i++) {
        for (int ii = 0; ii < 3; ii++) {
            obj_vector *v = objData->vertexList[objData->faceList[i]->vertex_index[ii]];
            glVertex3f((GLfloat) v->e[0],(GLfloat) v->e[1],(GLfloat) v->e[2]);
        }
    }

    glEnd();
    glEndList();

    glClearColor(0.0, 0.0, 0.0, 0.0);

    float ratio = (float) width / (float) height;

    /* Our shading model--Gouraud (smooth). */
    glShadeModel( GL_SMOOTH );

    /* Set the clear color. */
    glClearColor( 0, 0, 0, 0 );


    /*
     * Change to the projection matrix and set
     * our viewing volume.
     */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    /*
     * EXERCISE:
     * Replace this with a call to glFrustum.
     */
    gluPerspective( 60.0, ratio, 1.0, 1024.0 );
}



int main( int argc, char* argv[] )
{
    /* Information about the current video settings. */
    const SDL_VideoInfo* info = NULL;
    /* Dimensions of our window. */
    int width = 0;
    int height = 0;
    /* Color depth in bits of our window. */
    int bpp = 0;
    /* Flags we will pass into SDL_SetVideoMode. */
    int flags = 0;

    /* First, initialize SDL's video subsystem. */
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        /* Failed, exit. */
        fprintf( stderr, "Video initialization failed: %s\n",
                 SDL_GetError( ) );
        quit( 1 );
    }


    if (TTF_Init()==-1) {
        cerr << "TTF_Init: \n" << TTF_GetError();
        SDL_Quit( );
        exit(1);
    }
    load_font();


    /* Let's get some video information. */
    info = SDL_GetVideoInfo( );

    if ( !info ) {
        /* This should probably never happen. */
        fprintf( stderr, "Video query failed: %s\n",
                 SDL_GetError( ) );
        quit( 1 );
    }

    /*
     * Set our width/height to 640/480 (you would
     * of course let the user decide this in a normal
     * app). We get the bpp we will request from
     * the display. On X11, VidMode can't change
     * resolution, so this is probably being overly
     * safe. Under Win32, ChangeDisplaySettings
     * can change the bpp.
     */
    width = 500;
    height = 500;
    bpp = info->vfmt->BitsPerPixel;

    /*
     * Now, we want to setup our requested
     * window attributes for our OpenGL window.
     * We want *at least* 5 bits of red, green
     * and blue. We also want at least a 16-bit
     * depth buffer.
     *
     * The last thing we do is request a double
     * buffered window. '1' turns on double
     * buffering, '0' turns it off.
     *
     * Note that we do not use SDL_DOUBLEBUF in
     * the flags to SDL_SetVideoMode. That does
     * not affect the GL attribute state, only
     * the standard 2D blitting setup.
     */
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    /*
     * We want to request that SDL provide us
     * with an OpenGL window, in a fullscreen
     * video mode.
     *
     * EXERCISE:
     * Make starting windowed an option, and
     * handle the resize events properly with
     * glViewport.
     */
    flags = SDL_OPENGL;

    /*
     * Set the video mode
     */
    if ( SDL_SetVideoMode( width, height, bpp, flags ) == 0 ) {
        /*
         * This could happen for a variety of reasons,
         * including DISPLAY not being set, the specified
         * resolution not being available, etc.
         */
        fprintf( stderr, "Video mode set failed: %s\n",
                 SDL_GetError( ) );
        quit( 1 );
    }

    /*
     * At this point, we should have a properly setup
     * double-buffered window for use with OpenGL.
     */
    setup_opengl( width, height );

    /*
     * Now we want to begin our normal app process--
     * an event loop with a lot of redrawing.
     */
    while ( 1 ) {
        /* Process incoming events. */
        process_events( );
        /* Draw the screen. */
        draw_screen( );
    }

    /* Never reached. */
    return 0;
}
