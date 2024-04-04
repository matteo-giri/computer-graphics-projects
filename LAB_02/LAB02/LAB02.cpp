// LAB02.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//


#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include "HUD_Logger.h"
using namespace glm;

static unsigned int programId;

unsigned int VAO_CIELO, VAO_STELLE, VAO_NAVICELLA, VAO_ASTEROIDE, VAO_PROIETTILE, VAO_PROPULSORE, VAO_FRAMMENTO, VAO_UFO;
unsigned int VBO_C, VBO_S, VBO_N, VBO_A, VBO_Proj, VBO_Prop, VBO_F, VBO_U;
unsigned int MatProj, MatModel;

mat4 Projection;  //Matrice di proiezione
mat4 Model; //Matrice per il cambiamento da Sistema di riferimento dell'oggetto OCS a sistema di riferimento nel Mondo WCS

//viewport size
int	width = 700;
int	height = 700;

int score_asteroidi = 0;
int score_ufo = 0;

//definizione strutture degli elementi in scena
typedef struct {float r, g, b;} color;
typedef struct { float x, y, r, g, b, a; } Pointxy;

typedef struct { //particella (per propulsore, frammenti degli asteroidi)
	float x,y,alpha,xFactor,yFactor,drag;
	color color;
} PARTICLE;

typedef struct { //struttura dell'asteroide (posizione, angolo di rotazione, variazione in x e y del movimento, lato della finestra in cui spawna l'asteroide, velocità)
	float xpos, ypos,angle;
	int deltax, deltay, spawn_side,speed;
} Asteroid;

typedef struct { //struttura del proiettile (posizione, angolo di movimento, colore)
	float xpos, ypos, angle;
	vec4 col;
	bool enemy; //true se il proiettile è stato sparato da un nemico
} Projectile;

typedef struct { //struttura dell'ufo
	float xpos, ypos;
	int deltax, deltay, spawn_side, speed, life;
	int attack_time;
} UFO;

//Parametri per il cielo
int vertices_Cielo = 6;
Pointxy* Cielo = new Pointxy[vertices_Cielo];

//Parametri per le stelle
int stars_number = 60;
Pointxy* Stelle = new Pointxy[stars_number];

//Parametri per la navicella
int vertices_Navicella = 6;
Pointxy* Navicella = new Pointxy[vertices_Navicella];
double velocità_max = 10;
double velocità = 0;
int delta_rot = 4;  //velocità di rotazione
int navicella_width = 35;
int navicella_height = 40;
float navicella_posx = float(width) / 2;
float navicella_posy = float(height) / 2;
float navicella_angle = 0; //angolo in cui è disegnata la navicella
float navicella_moving_angle = 0; //angolo di movimento della navicella (diverso dall'angolo in cui è disegnata perchè se non uso il propulsore la navicella deve andare nella stessa direzione anche se la ruoto)
bool navicella_alive = true; //true se la navicella non è esplosa (game over)

//Parametri per il propulsore
vector <PARTICLE> particles; //vettore che contiene le particella
int number_particles = 5000;
Pointxy* Propulsore = new Pointxy[number_particles]; //vettore che contiene le particelle aggiornate da essere disegnate a schermo (uso particles per fare tutte le operazioni sulle particelle perchè è più semplice aggiungere e togliere elementi su vector)
void spawn_propulsore(color color);

//parametri per i frammenti
vector <PARTICLE> frammenti; //vettore che contiene i frammenti
int number_fragments = 5000;
Pointxy* Frammento = new Pointxy[number_fragments]; //vettore che contiene i fragmenti aggiornati da essere disegnati a schermo
void spawn_frammenti(float xpos,float ypos,vec4 color, int num_particles);
int frammenti_size = 6.0;
int frammenti_number = 25; //numero di frammenti spawnati per ogni asteroide distrutto

//parametri per gli asteroidi
int vertices_asteroide = 12;
int max_asteroidi = 5; //numero massimo di asteroidi a schermo
int number_asteroidi = 0;
int asteroide_width = 40;
int asteroide_height = 40;
Pointxy* Asteroide = new Pointxy[vertices_asteroide]; //struttura che contiene i vertici per disegnare un singolo asteroide
vector <Asteroid> asteroidi; //struttura che contiene gli asteroidi in scena

//parametri per i proiettili
int max_proiettili = 20;
int proj_speed = 5;
int proj_size = 8.0;
vector <Projectile> proiettili; // vettore che contiene i proiettili
Pointxy* Proiettile = new Pointxy[max_proiettili]; //vettore che contiene i proiettili aggiornati da essere disegnati a schermo
void spawn_proiettile(float xpos, float ypos, float angle, vec4 color, bool enemy);

//parametri per gli ufo
int vertices_ufo = 36;
int max_ufo = 1; //numero massimo di ufo a schermo
int number_ufo = 0;
int ufo_width = 70;
int ufo_height = 40;
int ufo_life = 5; //vita degli ufo
int ufo_attackTime = 40; //velocità d'attacco degli ufo
Pointxy* Ufo = new Pointxy[vertices_ufo]; //struttura che contiene i vertici per disegnare un singolo ufo
vector <UFO> ufos; //struttura che contiene gli ufo in scena


