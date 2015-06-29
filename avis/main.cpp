// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
using namespace glm;

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>

// pulseaudio for playback
#include <pulse/simple.h>
#include <pulse/error.h>

//read wave file
#include <sndfile.h>

//FFT
#include "AudioFFT.h"

//EGL
#include "../common/startScreen.h"
#include "../common/LoadShaders.h"
#include "../common/texture.h"
#include "../common/objloader.h"

static SF_INFO streamInfo;
static SNDFILE *sndfile;
static audiofft::AudioFFT fft;

static const size_t fftSize = 32; //must be power of 2 (2,4,8,16,32,64,128...)

static float *tmpbuf;
static float *re;
static float *im;
static float *am;

static void applyHannWindow(float *data, int fft_size)
{
    for (int i = 0; i < fft_size; i++)
    {
        double multiplier = 0.5 * (1 - cos(2*M_PI*i/ (fft_size - 1)));
        data[i] = multiplier * data[i];
    }
}

int main( int argc, const char * argv[] )
{
    //WAVE FILE
    if (argc != 2)
    {
        printf("No file!");

        return -1;
    }

    streamInfo.format = 0;

    sndfile = sf_open(argv[1], SFM_READ, &streamInfo);

    //PLAYBACK
    // static pa_sample_spec ss;
    // ss.format = PA_SAMPLE_S16LE;
    // ss.rate = 44100;
    // ss.channels = 2;
    
    // pa_simple *s = NULL;
    // s = pa_simple_new(NULL, argv[0], PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, NULL);

    //INIT FFT
    printf("bufsize: %d\n", audiofft::AudioFFT::ComplexSize(fftSize));
    size_t bufSize = sizeof(float) * audiofft::AudioFFT::ComplexSize(fftSize);

    tmpbuf = (float *) malloc(bufSize);
    im = (float *) malloc(bufSize);
    re = (float *) malloc(bufSize);
    am = (float *) malloc(bufSize);

    if (!sndfile)
    {
        printf("audio file error\nno file %s\n",argv[1]);
        return -1;
    }

    fft.init(fftSize);

    //INIT GRAPHICS
    InitGraphics();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);

    GLuint programID = LoadShaders( "TransformVertexShader.vert", "TextureFragmentShader.frag" );
    
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
    
    GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
    GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
    GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");

    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    glm::mat4 Model = glm::mat4(1.0f);

    GLuint Texture = loadBMP_custom("uvtemplate.bmp");
    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    bool res = loadOBJ("cube.obj", vertices, uvs, normals);

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

    glUseProgram(programID);
    GLuint LightID0 = glGetUniformLocation(programID, "LightPosition_a_worldspace");
    GLuint LightID1 = glGetUniformLocation(programID, "LightPosition_b_worldspace");
    GLuint LightID2 = glGetUniformLocation(programID, "LightPosition_c_worldspace");
    GLuint LightID3 = glGetUniformLocation(programID, "LightPosition_d_worldspace");

    glm::vec3 rotation = glm::vec3(0,0,0);
    srand (1337);
    
    bool run = true;
    printf("starting Main Loop\n");
    do{ // ============ MAIN LOOP

        int numSamples = 100;

        sf_count_t readFrames = sf_read_float(sndfile, tmpbuf, fftSize);
        if (readFrames == 0){
            printf("end of file reached\n");
            run = false;
        }
        //play segment
        //pa_simple_write(s, tmpbuf, (size_t) readFrames, NULL);
        //FFT
        applyHannWindow(tmpbuf, fftSize);
        fft.fft(tmpbuf, re, im);
        for (int t = 0; t < audiofft::AudioFFT::ComplexSize(fftSize); t++){
            float real = re[t];/* real */
            float ima = im[t];/* imaginary */
            double normBinMag = 2.*sqrt(real*real + ima*ima) / (double)fftSize; /* get normalized bin magnitude */
            double amplitude = 20. * log10( (double) normBinMag );/* convert to dB value */

            am[t] = amplitude;

            /* and display */
            // printf("bin: %d,\tfreq: %f [Hz],\tmag: %f,\t ampl.: %f [dB]\n", \
            //        t, streamInfo.samplerate*.5*(double)t/fftSize, normBinMag, amplitude);
        }

        static float waveseed = 0.0;
        waveseed+=0.01;
    
        // Camera matrix
        glm::mat4 View = glm::lookAt(glm::vec3(23,12,15),glm::vec3(0,0,0),glm::vec3(0,1,0));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(programID);
    
        // Lights
        glm::vec3 lightPos0 = glm::vec3(sinf(waveseed)*5,-5,sinf(waveseed)*5);
        glm::vec3 lightPos1 = glm::vec3(cosf(waveseed)*20,sinf(waveseed)*20,sinf(waveseed)*20);
        glm::vec3 lightPos2 = glm::vec3(sinf(waveseed)*5,5,cosf(waveseed)*5);
        glm::vec3 lightPos3 = glm::vec3(cosf(waveseed)*20,cosf(waveseed)*20,cosf(waveseed)*20);
        glUniform3f(LightID0, lightPos0.x, lightPos0.y, lightPos0.z);
        glUniform3f(LightID1, lightPos1.x, lightPos1.y, lightPos1.z);
        glUniform3f(LightID2, lightPos2.x, lightPos2.y, lightPos2.z);
        glUniform3f(LightID3, lightPos3.x, lightPos3.y, lightPos3.z);

        //Texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glUniform1i(TextureID, 0);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(vertexPosition_modelspaceID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(vertexPosition_modelspaceID,3,GL_FLOAT,GL_FALSE,0,(void*)0);

        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(vertexUVID);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(vertexUVID,2,GL_FLOAT,GL_FALSE,0,(void*)0);

        // 3rd attribute buffer : normals
        glEnableVertexAttribArray(vertexNormal_modelspaceID);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(vertexNormal_modelspaceID,3,GL_FLOAT,GL_FALSE,0,(void*)0);
    
        // Rebuild the Model matrix
        rotation.y += 0.01f;
        glm::mat4 rotationMatrix = glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z);
        
        int numberOfBands = audiofft::AudioFFT::ComplexSize(fftSize);

        //draw
        for (int i = 0; i < numberOfBands; i++) {
            for (int j = 0; j < numberOfBands; j++) {
                int acc = j;
                glm::vec3 scale;
                // if (j==fftSize-1){
                    scale = glm::vec3(1, fabs(am[acc]) / 70. * 10. + 1.,1); //use fft
                // }else{
                //     scale = glm::vec3(1,1,1);
                // }
                //glm::vec3 scale = glm::vec3(1,fabs(sinf(((float)i/3)+((float)j/3)+waveseed))*4+1.0,1); //use sin
                glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), scale);
                glm::vec3 position = glm::vec3(i-(float)numberOfBands/2.0,0,j-(float)numberOfBands/2.0);
                glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
                Model =  rotationMatrix * translationMatrix * scalingMatrix;
                glm::mat4 MVP = Projection * View * Model;
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
                glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model[0][0]);
                glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View[0][0]);

                // draw call
                glDrawArrays(GL_TRIANGLES, 0, vertices.size() );
            }
        }

        glDisableVertexAttribArray(vertexPosition_modelspaceID);
        glDisableVertexAttribArray(vertexUVID);
        glDisableVertexAttribArray(vertexNormal_modelspaceID);

        updateScreen();
    }while(run);

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);
    return 0;
}
