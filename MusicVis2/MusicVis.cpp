#pragma comment(lib, "winmm.lib")



#include <iostream>
#include <windows.h>
#include <mmsystem.h>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <fftw3.h>
#include <ctime>

#include <complex>
#include <iostream>
#include <valarray>


//OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>


//GLM Mathematics.
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//CompuGr�fica Graphic Engine.
#include "stb_image.h"
#include "SOIL2/SOIL2.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

typedef std::complex<double> Complex;
typedef std::valarray<Complex> CArray;
const double PI = 3.141592653589793238460;


void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);

// Cooley-Tukey FFT (in-place, breadth-first, decimation-in-frequency)
// Better optimized but less intuitive
// !!! Warning : in some cases this code make result different from not optimased version above (need to fix bug)
// The bug is now fixed @2017/05/30 

void fft(CArray& x);

//Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

Camera camera(glm::vec3(0.0f, 0.0f, 40.0));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

//

struct wav_header {

    /*
        Basandonos en la estructura  canonica del formato WAVE:
        http://soundfile.sapp.org/doc/WaveFormat/

    */
    char* ChunkID = (char*)calloc(4, 4);
    unsigned int ChunkSize = 0;
    char* Format = (char*)calloc(4, 4);

    char* Subchunk1ID = (char*)calloc(4, 4);
    unsigned long int SubChunk1Size = 0;
    unsigned short int AudioFormat = 0;
    unsigned short int NumChannels = 0;
    unsigned long SampleRate = 0;
    unsigned long ByteRate = 0;
    unsigned short int BlockAlign = 0;
    unsigned short int BitsPerSample = 0;

    char* Subchunk2ID = (char*)calloc(4, 4);
    unsigned long int Subchunk2Size = 0;
    short int* audiodata;

    unsigned int SampleCount;

public:
    void readWAV(std::string pathaudioFile) {

        FILE* audioin;
        fopen_s(&audioin, pathaudioFile.c_str(), "rb");

        fread(ChunkID, 4, 1, audioin);
        fread(&ChunkSize, 4, 1, audioin);
        fread(Format, 4, 1, audioin);
        fread(Subchunk1ID, 4, 1, audioin);
        fread(&SubChunk1Size, 4, 1, audioin);
        fread(&AudioFormat, 2, 1, audioin);
        fread(&NumChannels, 2, 1, audioin);
        fread(&SampleRate, 4, 1, audioin);
        fread(&ByteRate, 4, 1, audioin);
        fread(&BlockAlign, 2, 1, audioin);
        fread(&BitsPerSample, 2, 1, audioin);
        fread(Subchunk2ID, 4, 1, audioin);
        fread(&Subchunk2Size, 4, 1, audioin);

        //short int* data = (short int*) calloc(1, Subchunk2Size);
        //fread(data, Subchunk2Size, 1, audioin);


        int sample_size = BitsPerSample / 8; //Esto esta en Bytes.
        int sample_count = (Subchunk2Size) / sample_size;
        std::cout << "Sample count: " << sample_count << '\n';
        SampleCount = sample_count;

        audiodata = (short int*)calloc(sample_count, 2);
        
        for (int i = 0; i < sample_count; i++)
        {

            fread(&audiodata[i], 2, 1, audioin);
            //std::cout << "Sample " << i << " Data: " << audiodata[i] << '\n';


        }


        fclose(audioin);

        

    }

    void print_header() {

        std::cout << "ChunkID: " << ChunkID << std::endl;
        std::cout << "ChunkSize: " << ChunkSize << std::endl;
        std::cout << "Format: " << Format << std::endl;
        std::cout << "Subchink1ID: " << Subchunk1ID << std::endl;
        std::cout << "SubChunk1Size: " << SubChunk1Size << std::endl;
        std::cout << "AudioFormat: " << AudioFormat << std::endl;
        std::cout << "NumChannels: " << NumChannels << std::endl;
        std::cout << "SampleRate: " << SampleRate << std::endl;
        std::cout << "ByteRate: " << ByteRate << std::endl;
        std::cout << "BlockAllign: " << BlockAlign << std::endl;
        std::cout << "BitsPerSample: " << BitsPerSample << std::endl;
        std::cout << "Subchunk2ID: " << Subchunk2ID << std::endl;
        std::cout << "Subchunk2Size: " << Subchunk2Size << std::endl;

    }



};