bool pressing_left = false;
bool pressing_right = false;
bool pressing_up = false;

//colori
vec4 col_dark_blue = { 0.0,0.0,0.15,1.0 };
vec4 col_black = { 0.0,0.0,0.0,1.0 };
vec4 col_red = { 1.0,0.0,0.0,1.0 };
vec4 col_white = { 1.0,1.0,1.0,1.0 };
vec4 col_light_yellow = { 1.0,1.0,0.5,1.0 };
vec4 col_brown = {0.55,0.45,0.15,1.0};
vec4 col_grey = { 0.24,0.24,0.24,1.0 };
vec4 col_light_grey = { 0.6,0.6,0.6,1.0 };
vec4 col_green = {0.5,1.0,0.23,1.0};

// Restituisce un numero casuale tra "arange" e "brange"
int random_int(int arange,int brange) {
	std::random_device rd;
	std::mt19937 rng(rd()); 
	std::uniform_int_distribution<int> uni(arange, brange);
	return uni(rng);
}

// genera il colore delle particelle del propulsore
color computeRainbow() {
	static float rgb[3] = { 1.0, 0.0, 0.0 };
	static int fase = 0, counter = 0;
	const float step = 0.1;
	color paint;

	switch (fase) {
	case 0: rgb[1] += step;
		break;
	case 1: rgb[0] -= step;
		break;
	case 2: rgb[2] += step;
		break;
	case 3: rgb[1] -= step;
		break;
	case 4: rgb[0] += step;
		break;
	case 5: rgb[2] -= step;
		break;
	default:
		break;
	}

	counter++;
	if (counter > 1.0 / step) {
		counter = 0;
		fase < 5 ? fase++ : fase = 0;
	}

	paint.r = rgb[0];
	paint.g = rgb[1];
	paint.b = rgb[2];
	return paint;
}


/// ///////////////////////////////////////////////////////////////////////////////////
///									Gestione eventi
///////////////////////////////////////////////////////////////////////////////////////
void keyboardPressedEvent(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		pressing_left = true;
		break;
	case 'd':
		pressing_right = true;
		break;
	case 'w':
		pressing_up = true;
		break;
	case ' ':
		if (navicella_alive) {
			float xpos = (float)navicella_posx + (navicella_width / 2)*sin(radians(navicella_angle));
			float ypos = (float)navicella_posy + (navicella_height / 2)*cos(radians(navicella_angle));
			spawn_proiettile(xpos, ypos, navicella_angle, col_red, false);

		}
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}

void keyboardReleasedEvent(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		pressing_left = false;
		break;
	case 'd':
		pressing_right = false;
		break;
	case 'w':
		pressing_up = false;
		break;
	default:
		break;
	}
}

//funzione chiamata quando la navicella esplode
void killNavicella() {
	navicella_alive = false;
	spawn_frammenti(navicella_posx, navicella_posy, col_white, 100); //spawno i frammenti della navicella distrutta
	cout << endl << "\nGAME OVER" << endl;
	cout << "Asteroidi distrutti: " << score_asteroidi << endl;
	cout << "Ufo distrutti: " << score_ufo << endl;
}

//funzione per l'aggiornamento della navicella (e del propulsore)
void update_navicella(int a) {
	bool moving = false;		
	
	if (navicella_alive) {
		//se mi sto muovendo aumento sempre di più la velocità
		if (pressing_up)
		{
			if (velocità <= velocità_max)
				velocità += 0.4;
			moving = true;
			navicella_moving_angle = navicella_angle; //solo se mi sto muovendo aggiorno l'angolo di movimento con l'angolo della navicella
		}

		//se non mi sto muovendo diminuisco la velocità
		if (!moving) {
			if (velocità >= 0) {
				velocità -= 0.2;
				if (velocità < 0)
					velocità = 0;
			}
			if (velocità < 0) {
				velocità += 0.2;
				if (velocità > 0)
					velocità = 0;
			}
		}

		//se premo a o d devo ruotare la navicelle
		if (pressing_right) {
			navicella_angle = (int)(navicella_angle + delta_rot) % 360;
		}
		if (pressing_left) {
			if (navicella_angle == 0)
				navicella_angle = 360 - delta_rot;
			else
				navicella_angle -= delta_rot;
		}

		//la posizione della navicella è aggiornata in base alla velocità e all'angolo di movimento
		navicella_posx += velocità * sin(radians(navicella_moving_angle));
		navicella_posy += velocità * cos(radians(navicella_moving_angle));

		//se la navicella supera i bordi rimbalza indietro
		if (navicella_posx < 0.0) {
			navicella_posx = 0.0;
			velocità = velocità * 0.8;
			navicella_moving_angle = 360 - navicella_angle;
			navicella_angle = 360 - navicella_angle;
		}
		if (navicella_posx > width) {
			navicella_posx = width;
			velocità = velocità * 0.8;
			navicella_moving_angle = 360 - navicella_angle;
			navicella_angle = 360 - navicella_angle;
		}
		if (navicella_posy > height) {
			navicella_posy = height;
			velocità = velocità * 0.8;
			navicella_moving_angle = 180 - navicella_angle;
			navicella_angle = 180 - navicella_angle;
		}
		if (navicella_posy < 0) {
			navicella_posy = 0;
			velocità = velocità * 0.8;
			navicella_moving_angle = 180 - navicella_angle;
			navicella_angle = 180 - navicella_angle;
		}

		//se sto premendo w devo far vedere il propulsore
		if (moving) {
			color rgb = computeRainbow();
			spawn_propulsore(rgb);
		}

		//per ogni asteroide, vedo se la navicella ha colpito l'asteroide
		for (int j = 0; j < asteroidi.size(); j++) {
			float a_xpos = asteroidi.at(j).xpos;
			float a_ypos = asteroidi.at(j).ypos;
			if (navicella_posx > a_xpos - asteroide_width / 2 - navicella_width / 2 && navicella_posx < a_xpos + asteroide_width / 2 + navicella_width / 2 && navicella_posy> a_ypos - asteroide_height / 2 - navicella_height / 2 && navicella_posy < a_ypos + asteroide_height / 2 + navicella_height / 2) { //navicella nel box dell'asteroide
				killNavicella();
			}
		}

		//per ogni ufo, vedo se la navicella ha colpito l'ufo
		for (int j = 0; j < ufos.size(); j++) {
			float a_xpos = ufos.at(j).xpos;
			float a_ypos = ufos.at(j).ypos;
			if (navicella_posx > a_xpos - ufo_width / 2 - navicella_width / 2 && navicella_posx < a_xpos + ufo_width / 2 + navicella_width / 2 && navicella_posy> a_ypos - ufo_height / 2 - navicella_height / 2 && navicella_posy < a_ypos + ufo_height / 2 + navicella_height / 2) { //navicella nel box dell'asteroide
				killNavicella();
			}
		}
	}

	glutPostRedisplay();
	glutTimerFunc(24, update_navicella, 0);
}

