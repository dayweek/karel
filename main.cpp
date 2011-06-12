#include <iostream>
#include <math.h>
#include <GL/glew.h>
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
#include <fstream>
#include <algorithm>
#include "stb_image.h"
#ifdef CROSS
#include "time.h"

#endif
using namespace std;
int next_power ( int x ) {
    return pow ( 2, ceil ( log ( x ) /log ( 2 ) ) );
}

void draw_crate();
void draw_podvozek();
void draw_tyc(float vyska);


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
const int list_count = 20;
GLuint lists, start_list;

objLoader *objData;
Uint32 globalTime = 0;
float frame = 0;
Uint32 timeMile = 0;
float framesToPrint = 0.0;
GLuint crate_texture = 0;

const int size = 20;
typedef pair<int, int> pos;
pos closest;
vector<pair<int, int> > orders;
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
class Model {
public:

	int vertex_index;
	int vertex_count;
	GLuint triangle_list, quad_list;
	
	GLuint texture;

	Model() {}
    Model(string filename) {
        objData = new objLoader();

		
        objData->load (const_cast<char*>(filename.c_str()));

		if(objData->materialCount > 0) {
			string s(objData->materialList[0]->texture_filename);
			s.erase(s.end()-1);
			load_texture(s.c_str(),&texture);
		}
		quad_list = lists;
		glNewList(lists++, GL_COMPILE);
		glBegin(GL_QUADS);
        for ( int i = 0; i < objData->faceCount; i++ ) {
            for ( int ii = 0; ii < objData->faceList[i]->vertex_count; ii++ ) {
                obj_vector *v = objData->vertexList[objData->faceList[i]->vertex_index[ii]];
				//if no texture?
				;
				obj_vector *n = objData->normalList[objData->faceList[i]->normal_index[ii]];
				if(objData->faceList[i]->vertex_count == 4) {
					glNormal3f(n->e[0], n->e[1], n->e[2]);
					if(objData->faceList[i]->texture_index[ii] != -1) {
						obj_vector *t = objData->textureList[objData->faceList[i]->texture_index[ii]];
						glTexCoord2f(t->e[0], t->e[1]);
					}
					glVertex3f ( ( GLfloat ) v->e[0], ( GLfloat ) v->e[1], ( GLfloat ) v->e[2] );
				}
            }
        }
        glEnd();
		glEndList();

		triangle_list = lists;
		glNewList(lists++, GL_COMPILE);
		glBegin(GL_TRIANGLES);
        for ( int i = 0; i < objData->faceCount; i++ ) {
            for ( int ii = 0; ii < objData->faceList[i]->vertex_count; ii++ ) {
                obj_vector *v = objData->vertexList[objData->faceList[i]->vertex_index[ii]];
				obj_vector *t = objData->textureList[objData->faceList[i]->texture_index[ii]];
				obj_vector *n = objData->normalList[objData->faceList[i]->normal_index[ii]];
				if(objData->faceList[i]->vertex_count == 3) {
					glNormal3f(n->e[0], n->e[1], n->e[2]);
					glTexCoord2f(t->e[0], t->e[1]);
					glVertex3f ( ( GLfloat ) v->e[0], ( GLfloat ) v->e[1], ( GLfloat ) v->e[2] );
				}
            }
        }
        glEnd();
		glEndList();
        delete objData;
        objData = 0;
	}
	void render() {
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glCallList(quad_list);
		glCallList(triangle_list);
	}
	~Model() {
// 		glDeleteTextures(1, &texture);
	}
};
const double PI = 3.141592;
float slow_down(float time) {
	if(time < 0.5)
		return 2*time*time;
	else
		return sin((time - 0.5) * PI) / 2.0 + 0.5;
}
class Hand {
public:
	vector<Model> parts;
	vector<float> alphas, alphas1, alphas2;
	bool has_crate;
	float start_time;
	float duration;
	Hand(){
		parts.push_back(Model("hand01.obj"));
		parts.push_back(Model("hand02.obj"));
		parts.push_back(Model("hand03.obj"));
		parts.push_back(Model("hand04.obj"));
		parts.push_back(Model("hand05.obj"));
		alphas = vector<GLfloat>(5, 0.0);
		alphas2 = vector<GLfloat>(5, 0.0);
		alphas1 = vector<GLfloat>(5, 0.0);
		alphas[0] = 0;
		alphas[1] = -10;
		alphas[2] = 35.0651 ;
		alphas[3] = -25.0651;
		alphas[4] = 0;
		alphas1 = alphas;
		
		alphas2[0] = 270;
		alphas2[1] = -80;
		alphas2[2] = 50 ;
		alphas2[3] = 120;
		alphas2[4] = 180;
		start_time = 0;
		duration = 3000;
		has_crate = false;
		
	}
	void calculate_angles() {
		float rozdil = globalTime - start_time;
		if(rozdil > duration) {
			start_time += duration;
			if(has_crate)
				alphas = alphas1;
			else
				alphas = alphas2;
			has_crate = !has_crate;
		} else {
			float ujdene = rozdil / duration;
			ujdene = slow_down(ujdene);
			if(has_crate) 
				ujdene = 1.0 - ujdene;
			for(int i = 0; i < 5;i++)
				alphas[i] = ujdene * (alphas2[i] - alphas1[i]) + alphas1[i];
		}
	}
	void render() {
		calculate_angles();
		glPushMatrix();
		glTranslatef(0.5,0,-6.921904);
		glRotatef(alphas[0], 0, 1, 0);
		parts[0].render();
		
		glTranslatef(0,1,0);
		glRotatef(alphas[1],1,0,0);
		parts[1].render();
		
		glTranslatef(0,0,3);
		glRotatef(alphas[2],1,0,0);
		parts[1].render();
		
		glTranslatef(0,0,3);
		glRotatef(alphas[3],1,0,0);
		parts[2].render();
		
		glTranslatef(0,0.25,1);
		glRotatef(alphas[4],0,0,1);
		parts[3].render();
		
		glTranslatef(0,0,0.25);
		glPushMatrix();
		glTranslatef(-0.5,0,0);
		parts[4].render();
		glPopMatrix();
		glPushMatrix();
		glTranslatef(0.6,0,0);
		parts[4].render();
		glPopMatrix();
		glTranslatef(0,0,0.5);
		if(has_crate)
			draw_crate();
		glPopMatrix();

	}
};
Hand* hand = 0;

