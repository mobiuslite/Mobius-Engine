#ifndef _FBO_HG_
#define _FBO_HG_

#include <string>

#include "../GLCommon.h"

//Holds all frame buffer objects for deferred rendering
class cFBO
{
public:
	cFBO() : 
		ID(0), 
		colourTexture_0_ID(0),
		depthTexture_ID(0),
		vertexMatColour_1_ID(0),
		vertexNormal_2_ID(0),
		vertexWorldPos_3_ID(0),
		vertexSpecular_4_ID(0),
		brightColour_5_ID(0),
		vertexLightSpacePos_7_ID(0),
		vertexEmmision_6_ID(0),
		width(-1), height(-1) {};

	GLuint ID;						// = 0;
	GLuint colourTexture_0_ID;		// = 0;

	GLuint vertexMatColour_1_ID;
	GLuint vertexNormal_2_ID;
	GLuint vertexWorldPos_3_ID;
	GLuint vertexLightSpacePos_7_ID;
	GLuint vertexSpecular_4_ID;
	GLuint vertexEmmision_6_ID;

	GLuint brightColour_5_ID;

//	GLuint TBDTexture_1_ID;
//	GLuint TBDTexture_2_ID;

	GLuint depthTexture_ID;		// = 0;
	GLint width;		// = 512 the WIDTH of the framebuffer, in pixels;
	GLint height;

	// Inits the FBP
	bool init(int width, int height, std::string &error);
	bool shutdown(void);
	// Calls shutdown(), then init()
	bool reset(int width, int height, std::string &error);
	
	void clearBuffers(bool bClearColour = true, bool bClearDepth = true);

	void clearColourBuffer(int bufferindex);
	void clearAllColourBuffers(void);
	void clearDepthBuffer(void);
	void clearStencilBuffer( int clearColour, int mask = 0xFF );

	int getMaxColourAttachments(void);
	int getMaxDrawBuffers(void);
};

#endif