//funzione per l'aggiornamento degli asteroidi
void update_asteroidi(int a) {
	if (asteroidi.size() < max_asteroidi) {
		//genera asteroide (l'asteroide può spawnare in uno dei 4 lati dello schermo, in base al lato cambiano i modi in cui spawna e in cui si muove)
		int spawn_side = random_int(1, 4);
		int speed = random_int(1, 3);
		Asteroid ast;
		if (spawn_side == 1) {
			ast.xpos = random_int(-width / 2, width / 2);
			ast.ypos = height + asteroide_height;
			ast.deltax = 1;
			ast.deltay = -1;			
		}
		else if (spawn_side == 2) {
			ast.xpos = width + asteroide_width;
			ast.ypos = random_int(height/2, height / 2 + height);
			ast.deltax = -1;
			ast.deltay = -1;
		}
		else if (spawn_side == 3) {
			ast.xpos = random_int(-width / 2, width / 2);
			ast.ypos = 0-asteroide_height;
			ast.deltax = 1;
			ast.deltay = 1;
		}
		else if (spawn_side == 4) {
			ast.xpos = 0 - asteroide_width;
			ast.ypos = random_int(-height / 2, height / 2);
			ast.deltax = 1;
			ast.deltay = 1;
		}
		ast.spawn_side = spawn_side;
		ast.angle = 0;
		ast.speed = speed;
		asteroidi.push_back(ast);
	}

	//cambia posizione asteroidi
	for (int i = 0; i < asteroidi.size(); i++) {
		asteroidi.at(i).xpos += asteroidi.at(i).speed*asteroidi.at(i).deltax;
		asteroidi.at(i).ypos += asteroidi.at(i).speed*asteroidi.at(i).deltay;
		asteroidi.at(i).angle = (int)(asteroidi.at(i).angle + 1) % 360; //ruoto l'asteroide

		//elimina asteroidi fuori dallo schermo
		if (asteroidi.at(i).xpos > width + 3*asteroide_width || asteroidi.at(i).xpos < - 3 * asteroide_width || asteroidi.at(i).ypos <- 3*asteroide_height|| asteroidi.at(i).ypos > height + 3 * asteroide_height) {
			asteroidi.erase(asteroidi.begin() + i);
		}
	}
	glutPostRedisplay();
	glutTimerFunc(24, update_asteroidi, 0);
}

