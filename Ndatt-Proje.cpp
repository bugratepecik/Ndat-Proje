#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <gl/glew.h>
#include <gl/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <iostream>
using namespace std;
#define PENCERE_GENISLIGI 800
#define PENCERE_YUKSEKLIGI 800
#define Y_ESIGI 0
#define X_ESIGI 200
#define NEXT_OBJECT_Y 350
#define NESNE_YUKSEKLIGI 25
#define BASLANGIC_GENISLIGI 150
#define MAX_NESNE_SAYISI ((Y_ESIGI + PENCERE_GENISLIGI / 2) / NESNE_YUKSEKLIGI)
#define FEEDBACK_SURESI 50

#define PERIYOD 25
#define ZAMANLAYICI 1

#define D2R 0.0174532

typedef struct
{
    float x, y;
} point_t;

typedef struct
{
    float r, g, b;
} color_t;

typedef struct
{
    point_t koordinat;
    color_t renk;
    float boyut;
} object_t;

typedef struct
{
    point_t koordinat;
    point_t hiz;
    float yon;
    color_t renk;
    float boyut;
} overflowed_object_t;

typedef struct
{
    float x,
        boyut;
    color_t renk;
} static_object_t;

enum GameState
{
    Running,
    ObjectMoving,
    Animating,
    GameOver
};

bool yukari = false, asagi = false, sag = false, sol = false;
int winWidth, winHeight;
float lastSucceedSize;
static_object_t objects[MAX_NESNE_SAYISI];
object_t nextObject;
overflowed_object_t overflowedObject;
int objectCount;
GameState gameState;

int firstObjectIndex;
float speed;
float currentThreshold;
int score;
bool isFeedBackActive;
int feedBackTimer,
timer;
int feedback;
float aci = 0.0f;
float kaydirX = 0.0f;
float kaydirY = 0.0f;

void kupCiz()
{

    GLfloat renkler[6][3] = {
        {0.5f, 0.5f, 1.0f},
        {0.8f, 0.2f, 0.5f},
        {0.2f, 0.8f, 0.2f},
        {1.0f, 0.7f, 0.3f},
        {0.6f, 0.4f, 1.0f},
        {0.8f, 0.5f, 1.0f} };

    glBegin(GL_QUADS);

    for (int i = 0; i < 6; i++)
    {
        glColor3fv(renkler[i]);
        switch (i)
        {
        case 0:
            glVertex3f(-0.5f, -0.5f, 0.5f);
            glVertex3f(0.5f, -0.5f, 0.5f);
            glVertex3f(0.5f, 0.5f, 0.5f);
            glVertex3f(-0.5f, 0.5f, 0.5f);
            break;
        case 1:
            glVertex3f(-0.5f, -0.5f, -0.5f);
            glVertex3f(-0.5f, -0.5f, 0.5f);
            glVertex3f(-0.5f, 0.5f, 0.5f);
            glVertex3f(-0.5f, 0.5f, -0.5f);
            break;
        case 2:
            glVertex3f(0.5f, -0.5f, -0.5f);
            glVertex3f(-0.5f, -0.5f, -0.5f);
            glVertex3f(-0.5f, 0.5f, -0.5f);
            glVertex3f(0.5f, 0.5f, -0.5f);
            break;
        case 3:
            glVertex3f(0.5f, -0.5f, 0.5f);
            glVertex3f(0.5f, -0.5f, -0.5f);
            glVertex3f(0.5f, 0.5f, -0.5f);
            glVertex3f(0.5f, 0.5f, 0.5f);
            break;
        case 4:
            glVertex3f(-0.5f, 0.5f, 0.5f);
            glVertex3f(0.5f, 0.5f, 0.5f);
            glVertex3f(0.5f, 0.5f, -0.5f);
            glVertex3f(-0.5f, 0.5f, -0.5f);
            break;
        case 5:
            glVertex3f(-0.5f, -0.5f, -0.5f);
            glVertex3f(0.5f, -0.5f, -0.5f);
            glVertex3f(0.5f, -0.5f, 0.5f);
            glVertex3f(-0.5f, -0.5f, 0.5f);
            break;
        }
    }

    glEnd();
}