class Node {
public:
    Node() {
        free = true;
        dist = 0;
        dest = false;
        visited = false;
        crates_to_build = 0;
        crates = 0;
    }
    bool free;
    bool visited;
    int dist;
    bool dest;
    int crates_to_build, crates;
} n ;

struct Crate {
    int x,y,level;
    pair<int,int> otoceni;
};
vector<Crate> crates(0);
vector< vector <Node> > field(size, vector<Node>(size,n));
bool is_in_field(int x, int y) {
    if (x > -1 && x < size &&
            y > -1 && y < size)
        return true;
    return false;
}


void get_path(int x, int y, int i) {
	if(!field[x][y].free && !field[x][y].dest)
		return;
    if (!field[x][y].visited || field[x][y].dist > i) {
        field[x][y].dist = i;
        field[x][y].visited = true;
    }
    else return;
    if (field[x][y].dest) {
        if (closest.first == -1)
            closest = make_pair<int, int>(x,y);
        else if (field[x][y].dist < field[closest.first][closest.second].dist)
            closest = make_pair<int, int>(x,y);
        return;
    }
    if (is_in_field(x + 1, y))
        get_path(x + 1, y, i + 1);
    if (is_in_field(x, y + 1))
        get_path(x, y + 1, i + 1);
    if (is_in_field(x, y - 1))
        get_path(x, y -1, i + 1);
    if (is_in_field(x - 1, y))
        get_path(x -1, y, i + 1);
}
void reset_field_for_find() {
    for (int i = 0;i < size;i++)
        for (int ii = 0;ii < size;ii++) {
            field[i][ii].visited = false;
            field[i][ii].dist = -1;
            field[i][ii].dest = false;
            if (field[i][ii].crates > 0)
                field[i][ii].free = false;
        }

}