//funzione per l'aggiornamento degli ufo
void update_ufos(int a) {
	if (ufos.size() < max_ufo) {
		//genera ufo (l'ufo può spawnare in uno dei 2 lati dello schermo, in base al lato cambiano i modi in cui spawna e in cui si muove)
		int spawn_side = random_int(1, 2);
		int speed = random_int(1, 3);
		UFO u;
		if (spawn_side == 1) { //sinistra
			u.xpos = 0 - ufo_width;
			u.ypos = random_int(height * (1.0/4.0), height * (10.0/11.0));
			u.deltax = 1;
			u.deltay = 1;
		}
		else if (spawn_side == 2) { //destra
			u.xpos = width + ufo_width;
			u.ypos = random_int(height * (1.0 / 4.0), height * (10.0 / 11.0));
			u.deltax = -1;
			u.deltay = 1;
		}
		u.spawn_side = spawn_side;
		u.speed = speed;
		u.life = ufo_life;
		u.attack_time = 0;
		ufos.push_back(u);
	}

	//cambia posizione ufos
	for (int i = 0; i < ufos.size(); i++) {
		ufos.at(i).xpos += ufos.at(i).deltax*ufos.at(i).speed;
		ufos.at(i).ypos += 0.9*sin(ufos.at(i).xpos/10);

		//faccio sparare l'ufo
		ufos.at(i).attack_time++;
		if (ufos.at(i).attack_time >= ufo_attackTime) {
			ufos.at(i).attack_time = 0;
			float xposp = (float)ufos.at(i).xpos;
			float yposp = (float)ufos.at(i).ypos - (ufo_height / 2);
			spawn_proiettile(xposp, yposp, 180, col_green, true);
		}

		//elimina ufo fuori dallo schermo
		if (ufos.at(i).xpos > width + 3 * ufo_width || ufos.at(i).xpos < -3 * ufo_width) {
			ufos.erase(ufos.begin() + i);
		}
	}
	glutPostRedisplay();
	glutTimerFunc(24, update_ufos, 0);
}

//funzione per l'aggiornamento dei proiettili (e eventuali asteroidi colpiti)
void update_proiettili(int a) {
	for (int i = 0; i < proiettili.size(); i++) {
		bool colpito = false;

		//sposto il proiettile in avanti (in base all'angolo con cui è stato sparato)
		proiettili.at(i).xpos += proj_speed * sin(radians(proiettili.at(i).angle));
		proiettili.at(i).ypos += proj_speed * cos(radians(proiettili.at(i).angle));

		float xpos = proiettili.at(i).xpos;
		float ypos = proiettili.at(i).ypos;

		//per ogni asteroide, vedo se il proiettile ha colpito l'asteroide
		if (!colpito && proiettili.at(i).enemy == false) {
			for (int j = 0; j < asteroidi.size(); j++) {
				float a_xpos = asteroidi.at(j).xpos;
				float a_ypos = asteroidi.at(j).ypos;
				if (xpos > a_xpos - asteroide_width / 2 && xpos <a_xpos + asteroide_width / 2 && ypos> a_ypos - asteroide_height / 2 && ypos < a_ypos + asteroide_height / 2) { //proiettile nel box dell'asteroide
					colpito = true;
					score_asteroidi += 1;
					proiettili.erase(proiettili.begin() + i); //cancello il proiettile che ha colpito
					asteroidi.erase(asteroidi.begin() + j); //cancello l'asteroide che è stato colpito
					spawn_frammenti(a_xpos, a_ypos, col_brown, frammenti_number); //spawno i frammenti dell'asteroide distrutto
					cout << "\nAsteroidi distrutti: " << score_asteroidi;
				}
			}
		}

		//per ogni ufo, vedo se il proiettile ha colpito l'ufo
		if (!colpito && proiettili.at(i).enemy == false) {
			for (int j = 0; j < ufos.size(); j++) {
				float a_xpos = ufos.at(j).xpos;
				float a_ypos = ufos.at(j).ypos;
				if (xpos > a_xpos - ufo_width / 2 && xpos <a_xpos + ufo_width / 2 && ypos> a_ypos - ufo_height / 2 && ypos < a_ypos + ufo_height / 2) { //proiettile nel box dell'ufo
					colpito = true;
					ufos.at(j).life--; //tolgo un punto vita all'ufo
					proiettili.erase(proiettili.begin() + i); //cancello il proiettile che ha colpito

					if (ufos.at(j).life == 0) {
						score_ufo += 1;
						ufos.erase(ufos.begin() + j); //cancello l'ufo che è stato colpito
						spawn_frammenti(a_xpos, a_ypos, col_grey, 50); //spawno i frammenti dell'ufo distrutto
						cout << "\nUfo distrutti: " << score_ufo;
					}
				}
			}
		}

		//vedo se il proiettile ha colpito la navicella
		if (!colpito && proiettili.at(i).enemy == true) {
			if (xpos > navicella_posx - navicella_width / 2 && xpos <navicella_posx + navicella_width / 2 && ypos> navicella_posy - navicella_height / 2 && ypos < navicella_posy + navicella_height / 2) { //proiettile nel box della navicella
				colpito = true;
				proiettili.erase(proiettili.begin() + i); //cancello il proiettile che ha colpito
				killNavicella();
			}
		}

		if (colpito == false) {
			// elimino i proiettili fuori dalla finestra
			if (xpos < 0 || xpos > width || ypos < 0 || ypos > height) { 		
				proiettili.erase(proiettili.begin() + i);
			}
			else { // se il proiettile è vivo, aggiorno la struttura che li contiene
				Proiettile[i].x = xpos;
				Proiettile[i].y = ypos;
				Proiettile[i].r = proiettili.at(i).col.r;
				Proiettile[i].g = proiettili.at(i).col.g;
				Proiettile[i].b = proiettili.at(i).col.b;
				Proiettile[i].a = proiettili.at(i).col.a;
			}
		}
	}

	glutPostRedisplay();
	glutTimerFunc(24, update_proiettili, 0);
}

