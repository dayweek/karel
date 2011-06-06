#include <iostream>
#include <math.h>
#include <SDL/SDL.h>
#ifdef CROSS
#include <SDL_ttf.h>
#else
#include <SDL/SDL_ttf.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <objLoader.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include "stb_image.h"

int next_power ( int x ) {
    return pow ( 2, ceil ( log ( x ) /log ( 2 ) ) );
}

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
int countp = 0;
using namespace std;
objLoader *objData;
Uint32 globalTime = 0;
float frame = 0;
Uint32 timeMile = 0;
float framesToPrint = 0.0;
GLuint texture = 0;
GLuint crate_texture = 0;

const int size = 20;
typedef pair<int, int> pos;
pos closest;

class Node {
public:
	Node() {
		free = true;
		dist = 0;
		dest = false;
		visited = false;
	}
	bool free;
	bool visited;
	int dist;
	bool dest;
} n ;
vector< vector <Node> > field(size, vector<Node>(size,n));
bool is_in_field(int x, int y) {
	if(x > -1 && x < size && 
		y > -1 && y < size)
		return true;
	return false;
}
void draw_crate();
class Robot {
public:
	pos start_pos;
	Uint32 start_time;
	vector<pos> path;
	bool has_crate;
	void render() {
		float vzdalenost = (globalTime - start_time) / 1000.0;
		int policka = vzdalenost;
		if(policka > path.size())
			policka = path.size();
		float x = start_pos.first, y = start_pos.second;
		for(int i = 0; i < policka; i++) {
			x += path[i].first;
			y += path[i].second;
		}
		if(policka < path.size()) {
			x += path[policka].first * (vzdalenost - floor(vzdalenost));
			y += path[policka].second * (vzdalenost - floor(vzdalenost));
		}
		glMatrixMode ( GL_MODELVIEW );
		glPushMatrix();
		glTranslatef(x + 0.5, 0, y + 0.5);
		draw_crate();
		glPopMatrix();
	}
};

void get_path(int x, int y, int i) {
	if(!field[x][y].visited || field[x][y].dist > i) {
		field[x][y].dist = i;
		field[x][y].visited = true;
	}
	else return;
	if(field[x][y].dest) {
		closest = make_pair<int, int>(x,y);
		return;
	}
	if(is_in_field(x + 1, y) && field[x + 1][y].free)
		get_path(x + 1, y, i + 1);
	if(is_in_field(x, y + 1) && field[x][y + 1].free)
		get_path(x, y + 1, i + 1);
	if(is_in_field(x, y - 1) && field[x][y - 1].free)
		get_path(x, y -1, i + 1);
	if(is_in_field(x - 1, y) && field[x - 1][y].free)
		get_path(x -1, y, i + 1);
}
void reset_field_for_find() {
		for(int i;i < size;i++)
			for(int ii;ii < size;ii++) {
				field[i][ii].visited = false;
				field[i][ii].dist = -1;
				field[i][ii].dest = false;
			}
				
}

void backtrack(int x, int y , vector<pair<int, int> >& p) {
	p.push_back(make_pair<int, int>(x,y));
	int i = field[x][y].dist - 1;
	if(i == -1)
		return;
	if(is_in_field(x + 1, y) && field[x + 1][y].dist  == i &&  field[x+1][y].visited)
		backtrack(x + 1, y, p);
	else if(is_in_field(x, y + 1) && field[x][y + 1].dist  == i &&  field[x][y + 1].visited)
		backtrack(x, y + 1, p);
	else if(is_in_field(x, y - 1) && field[x][y - 1].dist  == i && field[x][y - 1].visited)
		backtrack(x, y -1,p);
	else if(is_in_field(x - 1, y) && field[x - 1][y].dist  == i && field[x - 1][y].visited)
		backtrack(x -1, y,p);
}

 vector<pair<int, int> > find_path(int x1,int y1, int x2, int y2) {
	closest = make_pair<int, int>(-1,-1);
	reset_field_for_find();
	field[x2][y2].dest = true;
	get_path(x1,y1,0);
	vector<pair<int, int> > p(0);
	if(closest.first == -1)
		return p;
	else {
		backtrack(closest.first, closest.second, p);
		vector<pair<int, int> > pp(0);
		for(int i = 0;i < p.size() - 1;i++)
			pp.push_back(make_pair<int,int>(p[i+1].first - p[i].first, p[i+1].second - p[i].second));
		return pp;
	}

}

Robot robot;