void backtrack(int x, int y , vector<pair<int, int> >& p) {
    p.push_back(make_pair<int, int>(x,y));
    int i = field[x][y].dist - 1;
    if (i == -1)
        return;
    if (is_in_field(x + 1, y) && field[x + 1][y].dist  == i &&  field[x+1][y].visited)
        backtrack(x + 1, y, p);
    else if (is_in_field(x, y + 1) && field[x][y + 1].dist  == i &&  field[x][y + 1].visited)
        backtrack(x, y + 1, p);
    else if (is_in_field(x, y - 1) && field[x][y - 1].dist  == i && field[x][y - 1].visited)
        backtrack(x, y -1,p);
    else if (is_in_field(x - 1, y) && field[x - 1][y].dist  == i && field[x - 1][y].visited)
        backtrack(x -1, y,p);
}

vector<pair<int, int> > find_path(int x1,int y1, int x2, int y2) {
    closest = make_pair<int, int>(-1,-1);
    reset_field_for_find();
    field[x2][y2].dest = true;
    get_path(x1,y1,0);
    vector<pair<int, int> > p(0);
    if (closest.first == -1)
        return p;
    else {
        backtrack(closest.first, closest.second, p);
        reverse(p.begin(), p.end());
        vector<pair<int, int> > pp(0);
        for (int i = 0;i < p.size() - 1;i++)
            pp.push_back(make_pair<int,int>(p[i+1].first - p[i].first, p[i+1].second - p[i].second));
        return pp;
    }

}
vector<pair<int, int> > find_path_to_depo(int x1,int y1) {
	return find_path(x1,y1,0,0);
}



vector<vector<int> > load_building() {
	ifstream file;
	string line;
	unsigned int crates;
	file.open("building.txt");
	vector<vector<int> > surface(0);
	if (file.is_open()) {
		while(getline(file, line)) {
			vector<int> row;
			istringstream iss(line);
			while (iss >> crates)
			{
				row.push_back(crates);
			}
			surface.push_back(row);
		}
		file.close();
	}
	return surface;
}
void generate_orders() {
	vector<vector<int> > p = load_building();

    for (int i = 0; i < p.size(); i++)
        for (int ii = 0; ii < p[i].size(); ii++) {
            for (int d = 0; d < p[i][ii]; d++) {
                orders.push_back(make_pair<int, int>(i+1, ii+1));


            }
            field[i+1][ii+1].crates_to_build = p[i][ii];
        }
    random_shuffle ( orders.begin(), orders.end() );

}
void print_path(vector<pair<int, int> > p) {
    cout << "path" << endl;
    for (int i = 0; i < p.size(); i++) {

        cout << p[i].first << p[i].second << endl;
    }
}