////////////////////////////////////////////////////////////////////////////
void disegna_pianoxy(float x, float y, float width, float height, vec4 color_top, vec4 color_bot, Pointxy* piano)
{
	piano[0].x = x;	piano[0].y = y;
	piano[0].r = color_bot.r; piano[0].g = color_bot.g; piano[0].b = color_bot.b; piano[0].a = color_bot.a;
	piano[1].x = x + width;	piano[1].y = y;
	piano[1].r = color_bot.r; piano[1].g = color_bot.g; piano[1].b = color_bot.b; piano[1].a = color_bot.a;
	piano[2].x = x + width;	piano[2].y = y + height;
	piano[2].r = color_top.r; piano[2].g = color_top.g; piano[2].b = color_top.b; piano[2].a = color_top.a;

	piano[3].x = x + width;	piano[3].y = y + height;
	piano[3].r = color_top.r; piano[3].g = color_top.g; piano[3].b = color_top.b; piano[3].a = color_top.a;
	piano[4].x = x;	piano[4].y = y + height;
	piano[4].r = color_top.r; piano[4].g = color_top.g; piano[4].b = color_top.b; piano[4].a = color_top.a;
	piano[5].x = x;	piano[5].y = y;
	piano[5].r = color_bot.r; piano[5].g = color_bot.g; piano[5].b = color_bot.b; piano[5].a = color_bot.a;
}

void disegna_stelle(float stars_number, float width, float height, vec4 color, Pointxy* stelle) {
	for (int i = 0; i < stars_number; i++) {
		int x = random_int(0,width);
		int y = random_int(0,height);
		stelle[i].x = x;
		stelle[i].y = y;
		stelle[i].r = color.r; stelle[i].g = color.g; stelle[i].b = color.b; stelle[i].a = color.a;	
	}
}

void disegna_navicella(float width, float height, vec4 color, Pointxy* navicella) {
	navicella[0].x = -width/2; navicella[0].y = -height/2;
	navicella[0].r = color.r; navicella[0].g = color.g; navicella[0].b = color.b; navicella[0].a = color.a;
	navicella[1].x = 0; navicella[1].y = -height/8;
	navicella[1].r = color.r; navicella[1].g = color.g; navicella[1].b = color.b; navicella[1].a = color.a;
	navicella[2].x = 0; navicella[2].y = height/2;
	navicella[2].r = color.r; navicella[2].g = color.g; navicella[2].b = color.b; navicella[2].a = color.a;

	navicella[3].x = 0; navicella[3].y = -height/8;
	navicella[3].r = color.r; navicella[3].g = color.g; navicella[3].b = color.b; navicella[3].a = color.a;
	navicella[4].x = width/2; navicella[4].y = -height/2;
	navicella[4].r = color.r; navicella[4].g = color.g; navicella[4].b = color.b; navicella[4].a = color.a;
	navicella[5].x = 0; navicella[5].y = height/2;
	navicella[5].r = color.r; navicella[5].g = color.g; navicella[5].b = color.b; navicella[5].a = color.a;
}

void disegna_asteroide(float width, float height, vec4 color, Pointxy* asteroide) {
	float ipotenusa = sqrt(width*width + height * height);
	asteroide[0].x = -width/2; asteroide[0].y = -height/2;
	asteroide[1].x = width/2; asteroide[1].y = height/2;
	asteroide[2].x = -width/2; asteroide[2].y = height/2;
	asteroide[3].x = -width / 2; asteroide[3].y = -height / 2;
	asteroide[4].x = width / 2; asteroide[4].y = -height / 2;
	asteroide[5].x = width / 2; asteroide[5].y = height / 2;

	asteroide[6].x = 0; asteroide[6].y = -ipotenusa/2;
	asteroide[7].x = 0; asteroide[7].y = ipotenusa/2;
	asteroide[8].x = -ipotenusa/2; asteroide[8].y = 0;
	asteroide[9].x = 0; asteroide[9].y = -ipotenusa/2;
	asteroide[10].x = ipotenusa/2; asteroide[10].y = 0;
	asteroide[11].x = 0; asteroide[11].y = ipotenusa/2;

	for (int i = 0; i < vertices_asteroide; i++) {
		asteroide[i].r = color.r; asteroide[i].g = color.g; asteroide[i].b = color.b; asteroide[i].a = color.a;
	}
}