string getDigits ( float fps ) {
    int f = fps;
    string digits;
    stringstream str;
    char c;
    while ( f != 0 ) {
        str << ( f % 10 );
        str.get ( c );
        digits += c;
        f /= 10;
    }
    reverse ( digits.begin(), digits.end() );
    return digits;
}



void render_text ( const string text ) {

    glDisable ( GL_DEPTH_TEST );
    glMatrixMode ( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D ( 0.0, 500, 0.0, 500 );

    glMatrixMode ( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    GLuint text_texture;

    SDL_Surface *i_surface, *o_surface, *on_surface;

    SDL_PixelFormat *format;
    SDL_Color color={0,255,0};
    if ( i_surface=TTF_RenderUTF8_Blended ( font,text.c_str(),color ) ) {
        int w = next_power ( i_surface->w );
        int h = next_power ( i_surface->h );
        o_surface = SDL_CreateRGBSurface ( SDL_SWSURFACE, w, h, 32,
                                           rmask, gmask, bmask, amask );
        SDL_BlitSurface ( i_surface, 0, o_surface, 0 );
        for ( int i = 0; i < w; i++ )
            for ( int ii = 0; ii < h; ii++ )
                if ( ( ( Uint32* ) o_surface->pixels ) [i*w+ii] == 0x00000000 )
                    ( ( Uint32* ) o_surface->pixels ) [i*w+ii] = ( ( Uint32* ) o_surface->pixels ) [i*w+ii] | 0xff000000;
        glGenTextures ( 1, &text_texture );
        glBindTexture ( GL_TEXTURE_2D, text_texture );
        glTexImage2D ( GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, o_surface->pixels );
// when texture area is small, bilinear filter the closest mipmap
        glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                          GL_LINEAR );
// when texture area is large, bilinear filter the original
        glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        // 	vector<char> digits = getDigits(framesToPrint);
// 	for(int i = 0; i < digits.size(); i++) {
// 		glRasterPos2i(i * 10 , 500 - 13);
// 		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, digits[i]);
// 	}
        glEnable ( GL_BLEND );
        glBlendFunc ( GL_SRC_ALPHA, GL_ZERO );
        glBegin ( GL_QUADS );
        glTexCoord2f ( 0.0f, 1.0f );
        glVertex3f ( 0.0f,0,-1 );

        glTexCoord2f ( 0.0f, 0.0f );
        glVertex3f ( 0.0f, h, -1 );

        glTexCoord2f ( 1.0f, 0.0f );
        glVertex3f ( w, h, -1 );
        glTexCoord2f ( 1.0f, 1.0f );
        glVertex3f ( w, 0.0f, -1 );
        glEnd();
        glDisable ( GL_BLEND );
        glDeleteTextures ( 1, &text_texture );
        SDL_FreeSurface ( i_surface );
        SDL_FreeSurface ( o_surface );
        //SDL_FreeSurface(on_surface);

    } else
        cerr << "cannot render font";
    glMatrixMode ( GL_MODELVIEW );
    glPopMatrix();

    glMatrixMode ( GL_PROJECTION );
    glPopMatrix();

}

void load_font() {
    font = TTF_OpenFont ( "Ubuntu-R.ttf", 16 );
    if ( !font ) {
        cerr << "TTF_OpenFont: \n" << TTF_GetError();
    }
}

//TODO mipmap
void load_texture(string filename, GLuint* texture) {

    int x,y,n;
    GLint tmp;

    unsigned char *data = stbi_load ( filename.c_str(), &x, &y, &n, 0 );
    if (!data)
        cerr << "cannot load " << filename << endl;
    else {
        glGenTextures ( 1, texture );
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &tmp);
        // ... process data if not NULL ...
        // ... x = width, y = height, n = # 8-bit components per pixel ...
        // ... replace '0' with '1'..'4' to force that many components per pixel



//gen texture

        glBindTexture ( GL_TEXTURE_2D, *texture );
        glTexImage2D ( GL_TEXTURE_2D, 0, 3, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
// when texture area is small, bilinear filter the closest mipmap
        glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                          GL_LINEAR );
// when texture area is large, bilinear filter the original
        glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        stbi_image_free ( data );
        glBindTexture ( GL_TEXTURE_2D, tmp);
    }
}

void load_textures() {

    load_texture("crate.png", &crate_texture);
    load_texture(string("test.png"), &texture);

}

