/*********
   CTIS164 - Template Source Program
----------
STUDENT : Furkan Adıgüzel
SECTION : 1
HOMEWORK: Chasing and Hitting a Lamp using Trigonometry and Vector Algebra.
----------
PROBLEMS: There is no problem I think.
----------
ADDITIONAL FEATURES:

1) I have an entrance screen which you can choose start or exit.

2) I have loading screen after by clicking the start button.

3) After start the game and play one time at least, if you press F5, the game will be start over.

4) If the arrow come to close of the lamp, the color of arrow will be changed.

5) After hitted the lamp, the lamp color will be changed.

6) I have ON/OFF indicator int he up-left of the screen which is showing the lamp is on or off.

*********/


#include <OpenGL/gl.h>
#include <OpenGl/glu.h>
#include <GLUT/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#define V_D2R 0.0174532
#define V_R2D 57.29608323

typedef struct {
    double x, y;
} vec_t;

typedef struct {
    double magnitude, angle;
} polar_t;

double magV(vec_t v);
double angleV(vec_t v);
vec_t addV(vec_t v1, vec_t v2);
vec_t subV(vec_t v1, vec_t v2);
vec_t mulV(double k, vec_t v);
double dotP(vec_t v1, vec_t v2);
vec_t unitV(vec_t v);
vec_t pol2rec(polar_t p);
polar_t rec2pol(vec_t v);
double angleBetween2V(vec_t v1, vec_t v2);

#include <math.h>

double magV(vec_t v) {
    return sqrt(v.x * v.x + v.y * v.y);
}

double angleV(vec_t v) {
    double angle = atan2(v.y, v.x) * V_R2D;
    return angle < 0 ? angle + 360 : angle;
}

vec_t addV(vec_t v1, vec_t v2) {
    return{ v1.x + v2.x, v1.y + v2.y };
}

vec_t subV(vec_t v1, vec_t v2) {
    return{ v1.x - v2.x, v1.y - v2.y };
}

vec_t mulV(double k, vec_t v) {
    return{ k * v.x, k * v.y };
}

double dotP(vec_t v1, vec_t v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

vec_t unitV(vec_t v) {
    return mulV(1.0 / magV(v), v);
}

// convert from polar representation to rectangular representation
vec_t pol2rec(polar_t p) {
    return{ p.magnitude * cos(p.angle * V_D2R),  p.magnitude * sin(p.angle * V_D2R) };
}

polar_t rec2pol(vec_t v) {
    return{ magV(v), angleV(v) };
}

double angleBetween2V(vec_t v1, vec_t v2) {
    double magV1 = magV(v1);
    double magV2 = magV(v2);
    double dot = dotP(v1, v2);
    double angle = acos(dot / (magV1 * magV2)) * V_R2D; // in degree
    return angle;
}



#define WINDOW_WIDTH 1080
#define WINDOW_HEIGHT 720

#define FPS 60
#define PI 3.1415

#define INIT 0
#define MENU 1
#define LOAD 2
#define RUN 3
#define CREDITS 4
#define OVER 5
int State = INIT; // initial Scene

int winWidth, winHeight, we, he;
int resume, initTime=70 , loadWidth, lampCount;
int mX, mY, k, brTemp = 3;
bool gameState, yellowCol = false;

typedef struct {
    double r, g, b;
}col_t;

typedef struct {
    col_t col[5];
    bool status[5];
}buttons_t;
buttons_t clicks;


typedef struct arrowish_t {
    vec_t pos;
    col_t col[2];
    int angle;
    bool status = false;
};
typedef struct lightSource_t {
    vec_t pos;
    col_t col;
    bool status;
    char light[5];
};
typedef struct {
    vec_t pos, N;
} vert_t;

arrowish_t WEAPONARROW;
lightSource_t shakingLamp;



col_t multiColor(float x, col_t coL) {
    return { x * coL.r, x * coL.g, x * coL.b };
}

col_t addColor(col_t coL1, col_t coL2) {
    return { coL1.r + coL2.r, coL1.g + coL2.g, coL1.b + coL2.b };
}

col_t calColor(lightSource_t lamp, vert_t v) {
    vec_t L = subV(lamp.pos, v.pos), uL = unitV(L);;
    float fact = dotP(uL, v.N) * ((-1 / 350.0) * magV(L) + 1);
    return multiColor(fact, lamp.col);
}

void circle(int x, int y, int r)
{
    float angle;
    glBegin(GL_POLYGON);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void circle_wire(int x, int y, int r)
{
    float angle;
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void vprint(int x, int y, void* font, const char* string, ...)
{
    va_list ap;
    va_start(ap, string);
    char str[1024];
    va_end(ap);

    int len, i;
    glRasterPos2f(x, y);
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, str[i]);
    }
}

void vprint2(int x, int y, float size, const char* string, ...) {
    va_list ap;
    va_start(ap, string);
    char str[1024];
    va_end(ap);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(size, size, 1);

    int len, i;
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
    }
    glPopMatrix();
}