void ekranCiz()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(kaydirX, kaydirY, -5.0f);
    glRotatef(aci, 1.0f, 1.0f, 1.0f);
    kupCiz();

    glutSwapBuffers();
}

void yenidenBoyutlandir(int genislik, int yukseklik)
{
    glViewport(0, 0, genislik, yukseklik);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)genislik / (float)yukseklik, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

void klavyeTus(int tus, int x, int y)
{
    switch (tus)
    {
    case GLUT_KEY_LEFT:
        aci = aci - 5.0f;
        break;
    case GLUT_KEY_RIGHT:
        aci = aci + 5.0f;
        break;
    case GLUT_KEY_UP:
        kaydirY = kaydirY + 0.1f;
        break;
    case GLUT_KEY_DOWN:
        kaydirY = kaydirY - 0.1f;
        break;
    }

    glutPostRedisplay();
}
void initializeGlobals()
{
    gameState = GameOver;
    timer = 0;
    score = 0;
    speed = 5;
    firstObjectIndex = 0;
    objectCount = MAX_NESNE_SAYISI;

    for (int i = 0; i < objectCount; i++)
    {
        objects[i].x = -BASLANGIC_GENISLIGI / 2;
        objects[i].boyut = BASLANGIC_GENISLIGI;
        objects[i].renk = { 0.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f };
    }

    nextObject.koordinat.x = -BASLANGIC_GENISLIGI / 2;
    nextObject.koordinat.y = NEXT_OBJECT_Y;
    nextObject.boyut = lastSucceedSize = BASLANGIC_GENISLIGI;
    nextObject.renk = { 0.0f, (rand() % 100) / 100.0f, 0.0f };

    currentThreshold = NESNE_YUKSEKLIGI - PENCERE_YUKSEKLIGI / 2;
}

//  (x,y) merkezinde daire çiz.
// yaricap r

void circle(int x, int y, int r)
{
#define PI 3.1415
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
#define PI 3.1415
    float angle;

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void print(int x, int y, const char* string, void* font)
{
    int len, i;

    glRasterPos2f(x, y);
    len = (int)strlen(string);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, string[i]);
    }
}

// degiskenle yaziyi goruntule.
// vprint(-winWidth / 2 + 10, winHeight / 2 - 20, GLUT_BITMAP_8_BY_13, "ERROR: %d", numClicks);
void vprint(int x, int y, void* font, const char* string, ...)
{
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);

    int len, i;
    glRasterPos2f(x, y);
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, str[i]);
    }
}