void disegna_ufo(float width, float height, vec4 color_top, vec4 color_bottom, vec4 color_alien, Pointxy* ufo) {
	//bottom
	ufo[0].x = -width/2; ufo[0].y = 0;
	ufo[1].x = -width/4; ufo[1].y = -height/2;
	ufo[2].x = -width/2; ufo[2].y = 0;
	ufo[3].x = -width/2; ufo[3].y = 0;
	ufo[4].x = -width/4; ufo[4].y = -height/2;
	ufo[5].x = width/4; ufo[5].y = 0;
	ufo[6].x = width / 4; ufo[6].y = 0;
	ufo[7].x = -width / 4; ufo[7].y = -height / 2;
	ufo[8].x = width / 4; ufo[8].y = -height / 2;
	ufo[9].x = width / 4; ufo[9].y = 0;
	ufo[10].x = width / 4; ufo[10].y = -height / 2;
	ufo[11].x = width / 2; ufo[11].y = 0;

	//top
	ufo[12].x = 0; ufo[12].y = 0;
	ufo[13].x = -width / 8; ufo[13].y = height / 2;
	ufo[14].x = -width / 4; ufo[14].y = 0;
	ufo[15].x = 0; ufo[15].y = 0;
	ufo[16].x = width / 8; ufo[16].y = height / 2;
	ufo[17].x = -width / 8; ufo[17].y = height / 2;
	ufo[18].x = 0; ufo[18].y = 0;
	ufo[19].x = width / 4; ufo[19].y = 0;
	ufo[20].x = width / 8; ufo[20].y = height / 2;

	//alien
	ufo[21].x = 0; ufo[21].y = 0;
	ufo[22].x = -width / 18; ufo[22].y = height / 3;
	ufo[23].x = -width / 8; ufo[23].y = height / 5;
	ufo[24].x = 0; ufo[24].y = 0;
	ufo[25].x = width / 18; ufo[25].y = height / 3;
	ufo[26].x = -width / 18; ufo[26].y = height / 3;
	ufo[27].x = 0; ufo[27].y = 0;
	ufo[28].x = width / 8; ufo[28].y = height / 5;
	ufo[29].x = width / 18; ufo[29].y = height / 3;

	ufo[30].x = -width / 20; ufo[30].y = height / 4;
	ufo[31].x = -width / 10; ufo[31].y = height / 5;
	ufo[32].x = -width / 30; ufo[32].y = height / 5;
	ufo[33].x = width / 20; ufo[33].y = height / 4;
	ufo[34].x = width / 10; ufo[34].y = height / 5;
	ufo[35].x = width / 30; ufo[35].y = height / 5;

	for (int i = 0; i < vertices_ufo - 9 - 9 - 6; i++) {
		ufo[i].r = color_bottom.r; ufo[i].g = color_bottom.g; ufo[i].b = color_bottom.b; ufo[i].a = color_bottom.a;
	}

	for (int i = vertices_ufo - 9 - 9 - 6; i < vertices_ufo -9 -6; i++) {
		ufo[i].r = color_top.r; ufo[i].g = color_top.g; ufo[i].b = color_top.b; ufo[i].a = color_top.a;
	}

	for (int i = vertices_ufo - 9 -6; i < vertices_ufo -6; i++) {
		ufo[i].r = color_alien.r; ufo[i].g = color_alien.g; ufo[i].b = color_alien.b; ufo[i].a = color_alien.a;
	}

	for (int i = vertices_ufo - 6; i < vertices_ufo; i++) {
		ufo[i].r = col_black.r; ufo[i].g = col_black.g; ufo[i].b = col_black.b; ufo[i].a = col_black.a;
	}
}