void PlayMusic(LPCWSTR pathname) {

    std::cout << "Playing music! " << std::endl;

    PlaySound(pathname, NULL, SND_FILENAME);
}

void ProcessSound(std::string pathname, wav_header &header) {


    header.readWAV(pathname);
    header.print_header();

}

void FFT(wav_header& AudioFile, fftw_complex* out) {
    
    fftw_plan p;
    
   
    p = fftw_plan_dft_r2c_1d( AudioFile.SampleCount, (double*)AudioFile.audiodata, out, FFTW_ESTIMATE);
    
    fftw_execute_dft_r2c(p, (double*)AudioFile.audiodata, out);

    fftw_destroy_plan(p);
}   

void Draw(wav_header&  AudioFile) {
    GLFWwindow* window;

    /* Inicializamos la biblioteca */
    if (!glfwInit())
        //return -1;
        return;

    /* Crear ventana y su contexto OpenGL*/
    window = glfwCreateWindow(WIDTH, HEIGHT, "MusicVis", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return;
    }


    /* Hacemos el contexto de la ventana actual */
    glfwMakeContextCurrent(window);
    
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    // Set the required callback functions
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
		std::cout << "GLEW ERROR " << std::endl;


	std::cout << glGetString(GL_VERSION) << std::endl;
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Initialization ends here.

    //Getting Shaders.
    Shader Anim2("Shaders/anim.vs", "Shaders/anim.frag");
    Model Piso((char*)"Models/Sea/Sea.obj");
	


    //Projection Matrixes.
    glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);



    /*float positions[] = { AudioFile.audiodata[0], 
                          AudioFile.audiodata[1], 
                          AudioFile.audiodata[2], 
                          AudioFile.audiodata[3],
                          AudioFile.audiodata[4],
                          AudioFile.audiodata[5],
                          AudioFile.audiodata[6],
                          AudioFile.audiodata[7],
                          AudioFile.audiodata[8],
                          AudioFile.audiodata[9],

    };*/

    //unsigned int indices[992474]; //Deber ser de tipo unsigned SIEMPRE
			
    unsigned int indices[] = { 1,0,2,3 };

	/* Loop until the user closes the window */

	//GLint iTime = glGetUniformLocation(shader, "iTime");
	float r = 0.0f;
	float increment = 0.05f;
	float initialTime = clock() / (float)CLOCKS_PER_SEC;
    float tiempo;
	int frames = 0;
	double elapsed = 0;
    int samplecount = 0;
        
		while (!glfwWindowShouldClose(window))
		{

			float currentTime = clock() / (float)CLOCKS_PER_SEC ;
			elapsed = (clock() / (double)CLOCKS_PER_SEC) - initialTime;

            float deltaT = currentTime - elapsed;


            //FFT Calculation each frame :S
            float positions[] = { 
                          AudioFile.audiodata[0 + samplecount],
                          AudioFile.audiodata[1 + samplecount],
                          AudioFile.audiodata[2 + samplecount],
                          AudioFile.audiodata[3 + samplecount],
                          AudioFile.audiodata[4 + samplecount],
                          AudioFile.audiodata[5 + samplecount],
                          AudioFile.audiodata[6 + samplecount],
                          AudioFile.audiodata[7 + samplecount],
                          AudioFile.audiodata[8 + samplecount],
                          AudioFile.audiodata[9 + samplecount],

            };


            Complex* samples = (Complex*)malloc(sizeof(Complex*) * 10 * 4);

            for (size_t i = 0; i < 10; i++)
            {
                Complex temp( positions[i], 1);
                //std::cout << i << std::endl;
                samples[i] = temp;

            }

            CArray sample_data(samples, 10);
            fft(sample_data);


			float framedata[]{
               
                sample_data[1].real(), 
                sample_data[2].real(), 
                sample_data[3].real(),
                /*sample_data[4].real(),
                sample_data[5].real(),
                sample_data[6].real(),
                sample_data[7].real(),
                sample_data[8].real(),
                sample_data[9].real()*/
			};

            
            // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
            glfwPollEvents();
            
            //DoMovement();

			/* Render here */
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			
            // Create camera transformations
            glm::mat4 view;
            view = camera.GetViewMatrix();

            // Get the uniform locations
            GLint modelLoc = glGetUniformLocation(Anim2.Program, "model");
            GLint viewLoc = glGetUniformLocation(Anim2.Program, "view");
            GLint projLoc = glGetUniformLocation(Anim2.Program, "projection");

            // Pass the matrices to the shader
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


            glm::mat4 model(1);
            glBindVertexArray(0);

            Anim2.Use();

            GLint viewPosLoc = glGetUniformLocation(Anim2.Program, "viewPos");
            glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
            
            tiempo = glfwGetTime();

            //Set matrices
            modelLoc = glGetUniformLocation(Anim2.Program, "model");
            viewLoc = glGetUniformLocation(Anim2.Program, "view");
            projLoc = glGetUniformLocation(Anim2.Program, "projection");
            
            //Set Uniforms.

            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1f(glGetUniformLocation(Anim2.Program, "time"), tiempo);
            glUniform3f(glGetUniformLocation(Anim2.Program, "sample"), framedata[0], framedata[1], framedata[2]);
            model = glm::mat4(1);
            model = glm::translate(model, glm::vec3(0.0f, 0.09f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Piso.Draw(Anim2);
            glBindVertexArray(0);


			

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			//Contador de Frames y tiempo transcurrido
            
           
            frames++;
            samplecount = samplecount + 1480;
            //samplecount++;

            if (samplecount >= 15969528)
                break;

            
#if 0
			
			std::cout << "Frame Count: " << frames << "  Elapsed " << elapsed  << std::endl;
            std::cout << "f0: " << framedata[0] << "  f1 " << framedata[1]
                << "  f0 " << framedata[2] << std::endl;
#endif    
		}

        glfwTerminate();

        
}


int main()
{
    
    std::cout << "Listen World!\n";


    
    

    wav_header audiodata; 
    ProcessSound("Tracks/kini.wav", audiodata);

    
   // fftw_complex* sampless = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * audiodata.SampleCount*4);
   // FFT(audiodata, sampless);

    //Complex* samples = (Complex*) malloc(sizeof(Complex*) * audiodata.SampleCount*4);
    //
    //

    //for (size_t i = 0; i < audiodata.SampleCount; i++)
    //{
    //    Complex temp(1, 0);
    //    //std::cout << i << std::endl;
    //    samples[i] = temp;
    //    
    //}

    //CArray sample_data(samples, audiodata.SampleCount);
    //std::cout << sample_data[0] << std::endl;

    //fft(sample_data);

    //std::cout << sample_data[0] << std::endl;
    
    
    

    LPCWSTR Pathname = L"Tracks/kini.wav";
    std::thread Playback(PlayMusic, Pathname);

    
    
    Draw(audiodata);
    
    Playback.hardware_concurrency();
    
    Playback.join();
    


    return 0;



}



// Cooley-Tukey FFT (in-place, breadth-first, decimation-in-frequency)
// Better optimized but less intuitive
// !!! Warning : in some cases this code make result different from not optimased version above (need to fix bug)
// The bug is now fixed @2017/05/30 

void fft(CArray& x)
{
    const size_t N = x.size();
    if (N <= 1) return;
    
    // divide
    CArray even = x[std::slice(0, N / 2, 2)];
    CArray  odd = x[std::slice(1, N / 2, 2)];

    // conquer
    fft(even);
    fft(odd);

    // combine
    for (size_t k = 0; k < N / 2; ++k)
    {
        Complex t = std::polar(1.0, -2 * PI * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k + N / 2] = even[k] - t;
    }
}


// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
        {
            keys[key] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            keys[key] = false;
        }
    }

    
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

    lastX = xPos;
    lastY = yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}