class Robot {
public:
    pos start_pos;
    Uint32 start_time;
    vector<pos> path;
    bool has_crate;
	Model podvozek, plosina;
    enum State { MOVE_TO_DEPO, MOVE_TO_BUILDING, LIFT_UP, LIFT_DOWN, STORE, STOPPED } state;
	Robot() {
	    start_pos = make_pair<int, int>(0,5);
		start_time = globalTime;
		state = Robot::MOVE_TO_DEPO;
		has_crate = false;
		path = find_path_to_depo(0,5);	
		podvozek = Model("podvozek.obj");
		plosina = Model("plosina.obj");
		print_path(path);
	}
	void get_state() {
		switch (state) {
        case MOVE_TO_DEPO:
        {
            float ujdene_sekundy = (globalTime - start_time) / 1000.0;
			int policka = ujdene_sekundy;
			if (policka >= path.size()) {
				start_time += 1000 * (float) path.size();
				start_pos.first = 0;
				start_pos.second = 0;
				if(where_to_put()) {
					state = MOVE_TO_BUILDING;
					print_path(path);

					has_crate = true;
				}
				else
					state = STOPPED;
			}
			break;
		}
        case MOVE_TO_BUILDING:
        {
            float ujdene_sekundy = (globalTime - start_time) / 1000.0;
			int policka = ujdene_sekundy;
			if (policka >= path.size() - 1) {
				start_time += 1000 * (float) (path.size() - 1);
                int x = start_pos.first;
                int y = start_pos.second;			
                for (int i = 0; i < path.size() - 1; i++) {
                    x += path[i].first;
                    y += path[i].second;
                }				
                start_pos.first = x;
				start_pos.second = y;
				state = LIFT_UP;
			}
			break;
		}
        case LIFT_UP:
        {
            float patro = (globalTime - start_time) / 1000.0;
            if (field[start_pos.first + path[path.size()-1].first][start_pos.second + path[path.size()-1].second].crates <= patro) {
                start_time += field[start_pos.first + path[path.size()-1].first][start_pos.second + path[path.size()-1].second].crates * 1000.0;
                state = STORE;
                cout << "vyklada" << endl;
			}
			break;
		}
		case STORE:
		{
		    float time = (globalTime - start_time) / 1000.0;
            if (time > 2.0) {
                start_time +=  2000.0;
                state = LIFT_DOWN;
                field[start_pos.first + path[path.size()-1].first][start_pos.second + path[path.size()-1].second].crates =
                    field[start_pos.first + path[path.size()-1].first][start_pos.second + path[path.size()-1].second].crates + 1;
                field[start_pos.first + path[path.size()-1].first][start_pos.second + path[path.size()-1].second].free = false;
				Crate c;
                c.x = start_pos.first + path[path.size()-1].first;
                c.y = start_pos.second + path[path.size()-1].second;
                c.level = field[start_pos.first + path[path.size()-1].first][start_pos.second + path[path.size()-1].second].crates -1;
                c.otoceni = path[path.size()-1];
                crates.push_back(c);
				has_crate = false;
			}
			break;
		}
		case LIFT_DOWN:
        {
            float patro = (globalTime - start_time) / 1000.0;
			int crates = field[start_pos.first + path[path.size()-1].first][start_pos.second + path[path.size()-1].second].crates - 1;
            if (crates < patro) {
                //dalsi stav
                if (to_depo()) {
// 					print_path(path);
                    state = MOVE_TO_DEPO;
                    start_time += (crates) * 1000.0;
                }
			} else {
				state = STOPPED;
				cout << "zastaveny";
			}
			break;
		}
		}
		
	}
    void render() {
		get_state();
        switch (state) {
        case MOVE_TO_DEPO:
        {
                float vzdalenost = (globalTime - start_time) / 1000.0;
                int policka = vzdalenost;
                float x = start_pos.first;
                float y = start_pos.second;
                for (int i = 0; i < policka; i++) {
                    x += path[i].first;
                    y += path[i].second;
                }

				x += (float)path[policka].first * (vzdalenost - floor(vzdalenost));
				y += (float)path[policka].second * (vzdalenost - floor(vzdalenost));
				glMatrixMode ( GL_MODELVIEW );
				glPushMatrix();
				glTranslatef(x + 0.5, 0, y + 0.5);
				podvozek.render();
				plosina.render();
				if (has_crate)
					draw_crate();
				glPopMatrix();
            break;
            }
        case MOVE_TO_BUILDING:
        {
            float vzdalenost = (globalTime - start_time) / 1000.0;
            int policka = vzdalenost;
            float x = start_pos.first;
            float y = start_pos.second;
            for (int i = 0; i < policka; i++) {
                x += path[i].first;
                y += path[i].second;
            }

            x += (float)path[policka].first * (vzdalenost - floor(vzdalenost));
            y += (float)path[policka].second * (vzdalenost - floor(vzdalenost));
            glMatrixMode ( GL_MODELVIEW );
            glPushMatrix();
            glTranslatef(x + 0.5, 0, y +0.5 );
            podvozek.render();
            plosina.render();
            glTranslatef(0, 0.5, 0 );
            if (has_crate)
                draw_crate();
            glPopMatrix();
            break;
        }




        case STOPPED:
        {
            glMatrixMode ( GL_MODELVIEW );
            glPushMatrix();
            glTranslatef(start_pos.first + 0.5, 0.5, start_pos.second + 0.5);
            if (has_crate)
                draw_crate();
            glPopMatrix();

            glPushMatrix();
            glTranslatef(start_pos.first + 0.5, 0, start_pos.second + 0.5);
            podvozek.render();
			plosina.render();
            glPopMatrix();
			break;
        }
     
        case LIFT_UP:
        {
            float patro = (globalTime - start_time) / 1000.0;
            glMatrixMode ( GL_MODELVIEW );
            glPushMatrix();
            glTranslatef(start_pos.first + 0.5, patro, start_pos.second + 0.5);
            draw_crate();
            glPopMatrix();

            glPushMatrix();
            glTranslatef(start_pos.first + 0.5, 0, start_pos.second + 0.5);
            podvozek.render();
            glTranslatef(0,0.5,0);
            draw_tyc(patro);
            glTranslatef(0,patro-0.5,0);
            plosina.render();
            glPopMatrix();

            break;
        }
     
        case STORE:
        {
				float time = (globalTime - start_time) / 1000.0;
                glMatrixMode ( GL_MODELVIEW );
                glPushMatrix();

                if (path[path.size()-1].first == 1) {
                    glTranslatef(start_pos.first + 1.0, field[start_pos.first + path[path.size()-1].first][start_pos.second + path[path.size()-1].second].crates, start_pos.second);
                    glRotatef(-time * 45, 0, 0, 1);
                    glTranslatef(-0.5, 0.5, 0.5);
                }
                if (path[path.size()-1].first == -1) {
                    glTranslatef(start_pos.first, field[start_pos.first + path[path.size()-1].first][start_pos.second + path[path.size()-1].second].crates, start_pos.second);
                    glRotatef( time * 45, 0, 0, 1);
                    glTranslatef(0.5, 0.5, 0.5);
                }

                if (path[path.size()-1].second == 1) {
                    glTranslatef(start_pos.first, field[start_pos.first + path[path.size()-1].first][start_pos.second + path[path.size()-1].second].crates, start_pos.second + 1.0);
                    glRotatef(time * 45, 1, 0, 0);
                    glTranslatef(0.5, 0.5, -0.5);
                }
                if (path[path.size()-1].second == -1) {
                    glTranslatef(start_pos.first, field[start_pos.first + path[path.size()-1].first][start_pos.second + path[path.size()-1].second].crates, start_pos.second  );
                    glRotatef(-time * 45, 1, 0, 0);
                    glTranslatef(0.5, 0.5, 0.5);
                }
                draw_crate();
                glPopMatrix();

                glPushMatrix();
                glTranslatef(start_pos.first + 0.5, 0, start_pos.second + 0.5);
                podvozek.render();
				
				glTranslatef(0,0.5,0);
                draw_tyc( field[start_pos.first + path[path.size()-1].first][start_pos.second + path[path.size()-1].second].crates);
				glTranslatef(0,field[start_pos.first + path[path.size()-1].first][start_pos.second + path[path.size()-1].second].crates - 0.5,0);
				plosina.render();
                glPopMatrix();
            break;
        }
        case LIFT_DOWN:
        {
                //nastav vysku
				float patro = (globalTime - start_time) / 1000.0;
                glPushMatrix();
                glTranslatef(start_pos.first + 0.5, 0, start_pos.second + 0.5);
                podvozek.render();
				glTranslatef(0,0.5,0);
                draw_tyc((field[start_pos.first + path[path.size()-1].first][start_pos.second + path[path.size()-1].second].crates - 1) - patro);
				glTranslatef(0,(field[start_pos.first + path[path.size()-1].first][start_pos.second + path[path.size()-1].second].crates - 1) - patro -0.5,0);
				plosina.render();
                glPopMatrix();
        }
		
        }


    }