void renderImage() {
    glDisable ( GL_DEPTH_TEST );
    glMatrixMode ( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D ( 0.0, 500, 0.0, 500 );

    glMatrixMode ( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();



//bind image to texture

//draw quad
    glColor3f ( 0.5, 0.6, 0.8 );
    glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL );
    glBindTexture ( GL_TEXTURE_2D, texture );
    glBegin ( GL_QUADS );
    glTexCoord2f ( 0.0f, 0.0f );
    glVertex3f ( 0.0f,0,-1 );
    glTexCoord2f ( 0.0f, 1.0f );
    glVertex3f ( 0.0f, 255, -1 );
    glTexCoord2f ( 1.0f, 1.0f );
    glVertex3f ( 255.0f, 255, -1 );
    glTexCoord2f ( 1.0f, 0.0f );
    glVertex3f ( 255.0f, 0.0f, -1 );


    glEnd();



    glMatrixMode ( GL_MODELVIEW );
    glPopMatrix();

    glMatrixMode ( GL_PROJECTION );
    glPopMatrix();
}




void reshape ( int w, int h ) {
    glViewport ( 0, 0, ( GLsizei ) w, ( GLsizei ) h );
    glMatrixMode ( GL_PROJECTION );
    glLoadIdentity();
    //(Left, Right, Bottom, Top, Near, Far);
    glFrustum ( -1.0, 1.0, -1.0, 1.0, 1.5, 100.0 );
    glMatrixMode ( GL_MODELVIEW );
}




static void quit ( int code ) {
    /*
     * Quit SDL so we can release the fullscreen
     * mode and restore the previous video settings,
     * etc.
     */
    if ( TTF_WasInit() ) {
        if (font)
            TTF_CloseFont ( font );
        TTF_Quit();
    }

    delete ( objData );
    if (texture)
        glDeleteTextures ( 1,&texture );
    if (crate_texture)
        glDeleteTextures ( 1,&crate_texture );
    /* Exit program. */
    SDL_Quit();
    exit ( code );
}



static void handle_key_down ( SDL_keysym* keysym ) {

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
        quit ( 0 );
        break;
    default:
        break;
    }

}


static void process_events ( void ) {
    /* Our SDL event placeholder. */
    SDL_Event event;

    /* Grab all the events off the queue. */
    while ( SDL_PollEvent ( &event ) ) {

        switch ( event.type ) {
        case SDL_KEYDOWN:
            /* Handle key presses. */
            handle_key_down ( &event.key.keysym );
            break;
        case SDL_QUIT:
            /* Handle quit requests (like Ctrl-c). */
            quit ( 0 );
            break;
        }

    }

}

