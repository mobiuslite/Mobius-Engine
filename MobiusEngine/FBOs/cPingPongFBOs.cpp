#include "cPingPongFBOs.h";

cPingPongFBOs::cPingPongFBOs(float width, float height, cShaderManager::cShaderProgram* program)
{
    this->width = width;
    this->height = height;

    this->program = program;

    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffer);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, this->width, this->height, 0, GL_RGBA, GL_FLOAT, NULL
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0
        );

        bool bFrameBufferIsGoodToGo = true;

        switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
        {
        case GL_FRAMEBUFFER_COMPLETE:
            bFrameBufferIsGoodToGo = true;
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            bFrameBufferIsGoodToGo = false;
            break;
            //	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        case GL_FRAMEBUFFER_UNSUPPORTED:
        default:
            bFrameBufferIsGoodToGo = false;
            break;
        }//switch ( glCheckFramebufferStatus(GL_FRAMEBUFFER) )

        // Point back to default frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

cPingPongFBOs::~cPingPongFBOs()
{
    glDeleteTextures(1, &(this->pingpongBuffer[0]));
    glDeleteTextures(1, &(this->pingpongBuffer[1]));

    glDeleteFramebuffers(1, &(this->pingpongFBO[0]));
    glDeleteFramebuffers(1, &(this->pingpongFBO[1]));

}

void cPingPongFBOs::ClearBuffers()
{
    GLfloat rgbBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    glBindFramebuffer(GL_FRAMEBUFFER, this->pingpongFBO[0]);
    glClearBufferfv(GL_COLOR, 0, rgbBlack);

    glBindFramebuffer(GL_FRAMEBUFFER, this->pingpongFBO[1]);
    glClearBufferfv(GL_COLOR, 0, rgbBlack);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cPingPongFBOs::BlurBuffer(bool firstIteration, bool horizontal, GLuint firstMap)
{
    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
    glUniform1f(program->uniformLocations["horizontal"], horizontal ? (float)GL_TRUE : (float)GL_FALSE);

    if (firstIteration)
    {
        //glBindTexture(GL_TEXTURE_2D, g_fbo->brightColour_5_ID);

        GLint unit = 20;
        glActiveTexture(unit + GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, firstMap);
        glUniform1i(program->uniformLocations["bloomMap"], unit);
    }
    else
    {
        //glBindTexture(GL_TEXTURE_2D, pingPongFBO->pingpongBuffer[!horizontal]);

        GLint unit = 21;
        glActiveTexture(unit + GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
        glUniform1i(program->uniformLocations["bloomMap"], unit);
    }
}