    bool where_to_put() {

        vector<pair<int, int> >::iterator it;

        vector<pair<int, int> > pp;
        bool found = false;
        int ii;
        for (int i  = 0; i < orders.size(); i++) {
            pp = find_path(start_pos.first, start_pos.second, orders[i].first, orders[i].second);
            if (!pp.empty()) {
                found = true;
                ii = i;
                break;
            }
        }

        path = pp;

        if (found) {
            vector<pair<int, int> >::iterator nth = orders.begin() + ii;
            orders.erase(nth);
            return true;
        } return false;
    }
    bool to_depo() {
        vector<pair<int, int> > pp;
        pp = find_path_to_depo(start_pos.first, start_pos.second);

        path = pp;
        return true;
    }
//     void path_from_field() {
//         int x = start_pos.first;
//         int y = start_pos.second;
//         for (int i  = 0; i < path.size(); i++) {
//             field[x][y].free = true;
//             x += path[i].first;
//             y += path[i].second;
//         }
// 
//     }
//     void path_to_field() {
//         int x = start_pos.first;
//         int y = start_pos.second;
//         for (int i  = 0; i < path.size(); i++) {
//             field[x][y].free = false;
//             x += path[i].first;
//             y += path[i].second;
//         }
// 
//     }
};


Robot robot1;

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