void Init_Globals() {

    resume = 1;
    gameState = WEAPONARROW.status = false;
    k = lampCount = loadWidth = 0;

    shakingLamp = { { 0, 0 }, {0,0,0}, false, {0} };
    clicks = { {{0,0,255},{0,0,255}, {0,0,255}, { 100,100,100 } }, {{false},{false},{false},{false} } };


    winWidth = we;
    winHeight = he;
    glViewport(0, 0, we, he);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-we / 2, we / 2, -he / 2, he / 2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void LoadingFunc() {

    glClearColor(15.0 / 255.0, 15.0 / 255.0, 15.0 / 255.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glLineWidth(5);
    glColor3f(0, 1, 0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-400, 0);
    glVertex2f(400, 0);
    glVertex2f(400, -100);
    glVertex2f(-400, -100);
    glEnd();

    glColor3f(0, 0, 1);
    glRectf(-400, 0, -400 + loadWidth, -100); //loading part

    if (loadWidth <= 810)
        loadWidth += 10;

}

void checkButtons() {

    if (State == MENU) {
        if (mX > -70 && mX < 70 && mY > 90 && mY < 150) {
            clicks.col[0] = { 100, 100, 255 };  clicks.status[0] = true;
        }  // START BUTTON
        else
        {
            clicks.col[0] = { 0, 0, 255 };     clicks.status[0] = false;
        }

        

        if (mX > -70 && mX < 70 && mY > -95 && mY < -35)
        {
            clicks.col[2] = { 100, 100, 255 };     clicks.status[2] = true;
        }    // EXIT BUTTON
        else
        {
            clicks.col[2] = { 0, 0, 255 };     clicks.status[2] = false;
        }
    }

    if (State == OVER || State == CREDITS) { //BACK BUTTON

        if (mX > 420 && mX < 500 && mY > -300 && mY < -250)
        {
            clicks.col[3] = { 150, 150, 150 };        clicks.status[3] = true;
        }
        else { clicks.col[3] = { 100, 100, 100 };    clicks.status[3] = false; }

        if (State == OVER) { //RETRY BUTTON
            if (mX > -70 && mX < 70 && mY > -130 && mY < -70)
            {
                clicks.col[4] = { 50, 255, 50 };    clicks.status[4] = true;
            }
            else
            {
                clicks.col[4] = { 0, 200, 0 };    clicks.status[4] = false;
            }
        }
    }

}



void MenuFunc() {
    checkButtons();
    glClearColor(100.0 / 255.0, 32.0 / 255.0, 32.0 / 255.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    

    glColor3ub(255, 100, 0);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP); //nameSurname Box
    glVertex2f(-(winWidth / 2) + 10, (winHeight / 2) - 5);
    glVertex2f(-(winWidth / 2) + 200, (winHeight / 2) - 5);
    glVertex2f(-(winWidth / 2) + 200, (winHeight / 2) - 50);
    glVertex2f(-(winWidth / 2) + 10, (winHeight / 2) - 50);
    glEnd();
    glColor3ub(100, 250, 0);
    vprint2(-510,325 , 0.15, "Furkan Adiguzel");

    glColor3f(1, 1, 1);
    glLineWidth(3);
    vprint2(-140, 220, 0.3, "Breaking Lamp");

    
    //BUTTONS on Menu
    glColor3ub(clicks.col[0].r, clicks.col[0].g, clicks.col[0].b);
    glRectf(-70, 150, 70, 90);
    glColor3ub(clicks.col[2].r, clicks.col[2].g, clicks.col[2].b);
    glRectf(-70, -35, 70, -95);

    glLineWidth(3);
    glColor3f(0, 0, 0);
    vprint2(-45, 105, 0.25, "START");
    vprint2(-27, -75, 0.25, "EXIT");

}



void OverFunc() {

    glClearColor(55.0 / 255.0, 225.0 / 255.0, 110.0 / 255.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3ub(0, 0, 50);
    glRectf(-540, 100, 540, -50);
    glColor3f(0.75, 0.75, 0.75);
    glLineWidth(10);
    glBegin(GL_LINES);
    glVertex2f(-200, 100);
    glVertex2f(-200, -50);
    glVertex2f(200, 100);
    glVertex2f(200, -50);
    glEnd();

}

void infoFunc() {
    glColor3f(0, 0, 0);
    glRectf(-470, 360, -540, 310);

    glColor3f(1, 1, 1); //whiteNum
    if (yellowCol)
        glColor3f(1, 1, 0); // yellowNum

    glColor3f(1,0 , 0); //RED -OFF
    if (strcmp(shakingLamp.light, "ON") == 0)
        glColor3f(0,0 , 1); // BLUE -ON
    vprint2(-530, 322, 0.225, "%s", shakingLamp.light);

    glColor3f(1, 1, 1);
    vprint((winWidth / 2) - 1050, -(winHeight / 2) + 115, GLUT_BITMAP_HELVETICA_10, "<Right Mouse Click>           :  switch on");
    vprint((winWidth / 2) - 1050, -(winHeight / 2) + 100, GLUT_BITMAP_HELVETICA_10, "<Left Mouse Click>             :  spawn arrow");
    vprint((winWidth / 2) - 1050, -(winHeight / 2) + 85, GLUT_BITMAP_HELVETICA_10, "<F5 Button>                       :  retry");
}


void draw_circleLamp() {

    glColor3f(0.1, 0.1, 0.1);
    glLineWidth(5);
    glBegin(GL_LINES);
    glVertex2f(shakingLamp.pos.x, shakingLamp.pos.y - 15);
    glVertex2f(0, 320);
    glEnd();

    glColor3f(shakingLamp.col.r, shakingLamp.col.g, shakingLamp.col.b); // WHITE-YELLOW
    circle(shakingLamp.pos.x, shakingLamp.pos.y -15, 15);

}

void draw_weaponArrow() {

    glColor3f(WEAPONARROW.col[0].r, WEAPONARROW.col[0].g, WEAPONARROW.col[0].b);// GREEN - ORANGE
    glBegin(GL_QUADS);
    glVertex2f(WEAPONARROW.pos.x + 60 * cos((WEAPONARROW.angle + 360) * (PI / 180)), WEAPONARROW.pos.y + 60 * sin((WEAPONARROW.angle + 360) * (PI / 180)));
    glVertex2f(WEAPONARROW.pos.x + 8 * cos((WEAPONARROW.angle + 260) * (PI / 180)), WEAPONARROW.pos.y + 8 * sin((WEAPONARROW.angle + 260) * (PI / 180)));
    glVertex2f(WEAPONARROW.pos.x + 12 * cos((WEAPONARROW.angle + 180) * (PI / 180)), WEAPONARROW.pos.y + 12 * sin((WEAPONARROW.angle + 180) * (PI / 180)));
    glVertex2f(WEAPONARROW.pos.x + 8 * cos((WEAPONARROW.angle + 100) * (PI / 180)), WEAPONARROW.pos.y + 8 * sin((WEAPONARROW.angle + 100) * (PI / 180)));
    glEnd();

    glLineWidth(4);
    glColor3f(WEAPONARROW.col[1].r, WEAPONARROW.col[1].g, WEAPONARROW.col[1].b); // GREEN/BLACK - ORANGE/WHITE
    glBegin(GL_LINE_LOOP);
    glVertex2f(WEAPONARROW.pos.x + 60 * cos((WEAPONARROW.angle + 360) * (PI / 180)), WEAPONARROW.pos.y + 60 * sin((WEAPONARROW.angle + 360) * (PI / 180)));
    glVertex2f(WEAPONARROW.pos.x + 8 * cos((WEAPONARROW.angle + 260) * (PI / 180)), WEAPONARROW.pos.y + 8 * sin((WEAPONARROW.angle + 260) * (PI / 180)));
    glVertex2f(WEAPONARROW.pos.x + 17 * cos((WEAPONARROW.angle + 180) * (PI / 180)), WEAPONARROW.pos.y + 17 * sin((WEAPONARROW.angle + 180) * (PI / 180)));
    glVertex2f(WEAPONARROW.pos.x + 8 * cos((WEAPONARROW.angle + 100) * (PI / 180)), WEAPONARROW.pos.y + 8 * sin((WEAPONARROW.angle + 100) * (PI / 180)));
    glEnd();

}


void RunFunc() {

    glClearColor(0 / 255.0, 0 / 255.0, 100.0 / 255.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    
    draw_circleLamp();

    if (WEAPONARROW.status) // weapon will not be shown without pressing MOUSE1,
        draw_weaponArrow();
    
    infoFunc(); //Left bottom

}

void display() {

    checkButtons();

    switch (State) {
    case MENU:    MenuFunc(); break;
    case LOAD:    LoadingFunc(); break;
    case RUN:     RunFunc(); break;
    case OVER:    OverFunc(); break;
    }

    glutSwapBuffers();
}

void onKeyDown(unsigned char key, int x, int y)
{
    if (key == 27 && State == RUN && resume && gameState)
    {
        State = OVER;
        gameState = false; resume = 0;
    }
    if (key == ' ' && State == RUN && resume && gameState && shakingLamp.status)
        yellowCol = !yellowCol; // changes light color

}

void onSpecialKeyDown(int key, int x, int y)
{
    // pause/resume and retry buttons
    if (key == GLUT_KEY_F5 && State == RUN && resume && gameState)
        State = LOAD;
    

}

void onClick(int button, int stat, int x, int y)
{
    int mouX = x - winWidth / 2, mouY = winHeight / 2 - y;

    if (button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN)
    {
        if (State == MENU) {
            if (clicks.status[0] == true) // START
            {
                Init_Globals(); State = LOAD;
            }
            else if (clicks.status[2] == true) // EXIT
                exit(0);
        }

        if (State == CREDITS || State == OVER)
        {
            if (clicks.status[3] == true)  // EXIT
            {
                Init_Globals(); State = MENU;
            }
            if (State == OVER && clicks.status[4] == true)  //RETRY
                State = LOAD;
        }

    }

    if (button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN && State == RUN && !WEAPONARROW.status && gameState && resume && shakingLamp.status)
    { //spawn Arrow
        WEAPONARROW.pos.x = mouX;
        WEAPONARROW.pos.y = mouY;
        WEAPONARROW.status = true;
        WEAPONARROW.col[0] = { 0,0.7,0 };
        WEAPONARROW.col[1] = { 0,0,0 };
    }

    if (button == GLUT_RIGHT_BUTTON && stat == GLUT_DOWN && resume && State == RUN)
    { //switch on the light
        if (!gameState) { //initial switching
            gameState = true; shakingLamp.status = true;
            if (yellowCol)
                shakingLamp.col = { 1,1,0 };
            else
                shakingLamp.col = { 1,1,1 };
        }
        if (!shakingLamp.status) { // switching after the hit
            gameState = true; shakingLamp.status = true;
            if (yellowCol)
                shakingLamp.col = { 1,1,0 };
            else
                shakingLamp.col = { 1,1,1 };
        }
    }
}

void onMove(int x, int y) { //OPENGL Position

    mX = x - winWidth / 2;
    mY = winHeight / 2 - y;

    glutPostRedisplay();
}

void gameCalculations() {

    if (gameState) {
        WEAPONARROW.angle = atan2((shakingLamp.pos.y - 15 - WEAPONARROW.pos.y), (shakingLamp.pos.x - WEAPONARROW.pos.x)) / (PI / 180); // find the correct angle between arrow and lamp

        //arrow pace and direction calculation.
        WEAPONARROW.pos.x += 4 * cos((WEAPONARROW.angle) * (PI / 180));
        WEAPONARROW.pos.y += 4 * sin((WEAPONARROW.angle) * (PI / 180));

        //Circle lamp direction and pace calculation.
        float temp = ((sin((PI / 180) * k) * 52 + 270) * (PI / 180)); //( 270 = default )
        shakingLamp.pos.x = cos(temp) * 270.0;
        shakingLamp.pos.y = 270.0 + (sin(temp) * 270.0);
        //To prevent finite numbers
        if (shakingLamp.pos.x >= 275.75)
            k = 91;
        else
            k++;
    }
}

void collision() {
    int hitX = WEAPONARROW.pos.x - 15 * cos((360 + WEAPONARROW.angle) * (PI / 180)), hitY = WEAPONARROW.pos.y - 15 * sin((360 + WEAPONARROW.angle) * (PI / 180)); // firstHit

    if (WEAPONARROW.status) {
        if (WEAPONARROW.pos.x > shakingLamp.pos.x - 15 && WEAPONARROW.pos.x < shakingLamp.pos.x + 15 && WEAPONARROW.pos.y < shakingLamp.pos.y + 15 && WEAPONARROW.pos.y > shakingLamp.pos.y - 15) { //broken lamp
            shakingLamp.col = { 0,0,0 };
            shakingLamp.status = false;
            WEAPONARROW.status = false;
            lampCount++;
        }
        if (hitX > shakingLamp.pos.x - 60 && hitX < shakingLamp.pos.x + 60 && hitY < shakingLamp.pos.y + 60 && hitY > shakingLamp.pos.y - 60) { // orangeArrow
            WEAPONARROW.col[0] = { 1,05,0 };
            WEAPONARROW.col[1] = { 0.85,0.85,0.85 };
        }
    }

}



void onTimer(int v) {

    glutTimerFunc((1000 / FPS), onTimer, 0);

    if (State == INIT) {
        initTime++;
        if (initTime >= 75)
            State = MENU;
    }

    if (State == LOAD) {
        if (loadWidth >= 810) { //LoadingTime
            Init_Globals();    State = RUN;
        }

    }

    if (resume) {
        gameCalculations();
        collision();
    }

    glutPostRedisplay();
}

void onResize(int w, int h)
{
    we = w; he = h;
    winWidth = w;
    winHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    display();
}

void Init() {

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Init_Globals();
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Breaking Lamp | Furkan Adiguzel");

    glutDisplayFunc(display);
    glutReshapeFunc(onResize);

    glutKeyboardFunc(onKeyDown);
    glutSpecialFunc(onSpecialKeyDown);

    glutMouseFunc(onClick);
    glutPassiveMotionFunc(onMove);

    glutTimerFunc((1000 / FPS), onTimer, 0);

    Init();

    glutMainLoop();
    return 0;
}
