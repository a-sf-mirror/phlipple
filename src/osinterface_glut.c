/*
 * Phlipple
 * Copyright (C) Remigiusz Dybka 2011 <remigiusz.dybka@gmail.com>
 *
 Phlipple is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Phlipple is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef WIN32
#include <windows.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "osinterface.h"
#include "GL/glew.h"
#include <GL/glut.h>

void keyboard(unsigned char key, int x, int y);
void special(int key, int x, int y);
void mouse(int butt, int state, int x, int y);

void display(void);
void reshape(int width, int height);
void gameLoop();

static int recordVideo = 0;
static GLbyte *buff = 0;
FILE *videoFile = 0;
static int width;
static int height;

double ltime;
double timenow;
double frameTime;

int hasQuit = 0;

double getNanoTime()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (double) tv.tv_sec + ((double) tv.tv_usec / 1000000.0L);
}

SceneEvents *evts;

void OS_init(int *argc, char **argv)
{
	frameTime = 1.0 / 60.0;
	evts = SceneManager_eventsInit();
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA);
	glutInitWindowSize(960, 640);
	glutCreateWindow("Phlipple");
	glutReshapeFunc(&reshape);
	glutKeyboardFunc(&keyboard);
	glutMouseFunc(&mouse);
	glutDisplayFunc(&display);
	glutIdleFunc(&gameLoop);
	glutSpecialFunc(&special);
}

int OS_mainLoop()
{
	GLenum err = glewInit();
	if (err == GLEW_OK)
	{
		SceneManager_resume();
		glutMainLoop();
	}

	return EXIT_SUCCESS;
}

void OS_quit()
{
	SceneManager_destroy();
	SceneManager_eventsDestroy(evts);
	hasQuit = 1;
	glutLeaveMainLoop();
}

void mouse(int butt, int state, int x, int y)
{
	if (butt != GLUT_LEFT_BUTTON)
		return;

	if (state == GLUT_DOWN)
	{
		SceneManager_eventsAdd(evts, SCENEEVENT_TYPE_FINGERDOWN, 0, x, y);
	}

	if (state == GLUT_UP)
	{
		SceneManager_eventsAdd(evts, SCENEEVENT_TYPE_FINGERUP, 0, x, y);
	}

}

void keyboard(unsigned char key, int x, int y)
{
	SceneManager_eventsAdd(evts, SCENEEVENT_TYPE_KEYDOWN, key, x, y);
}

void special(int key, int x, int y)
{
	if (key == 11)
	{
		if (videoFile == 0)
			videoFile = fopen("video.raw", "ab");
		recordVideo = 1;
	}

	if (key == 12)
	{
		if (videoFile)
		{
			fclose(videoFile);
			videoFile = 0;
		}

		recordVideo = 0;
	}

	SceneManager_eventsAdd(evts, SCENEEVENT_TYPE_KEYDOWN, -key, x, y);
}

void reshape(int _width, int _height)
{
	width = _width;
	height = _height;
	SceneManager_reshape(width, height);
}


void gameLoop()
{
	if (glutGetWindow() == 0)
	{
		OS_quit();
	}

	timenow = getNanoTime();

	while (timenow - ltime < (frameTime))
	{
		timenow = getNanoTime();
	}

	glutPostRedisplay();
}

void display()
{
	if (recordVideo)
	{
		SceneManager_pump(1.0 / 25.0, evts);
	}
	else
	{
		SceneManager_pump(timenow - ltime, evts);
	}
	if (hasQuit)
		return;

	SceneManager_eventsZero(evts);
	if (recordVideo)
	{
		if (buff == 0)
			buff = malloc(width * height * 3);


		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buff);
		fwrite(buff, width * height *3, 1, videoFile);
	}

	glutSwapBuffers();
	ltime = getNanoTime();
}

char *OS_getFileName(char *fileName)
{
	static char ret[1024];
	sprintf(ret, "%s/%s/%s", PACKAGE_DATA_DIR, PACKAGE, fileName);
	return ret;
}

FILE *OS_getFile(char *fileName)
{
	char absfn[1024];

	sprintf(absfn, "%s/%s/%s", PACKAGE_DATA_DIR, PACKAGE, fileName);

	FILE *ph = fopen(absfn, "rb");

	return ph;
}

size_t OS_getResource(char *resName, char **data)
{
	FILE *inph = OS_getFile(resName);

	if (inph == NULL)
		return 0;

	fseek(inph, 0, SEEK_END);
	size_t len = ftell(inph);
	fseek(inph, 0, SEEK_SET);

	*data = malloc(len);
	size_t ret = fread(*data, 1, len, inph);
	fclose(inph);
	return ret;
}

char *OS_getPref(const char *name, const char *defVal)
{
	return "8";
}

void OS_setPref(const char *name, const char *value)
{

}

// sound related
int OS_loadSound(char *desc)
{
	return 0;
}

int OS_loadMusic(char *desc)
{
	return 0;
}

void OS_playSound(int id)
{

}

void OS_playMusic(int id)
{

}

void OS_stopMusic(int id)
{

}