void load_textures() {

    load_texture("crate.png", &crate_texture);

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
//     glBindTexture ( GL_TEXTURE_2D, texture );
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
	if(objData)
		delete ( objData );
    if (crate_texture)
        glDeleteTextures ( 1,&crate_texture );
    /* Exit program. */
	glDeleteLists(start_list, list_count);
    SDL_Quit();
	if(hand)
		delete hand;
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


void draw_tyc(float vyska) {

    glBindTexture(GL_TEXTURE_2D, NULL);

    glColor3f(0.1, 0.1, 0.7);

    glBegin ( GL_QUADS);
    // Front Face
    // Bottom Face
    glNormal3f(0,-1,0);
    // Front Face
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-0.2f, -0.5f,  0.2f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f( 0.2f, -0.5f,  0.2f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f( 0.2f,  -0.5f + vyska,  0.2f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-0.2f,   -0.5f + vyska,  0.2f);	// Top Left Of The Texture and Quad
// 		Back Face
    glNormal3f(0,0,-1);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-0.2f, -0.5f, -0.2f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-0.2f,   -0.5f + vyska, -0.2f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f( 0.2f,   -0.5f + vyska, -0.2f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f( 0.2f, -0.5f, -0.2f);	// Bottom Left Of The Texture and Quad
    // Top Face
    //TODO weird normal
    glNormal3f(0,-1,0);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-0.2f,   -0.5f + vyska, -0.2f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-0.2f,   -0.5f + vyska,  0.2f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f( 0.2f,   -0.5f + vyska,  0.2f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f( 0.2f,   -0.5f + vyska, -0.2f);	// Top Right Of The Texture and Quad
    // Bottom Face
    glNormal3f(0,1,0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-0.2f, -0.5f, -0.2f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f( 0.2f, -0.5f, -0.2f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f( 0.2f, -0.5f,  0.2f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-0.2f, -0.5f,  0.2f);	// Bottom Right Of The Texture and Quad
    // Right face
    glNormal3f(1,0,0);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f( 0.2f, -0.5f, -0.2f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f( 0.2f,   -0.5f + vyska, -0.2f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f( 0.2f,   -0.5f + vyska,  0.2f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f( 0.2f, -0.5f,  0.2f);	// Bottom Left Of The Texture and Quad
    // Left Face
    glNormal3f(-1,0,0);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-0.2f, -0.5f, -0.2f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-0.2f, -0.5f,  0.2f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-0.2f,   -0.5f + vyska,  0.2f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-0.2f,  -0.5f + vyska, -0.2f);	// Top Left Of The Texture and Quad

    glEnd();

}
void draw_depo() {

    glBindTexture(GL_TEXTURE_2D, NULL);

    glColor3f(1, 0, 0);
    glBegin ( GL_QUADS);
    // Front Face
    // Bottom Face
    glNormal3f(0,-1,0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(0, 0, 0);	// Top Right Of The Texture and Quad
    glTexCoord2f(.0f, 1.0f);
    glVertex3f( 0, 0, 1);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f( 1, 0,  1);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1, 0,  0);	// Bottom Right Of The Texture and Quad

    glEnd();
}

void draw_crates() {
    for (int i = 0; i< crates.size(); i++) {
        glPushMatrix();
glTranslatef(crates[i].x + 0.5 , crates[i].level + 0.5, crates[i].y + 0.5);

//         if (crates[i].otoceni.first == 1) {
//             glTranslatef(crates[i].x, field[crates[i].x ][crates[i].y ].crates, crates[i].y );
//             glRotatef(-90, 0, 0, 1);
//             glTranslatef(-0.5, 0.5, 0.5);
//         }
//         if (crates[i].otoceni.first == -1) {
//             glTranslatef(crates[i].x + 1.0, field[crates[i].x ][crates[i].y ].crates , crates[i].y );
//             glRotatef(90, 0, 0, 1);
//             glTranslatef(0.5, 0.5, 0.5);
//         }
// 
//         if (crates[i].otoceni.second == 1) {
//             glTranslatef(crates[i].x + 1.0, field[crates[i].x][crates[i].y ].crates, crates[i].y + 1.0 );
//             glRotatef(90, 1, 0, 0);
//             glTranslatef(0.5, 0.5, -0.5);
//         }
//         if (crates[i].otoceni.second == -1) {
//             glTranslatef(crates[i].x, field[crates[i].x][crates[i].y].crates , crates[i].y + 1.0);
//             glRotatef(-90, 1, 0, 0);
//             glTranslatef(0.5, 0.5, 0.5);
//         }

	    draw_crate();
        glPopMatrix();
    }
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


draw_depo();

    draw_crates();
	hand->render();
    robot1.render();
// 	robot2.render();
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

}
void setup_shading() {
    glShadeModel ( GL_SMOOTH );
}
void setup_lighting() {

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    float l = 1.0;
    glEnable(GL_COLOR_MATERIAL);

    glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE,&l);


    GLfloat light_position[] = {0,10,10,1.0};//w=0:infinite
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
	
	lists = glGenLists(list_count);
	start_list = lists;
    setup_texturing();
    setup_shading();
    setup_lighting();
	
    setup_matrices(width, height);

}


void load_models() {
	hand = new Hand();

}


int main ( int argc, char* argv[] ) {
    srand ( unsigned ( time (NULL) ) );
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
    width = 800;
    height = 800;
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
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		cerr << "glew init failed:/" << endl;
        quit(1);
	}
    setup_opengl ( width, height );
    load_font();
    load_textures();
	load_models();
    generate_orders();
	robot1 = Robot();

    /*
     * Now we want to begin our normal app process--
     * an event loop with a lot of redrawing.
     */
    while ( 1 ) {
        /* Process incoming events. */
        process_events( );
        /* Draw the screen. */
        draw_screen( );
        SDL_Delay(5);

    }

    /* Never reached. */
    quit(0);
}