void draw_crate() {
    glBindTexture(GL_TEXTURE_2D, crate_texture);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBegin ( GL_QUADS);
    glNormal3f(0,0,1);
    // Front Face
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f( 0.5f, -0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f( 0.5f,  0.5f,  0.5f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-0.5f,  0.5f,  0.5f);	// Top Left Of The Texture and Quad
// 		Back Face
    glNormal3f(0,0,-1);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-0.5f,  0.5f, -0.5f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f( 0.5f,  0.5f, -0.5f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f( 0.5f, -0.5f, -0.5f);	// Bottom Left Of The Texture and Quad
    // Top Face
    //TODO weird normal
    glNormal3f(0,-1,0);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-0.5f,  0.5f, -0.5f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-0.5f,  0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f( 0.5f,  0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f( 0.5f,  0.5f, -0.5f);	// Top Right Of The Texture and Quad
    // Bottom Face
    glNormal3f(0,1,0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f( 0.5f, -0.5f, -0.5f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f( 0.5f, -0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
    // Right face
    glNormal3f(1,0,0);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f( 0.5f, -0.5f, -0.5f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f( 0.5f,  0.5f, -0.5f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f( 0.5f,  0.5f,  0.5f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f( 0.5f, -0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
    // Left Face
    glNormal3f(-1,0,0);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-0.5f,  0.5f,  0.5f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-0.5f,  0.5f, -0.5f);	// Top Left Of The Texture and Quad
    glEnd();

}

static void draw_screen ( void ) {

    globalTime = SDL_GetTicks();
    if ( globalTime - timeMile > 100 ) {
        framesToPrint = frame/ ( globalTime - timeMile ) *1000;
        timeMile = globalTime;
        countp++;
        frame = 0;
    }
    frame++;

    /* Clear the color and depth buffers. */
    glClearColor(0.0,0.0,0.0,0.0);
    glClearDepth(1);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    glMatrixMode ( GL_PROJECTION );
    glLoadIdentity();
    //(Left, Right, Bottom, Top, Near, Far);
    glFrustum ( -1.0, 1.0, -1.0, 1.0, 1.5, 100.0 );
    glMatrixMode ( GL_MODELVIEW );
    glColor3f ( 1.0, 1.0, 1.0 );
    glLoadIdentity();
    gluLookAt ( 10, 10,10, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 );
    //glTranslatef ( timeMile/250.0 - 10,0,0 );
// 	glutWireCube(2.0);
    //glCallList ( 1 );



	robot.render();
    if ( countp == 10 ) {
        cout << framesToPrint << " " << "FPS" << endl;
        countp = 0;
    }
    //renderImage();
    //render_text ( getDigits ( framesToPrint ) );

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

void setup_texturing() {
    glEnable ( GL_TEXTURE_2D );
    glGenTextures ( 1, &texture );

}
void setup_shading() {
    glShadeModel ( GL_SMOOTH );
}
void setup_lighting() {

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat light_position[] = {0,0,10,1.0};//w=0:infinite
    GLfloat light_diffuse[] = {1,1,1,1};
    GLfloat light_ambient[] = {0.1,0.1,0.1,1};
// set the lights
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);


}

void setup_matrices(int width, int height) {
    float ratio = ( float ) width / ( float ) height;
    /*
     * Change to the projection matrix and set
     * our viewing volume.
     */
    glMatrixMode ( GL_PROJECTION );
    glLoadIdentity( );
    /*
     * EXERCISE:
     * Replace this with a call to glFrustum.
     */
    gluPerspective ( 60.0, ratio, 1.0, 1024.0 );
}

void setup_opengl ( int width, int height ) {
    timeMile = 0.0;
    glEnable(GL_DEPTH_TEST);
// 	glDepthFunc(GL_LEQUAL);
    glClearColor(0.0,0.0,0.0,0.0);
    glClearDepth(1);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    setup_texturing();
    setup_shading();
    setup_lighting();
    setup_matrices(width, height);

}

void load_models() {
    objData = new objLoader();
    objData->load ("test.obj");
    cout << objData->faceCount;



    glNewList ( 1, GL_COMPILE );
    glBegin ( GL_TRIANGLES );
    for ( int i = 0; i < objData->faceCount; i++ ) {
        for ( int ii = 0; ii < 3; ii++ ) {
            obj_vector *v = objData->vertexList[objData->faceList[i]->vertex_index[ii]];
            glVertex3f ( ( GLfloat ) v->e[0], ( GLfloat ) v->e[1], ( GLfloat ) v->e[2] );
        }
    }
    glEnd();
    glEndList();
    delete objData;
}


int main ( int argc, char* argv[] ) {
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
    if ( SDL_Init ( SDL_INIT_VIDEO ) < 0 ) {
        /* Failed, exit. */
        fprintf ( stderr, "Video initialization failed: %s\n",
                  SDL_GetError( ) );
        quit ( 1 );
    }


    if ( TTF_Init() ==-1 ) {
        cerr << "TTF_Init: \n" << TTF_GetError();
        quit(1);
    }



    /* Let's get some video information. */
    info = SDL_GetVideoInfo( );

    if ( !info ) {
        /* This should probably never happen. */
        fprintf ( stderr, "Video query failed: %s\n",
                  SDL_GetError( ) );
        quit ( 1 );
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
    SDL_GL_SetAttribute ( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute ( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute ( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute ( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute ( SDL_GL_DOUBLEBUFFER, 1 );

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
    if ( SDL_SetVideoMode ( width, height, bpp, flags ) == 0 ) {
        /*
         * This could happen for a variety of reasons,
         * including DISPLAY not being set, the specified
         * resolution not being available, etc.
         */
        fprintf ( stderr, "Video mode set failed: %s\n",
                  SDL_GetError( ) );
        quit ( 1 );
    }

    /*
     * At this point, we should have a properly setup
     * double-buffered window for use with OpenGL.
     */
    setup_opengl ( width, height );
    load_font();
    load_textures();
	robot.start_pos = make_pair<int, int>(0,0);
	robot.start_time = globalTime;
	field[3][0].free = false;
	robot.path = find_path(0,0,5,0);
	
    //load_models();

    /*
     * Now we want to begin our normal app process--
     * an event loop with a lot of redrawing.
     */
    while ( 1 ) {
        /* Process incoming events. */
        process_events( );
        /* Draw the screen. */
        draw_screen( );
        SDL_Delay(1);

    }

    /* Never reached. */
    return 0;
}