//funzione che disegna il propulsore (e aggiornamento delle particelle già esistenti nel propulsore, la creazione di nuove particelle del propulsore è nella update_navicella)
void disegna_propulsore() {
	int P_size = 0; // particles.size();
	// For each particle that is(still) alive we update the values :
	for (int i = 0; i < particles.size(); i++) {
		particles.at(i).xFactor /= particles.at(i).drag;
		particles.at(i).yFactor /= particles.at(i).drag;

		particles.at(i).x += particles.at(i).xFactor;
		particles.at(i).y += particles.at(i).yFactor;

		particles.at(i).alpha -= 0.05;  // reduce life

		float xPos = particles.at(i).x;
		float yPos = particles.at(i).y;

		if (particles.at(i).alpha <= 0.0) { // particle is dead
			particles.erase(particles.begin() + i);
		}
		else { // particle is alive, thus update
			Propulsore[i].x = xPos;
			Propulsore[i].y = yPos;
			Propulsore[i].r = particles.at(i).color.r;
			Propulsore[i].g = particles.at(i).color.g;
			Propulsore[i].b = particles.at(i).color.b;
			Propulsore[i].a = particles.at(i).alpha;
			P_size += 1;
		}
	}
	//tutto fino a qui potremme essere messo in una update (come per disegna_proiettili)

	//disegno il propulsore
	glBindVertexArray(VAO_PROPULSORE);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Prop);
	glBufferData(GL_ARRAY_BUFFER, P_size * sizeof(Pointxy), &Propulsore[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glPointSize(3.0);
	glDrawArrays(GL_POINTS, 0, P_size);
	glBindVertexArray(0);

}

//funzione che disegna tutti i frammenti che sono in scena (e li aggiorna, la creazione di nuovi frammenti avviene nell'update_proiettili)
void disegna_frammenti() {
	int P_size = 0; // particles.size();
	// For each particle that is(still) alive we update the values :
	for (int i = 0; i < frammenti.size(); i++) {
		frammenti.at(i).xFactor /= frammenti.at(i).drag;
		frammenti.at(i).yFactor /= frammenti.at(i).drag;

		frammenti.at(i).x += frammenti.at(i).xFactor;
		frammenti.at(i).y += frammenti.at(i).yFactor;

		frammenti.at(i).alpha -= 0.02;  // reduce life

		float xPos = frammenti.at(i).x;
		float yPos = frammenti.at(i).y;

		if (frammenti.at(i).alpha <= 0.0) { // particle is dead
			frammenti.erase(frammenti.begin() + i);
		}
		else { // particle is alive, thus update
			Frammento[i].x = xPos;
			Frammento[i].y = yPos;
			Frammento[i].r = frammenti.at(i).color.r;
			Frammento[i].g = frammenti.at(i).color.g;
			Frammento[i].b = frammenti.at(i).color.b;
			Frammento[i].a = frammenti.at(i).alpha;
			P_size += 1;
		}
	}
	//tutto fino a qui potremme essere messo in una update (come per disegna_proiettili)

	glBindVertexArray(VAO_FRAMMENTO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_F);
	glBufferData(GL_ARRAY_BUFFER, P_size * sizeof(Pointxy), &Frammento[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glPointSize(frammenti_size);
	glDrawArrays(GL_POINTS, 0, P_size);
	glBindVertexArray(0);

}

//funzione che disegna i proiettili in scena (l'aggiornamento dei proiettili è effettuato in update_proiettili, la creazione di nuovi proiettili è effettuata quando si preme 'spazio').
void disegna_proiettili() {
	glBindVertexArray(VAO_PROIETTILE);
	glBufferData(GL_ARRAY_BUFFER, max_proiettili * sizeof(Pointxy), &Proiettile[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glPointSize(proj_size);
	glDrawArrays(GL_POINTS, 0, proiettili.size());
	glBindVertexArray(0);
}

//funzione che permette la creazione di un proiettile
void spawn_proiettile(float xpos, float ypos, float angle, vec4 col, bool enemy) {
	if (proiettili.size() < max_proiettili) {
		Projectile proiettile;
		proiettile.xpos = xpos;
		proiettile.ypos = ypos;
		proiettile.angle = angle;
		proiettile.col = col;
		proiettile.enemy = enemy;
		proiettili.push_back(proiettile);
	}
}

//funzione che permette la creazione dei frammenti
void spawn_frammenti(float xpos, float ypos, vec4 color, int num_particles) {
	for (int i = 0; i < num_particles; i++) {
		PARTICLE p;
		p.x = xpos;
		p.y = ypos;
		p.alpha = 1.0;
		p.drag = 1.0;
		p.xFactor = (rand() % 1000 + 1) / 300.0 * (rand() % 2 == 0 ? -1 : 1);
		p.yFactor = (rand() % 1000 + 1) / 300.0 * (rand() % 2 == 0 ? -1 : 1);
		p.color.r = color.r;
		p.color.g = color.g;
		p.color.b = color.b;
		// Adds the new element p at the end of the vector, after its current last element
		frammenti.push_back(p);
	}
}

//funzione che spawna le particelle del propulsore
void spawn_propulsore(color color) {
	for (int i = 0; i < 10; i++) {
		PARTICLE p;
		p.x = (float)navicella_posx - (navicella_width / 2)*sin(radians(navicella_moving_angle));
		p.y = (float)navicella_posy - (navicella_height / 2)*cos(radians(navicella_moving_angle));
		p.alpha = 1.0;
		p.drag = 1.05;
		p.xFactor = (rand() % 1000 + 1) / 300.0 * (rand() % 2 == 0 ? -1 : 1);
		p.yFactor = (rand() % 1000 + 1) / 300.0 * (rand() % 2 == 0 ? -1 : 1);
		p.color.r = color.r;
		p.color.g = color.g;
		p.color.b = color.b;
		// Adds the new element p at the end of the vector, after its current last element
		particles.push_back(p);
	}
}


void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader_C_M.glsl";
	char* fragmentShader = (char*)"fragmentShader_C_M.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);
}


void init(void) {
	Projection = ortho(0.0f, float(width), 0.0f, float(height));
	MatProj = glGetUniformLocation(programId, "Projection");
	MatModel = glGetUniformLocation(programId, "Model");

	//Costruzione geometria e colori del CIELO
	vec4 col_top = col_black;
	vec4 col_bottom = col_dark_blue;
	disegna_pianoxy(0, 0, width, height, col_top, col_bottom, Cielo);
	//Generazione del VAO del Cielo
	glGenVertexArrays(1, &VAO_CIELO);
	glBindVertexArray(VAO_CIELO);
	glGenBuffers(1, &VBO_C);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_C);
	glBufferData(GL_ARRAY_BUFFER, vertices_Cielo * sizeof(Pointxy), &Cielo[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Costruzione geometria e colori delle STELLE
	vec4 col_stelle = col_white;
	disegna_stelle(stars_number,width, height,col_stelle, Stelle);
	//Generazione del VAO delle stelle
	glGenVertexArrays(1, &VAO_STELLE);
	glBindVertexArray(VAO_STELLE);
	glGenBuffers(1, &VBO_S);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_S);
	glBufferData(GL_ARRAY_BUFFER, stars_number * sizeof(Pointxy), &Stelle[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Costruzione geometria e colori della NAVICELLA
	vec4 col_navicella = col_white;
	disegna_navicella(navicella_width, navicella_height, col_navicella, Navicella);
	//Generazione del VAO del Navicella
	glGenVertexArrays(1, &VAO_NAVICELLA);
	glBindVertexArray(VAO_NAVICELLA);
	glGenBuffers(1, &VBO_N);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_N);
	glBufferData(GL_ARRAY_BUFFER, vertices_Navicella * sizeof(Pointxy), &Navicella[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Costruzione geometria e colori della ASTEROIDE
	vec4 col_asteroide = col_brown;
	disegna_asteroide(asteroide_width, asteroide_height, col_asteroide, Asteroide);
	//Generazione del VAO dell'asteroide
	glGenVertexArrays(1, &VAO_ASTEROIDE);
	glBindVertexArray(VAO_ASTEROIDE);
	glGenBuffers(1, &VBO_A);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_A);
	glBufferData(GL_ARRAY_BUFFER, vertices_asteroide * sizeof(Pointxy), &Asteroide[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Costruzione geometria e colori dell'ufo
	vec4 col_ufo_top = col_light_grey;
	vec4 col_ufo_bottom = col_grey;
	vec4 col_ufo_alien = col_green;
	disegna_ufo(ufo_width, ufo_height, col_ufo_top, col_ufo_bottom, col_ufo_alien, Ufo);
	//Generazione del VAO dell'ufo
	glGenVertexArrays(1, &VAO_UFO);
	glBindVertexArray(VAO_UFO);
	glGenBuffers(1, &VBO_U);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_U);
	glBufferData(GL_ARRAY_BUFFER, vertices_ufo * sizeof(Pointxy), &Ufo[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Generazione del VAO del Propulsore
	glGenVertexArrays(1, &VAO_PROPULSORE);
	glBindVertexArray(VAO_PROPULSORE);
	glGenBuffers(1, &VBO_Prop);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Prop);
	glBindVertexArray(0);

	//Generazione del VAO dei Frammenti
	glGenVertexArrays(1, &VAO_FRAMMENTO);
	glBindVertexArray(VAO_FRAMMENTO);
	glGenBuffers(1, &VBO_F);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_F);
	glBindVertexArray(0);

	//Generazione del VAO dei proiettili
	glGenVertexArrays(1, &VAO_PROIETTILE);
	glBindVertexArray(VAO_PROIETTILE);
	glGenBuffers(1, &VBO_Proj);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Proj);
	glBindVertexArray(0);
	
	glutSwapBuffers();
}

void drawScene(void) {
	glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
	glClear(GL_COLOR_BUFFER_BIT);

	//Disegna cielo
	Model = mat4(1.0);
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(VAO_CIELO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, vertices_Cielo);
	glBindVertexArray(0);
	
	//Disegna stelle
	Model = mat4(1.0);
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(VAO_STELLE);
	glPointSize(2.0);
	glDrawArrays(GL_POINTS, 0, stars_number);
	glBindVertexArray(0);

	//Disegna navicella
	if (navicella_alive) {
		Model = mat4(1.0);
		Model = translate(Model, vec3(navicella_posx, navicella_posy, 0.0));
		Model = rotate(Model, -radians(navicella_angle), vec3(0, 0, 1));
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
		glBindVertexArray(VAO_NAVICELLA);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, vertices_Navicella);
		glBindVertexArray(0);
	}

	//Disegna asteroidi
	for (int i = 0; i < asteroidi.size(); i++) {
		int asteroide_posx = asteroidi.at(i).xpos;
		int asteroide_posy = asteroidi.at(i).ypos;
		float asteroide_angle = asteroidi.at(i).angle;
		Model = mat4(1.0);
		Model = translate(Model, vec3(asteroide_posx, asteroide_posy, 0.0));
		Model = rotate(Model, radians(asteroide_angle), vec3(0, 0, 1));
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
		glBindVertexArray(VAO_ASTEROIDE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, vertices_asteroide);
		glBindVertexArray(0);
	}

	//Disegna ufo
	for (int i = 0; i < ufos.size(); i++) {
		int ufo_posx = ufos.at(i).xpos;
		int ufo_posy = ufos.at(i).ypos;
		Model = mat4(1.0);
		Model = translate(Model, vec3(ufo_posx, ufo_posy, 0.0));
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
		glBindVertexArray(VAO_UFO);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, vertices_ufo);
		glBindVertexArray(0);
	}

	//Disegna Propulsore
	Model = mat4(1.0);
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	disegna_propulsore();

	//Disegna Frammenti
	Model = mat4(1.0);
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	disegna_frammenti();

	//Disegna proiettili
	Model = mat4(1.0);
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	disegna_proiettili();

	glutSwapBuffers();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutSetOption(GLUT_MULTISAMPLE, 4);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(400, 50);
	glutCreateWindow("Space Invasion");
	glutDisplayFunc(drawScene);

	glutKeyboardFunc(keyboardPressedEvent);
	glutKeyboardUpFunc(keyboardReleasedEvent);

	glutTimerFunc(66, update_navicella, 0);
	glutTimerFunc(66, update_asteroidi, 0);
	glutTimerFunc(66, update_ufos, 0);
	glutTimerFunc(66, update_proiettili, 0);
	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutMainLoop();
}
