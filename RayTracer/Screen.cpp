#include "Screen.h"

Screen::Screen()
{
}

Screen::~Screen()
{
}

void Screen::Draw( unsigned tex )
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
	// clear all relevant buffers
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	m_ScreenQuad.Draw();
}