void vprint2(int x, int y, float size, const char* string, ...)
{
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
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

void rectBorder(float x1, float y1, float x2, float y2)
{
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y1);
    glVertex2f(x1, y2);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void displayStaticObjects()
{
    int j;
    for (int i = 0; i < MAX_NESNE_SAYISI; i++)
    {
        j = (firstObjectIndex + i) % objectCount;
        glColor4f(objects[j].renk.r, objects[j].renk.g, objects[j].renk.b, 0.5);
        glRectf(objects[j].x, i * NESNE_YUKSEKLIGI - PENCERE_YUKSEKLIGI / 2,
            objects[j].x + objects[j].boyut, (i + 1) * NESNE_YUKSEKLIGI - PENCERE_YUKSEKLIGI / 2);
        glColor4f(1 - objects[j].renk.r, 1 - objects[j].renk.g, 1 - objects[j].renk.b, 0.5);
        rectBorder(objects[j].x, i * NESNE_YUKSEKLIGI - PENCERE_YUKSEKLIGI / 2,
            objects[j].x + objects[j].boyut, (i + 1) * NESNE_YUKSEKLIGI - PENCERE_YUKSEKLIGI / 2);
    }
}

void displayPlayerObject()
{
    glColor4f(nextObject.renk.r, nextObject.renk.g, nextObject.renk.b, 0.5);

    glRectf(nextObject.koordinat.x, nextObject.koordinat.y,
        nextObject.koordinat.x + nextObject.boyut,
        nextObject.koordinat.y + NESNE_YUKSEKLIGI);

    glColor3f(0, 0, 0);
    rectBorder(nextObject.koordinat.x, nextObject.koordinat.y,
        nextObject.koordinat.x + nextObject.boyut,
        nextObject.koordinat.y + NESNE_YUKSEKLIGI);

    glBegin(GL_LINES);
    glColor4f(1, 1, 1, 0.2);
    glVertex2f(nextObject.koordinat.x, nextObject.koordinat.y + NESNE_YUKSEKLIGI);
    glColor4f(1, 1, 1, 0);
    glVertex2f(nextObject.koordinat.x, 0);
    glColor4f(0.0f, 0.0f, 1.0f, 0.5);
    glVertex2f(nextObject.koordinat.x + nextObject.boyut, nextObject.koordinat.y + NESNE_YUKSEKLIGI);
    glColor4f(1, 1, 1, 0);
    glVertex2f(nextObject.koordinat.x + nextObject.boyut, 0);
    glEnd();
}

void displayScore()
{
    // Skorun konumunu üst kýsma taþý
    // Skorun rengi beyaz olarak ayarlandý
    glColor3f(1, 1, 1);
    // vprint fonksiyonunun ilk iki parametresi (x, y) skorun ekrandaki konumunu belirler.
    // Ýlk parametre x (yatay konum) olarak -350 kullanýldý, böylece skor ekranýn sol üst köþesine yakýn görüntülenir.
    // Ýkinci parametre y (dikey konum) olarak 350 kullanýldý, böylece skor ekranýn üst kýsmýnda görüntülenir.
    vprint(-350, 350, GLUT_BITMAP_TIMES_ROMAN_24, "SKOR: %d", score);

    // Skorun geribildirim (feedback) mesajlarýný ekranda görüntülemek için kodlar burada yer alýyor
    if (isFeedBackActive)
    {
        switch (feedback)
        {
        case 5:
            glColor3f(1, 0, 0);
            vprint(-250, 350, GLUT_BITMAP_TIMES_ROMAN_24, "HARIKA : 5 PUAN");
            break;
        case 4:
            glColor3f(1, 0, 0);
            vprint(-250, 350, GLUT_BITMAP_TIMES_ROMAN_24, "IYI : 4 PUAN");
            break;
        case 3:
            glColor3f(1, 0, 0);
            vprint(-250, 350, GLUT_BITMAP_TIMES_ROMAN_24, "EH ISTE : 3 PUAN");
            break;
        case 2:
            glColor3f(1, 0, 0);
            vprint(-180, 350, GLUT_BITMAP_TIMES_ROMAN_24, "2 PUAN");
            break;
        case 1:
            glColor3f(1, 1, 1);
            vprint(-180, 350, GLUT_BITMAP_TIMES_ROMAN_24, "1 PUAN");
            break;
        default:
            break;
        }
    }
}

void displayOverflowingPart()
{
    if (gameState == Animating)
    {
        glColor3f(overflowedObject.renk.r, overflowedObject.renk.g, overflowedObject.renk.b);
        glTranslatef(overflowedObject.koordinat.x + overflowedObject.boyut / 2,
            overflowedObject.koordinat.y - NESNE_YUKSEKLIGI / 2,
            0);
        glRotatef(overflowedObject.yon, 0, 0, 1);
        glRectf(-overflowedObject.boyut / 2, NESNE_YUKSEKLIGI / 2, overflowedObject.boyut / 2, -NESNE_YUKSEKLIGI / 2);
        glLoadIdentity();
    }
}

void displayStartScreen()
{
    glColor4f(0, 0, 0, 0.7);
    glRectf(-PENCERE_GENISLIGI / 2, PENCERE_YUKSEKLIGI / 2, PENCERE_GENISLIGI / 2, -PENCERE_YUKSEKLIGI / 2);
    glColor3f(1, 1, 1);
    vprint2(-200, 0, 0.5, "F1	:	BASLAT");
    vprint2(-250, -100, 0.3, "SPACE	:	NESNE GONDER");
    if (score != 0)
        vprint2(-200, -200, 0.2, "SON SKOR: %d", score);
}

void GameDisplay()
{

    // ekraný temizle

    glClearColor(0.2, 0.2, 0.2, 0.7);
    glClear(GL_COLOR_BUFFER_BIT);

    displayStaticObjects();
    if (gameState != Animating)
        displayPlayerObject();
    else
        displayOverflowingPart();
    displayScore();
    if (gameState == GameOver)
        displayStartScreen();

    glutSwapBuffers();
}

void onKeyDown(unsigned char key, int x, int y)
{
    // çýkmak icin esc
    if (key == 27)
        exit(0);
    if (gameState == Running && key == ' ')
        gameState = ObjectMoving;

    // ekrani yeniler
    glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y)
{
    // çýkmak için esc
    if (key == 27)
        exit(0);

    // ekrani yeniler
    glutPostRedisplay();
}

void onSpecialKeyDown(int key, int x, int y)
{
    if (key == GLUT_KEY_F1)
    {
        initializeGlobals();
        gameState = Running;
    }
    // ekrani yeniler
    glutPostRedisplay();
}

void onSpecialKeyUp(int key, int x, int y)
{

    switch (key)
    {
    case GLUT_KEY_UP:
        yukari = false;
        break;
    case GLUT_KEY_DOWN:
        asagi = false;
        break;
    case GLUT_KEY_LEFT:
        sol = false;
        break;
    case GLUT_KEY_RIGHT:
        sag = false;
        break;
    }

    // ekraný yenile
    glutPostRedisplay();
}

void onClick(int button, int stat, int x, int y)
{

    glutPostRedisplay();
}

void onResize(int w, int h)
{
    winWidth = w;
    winHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    GameDisplay();
}

void onMoveDown(int x, int y)
{

    glutPostRedisplay();
}

void onMove(int x, int y)
{

    glutPostRedisplay();
}

void scoreHandler(float dif)
{

    float percent = dif / lastSucceedSize;
    if (dif == 0)
        feedback = 5;
    else if (percent < 0.10)
        feedback = 4;
    else if (percent < 0.20)
        feedback = 3;
    else if (percent < 0.30)
        feedback = 2;
    else
        feedback = 1;
    score += feedback;
    feedBackTimer = timer;
    isFeedBackActive = true;
}

#if ZAMANLAYICI == 1
void onTimer(int v)
{

    glutTimerFunc(PERIYOD, onTimer, 0);

    timer++;
    if (isFeedBackActive && feedBackTimer + FEEDBACK_SURESI < timer)
    {
        isFeedBackActive = false;
    }

    if (gameState == Running)
    {
        nextObject.koordinat.x += speed;
        if (nextObject.koordinat.x + nextObject.boyut >= X_ESIGI || nextObject.koordinat.x <= -X_ESIGI)
            speed *= -1;
    }
    else if (gameState == ObjectMoving)
    {
        if (!(nextObject.koordinat.y <= 0))
            nextObject.koordinat.y -= NESNE_YUKSEKLIGI;
        else
        {

            int lastIndex = (firstObjectIndex - 1 + objectCount) % objectCount;

            if (nextObject.koordinat.x + nextObject.boyut < objects[lastIndex].x || nextObject.koordinat.x > objects[lastIndex].x + objects[lastIndex].boyut)
                gameState = GameOver;
            else
            {

                float difference = fabs(objects[lastIndex].x - nextObject.koordinat.x);
                scoreHandler(difference);
                lastSucceedSize -= difference;

                overflowedObject.boyut = difference;
                overflowedObject.renk = nextObject.renk;

                if (nextObject.koordinat.x < objects[lastIndex].x)
                {
                    overflowedObject.koordinat.x = nextObject.koordinat.x;
                    overflowedObject.hiz = { -3, 5 };
                }

                else
                {
                    overflowedObject.koordinat.x = objects[lastIndex].x + objects[lastIndex].boyut;
                    overflowedObject.hiz = { 3, 5 };
                }
                overflowedObject.koordinat.y = 0;
                overflowedObject.yon = 0;

                objects[firstObjectIndex].boyut = lastSucceedSize;
                objects[firstObjectIndex].x = fmax(objects[lastIndex].x, nextObject.koordinat.x);
                objects[firstObjectIndex].renk = nextObject.renk;

                firstObjectIndex = (firstObjectIndex + 1) % objectCount;

                nextObject.koordinat.x = -BASLANGIC_GENISLIGI / 2;
                nextObject.koordinat.y = NEXT_OBJECT_Y;
                nextObject.boyut = lastSucceedSize;
                nextObject.renk = { (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f };
                gameState = Animating;
            }
        }
    }
    else if (gameState == Animating)
    {
        overflowedObject.hiz.y -= 0.5;
        overflowedObject.yon += 5 * overflowedObject.hiz.x;

        overflowedObject.koordinat.y += overflowedObject.hiz.y;
        overflowedObject.koordinat.x += overflowedObject.hiz.x;
        if (overflowedObject.koordinat.y < -PENCERE_YUKSEKLIGI / 2)
            gameState = Running;
    }

    glutPostRedisplay();
}
#endif

void Init()
{

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//
// animasyon
//

float rotationAngle = 0.3f;
float translationDistance = 0.5f;
int windowWidth = 800;
int windowHeight = 600;
bool gameStarted = false;

void mouseClick(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {

        gameStarted = true;
    }
}

void update(int value)
{
    // Küp döndürme açýsýný güncelle
    rotationAngle += 1.0f;

    // Küp öteleme mesafesini güncelle
    if (gameStarted)
    {
        translationDistance += 0.01f;
    }

    // Ekraný güncelle
    glutPostRedisplay();

    // Bir sonraki güncellemeyi planla
    glutTimerFunc(16, update, 0);
}

void drawStartScreen()
{
    // Ekraný temizle
    glClear(GL_COLOR_BUFFER_BIT);

    // Baþlangýç ekranýný çiz
    glColor3f(1.0f, 1.0f, 1.0f); // Yazý rengi: beyaz
    glColor3f(1.0, 1.0, 1.0);    // Beyaz renk

    glutSwapBuffers();
}
const GLfloat light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

void main(int argc, char* argv[])
{

    cout << "istediginiz secenegi secin..." << endl;
    cout << "1--> brick stack" << endl;
    cout << "	F1 --> BASLAT  |---|  SPACE --> NESNE GONDER 	" << endl;
    cout << "2--> animasyon" << endl;
    cout << "	Yukari ve Asagi ok tuslarý ile kupu oteleyin veya sag ve sol ok tuslari ile kupu dondurun	" << endl;
    cout << "3--> Hazirlayanlar" << endl;
    cout << endl;
    cout << "cikis yapmak icin diger tuslardan birine basin" << endl;

    int secim;
    cin >> secim;
    cout << "yapilcan secim : " << secim << endl;

    if (secim == 1)
    {
        srand(time(NULL));
        initializeGlobals();
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
        glutInitWindowSize(PENCERE_GENISLIGI, PENCERE_YUKSEKLIGI);

        glutCreateWindow("BRICK STACK");

        glutDisplayFunc(GameDisplay);
        glutReshapeFunc(onResize);

        glutKeyboardFunc(onKeyDown);
        glutSpecialFunc(onSpecialKeyDown);

        glutKeyboardUpFunc(onKeyUp);
        glutSpecialUpFunc(onSpecialKeyUp);

        glutMouseFunc(onClick);
        glutMotionFunc(onMoveDown);
        glutPassiveMotionFunc(onMove);

#if ZAMANLAYICI == 1

        glutTimerFunc(PERIYOD, onTimer, 0);
#endif

        Init();

        glutMainLoop();
    }

    else if (secim == 2)
    {

        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
        glutInitWindowSize(800, 600);
        glutCreateWindow("Renkli Kup");

        glEnable(GL_DEPTH_TEST);
        glutDisplayFunc(ekranCiz);
        glutReshapeFunc(yenidenBoyutlandir);
        glutSpecialFunc(klavyeTus);

        glutMainLoop();
    }
    else if (secim == 3) {
        cout << "Bugra Duha Tepecik 210401089" << endl;
        cout << "Gokhan Ciftan 200401039" << endl;
        cout << "Omer Turkay 200401062" << endl;
        cout << "Semanur Gungor 210401023" << endl;  
    
    }
    else{
        exit(0);
    }
}