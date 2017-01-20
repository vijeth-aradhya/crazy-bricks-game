#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

class Laser {
   public:
      float x;
      float y;
      float x_stick;
      float y_stick;
      float stick_width;
      float stick_length;
      float x_bullet;
      float y_bullet;
      float x_shift;
      float y_shift;
      float x_stick_shift;
      float y_stick_shift;
      float rotate_angle;
      float x_shift_coord;
      float y_shift_coord;
      VAO *laserObj;
      VAO *stickObj;
      VAO *holeObj;

      void moveUp() {
        this->y+=0.07;
        this->y_stick+=0.07;
        this->y_shift+=0.07;
        this->y_stick_shift+=0.07;
      }

      void moveDown() {
        this->y+=-0.07;
        this->y_stick+=-0.07;
        this->y_shift+=-0.07;
        this->y_stick_shift+=-0.07;
      }

      void stickMoveUp() {
        if(this->rotate_angle<35.0) {
          this->rotate_angle+=1.5;
          if(this->rotate_angle>=0)
            this->x_bullet = this->stick_width*(1 - cos(this->rotate_angle*M_PI/180.0f));
          else
            this->x_bullet = this->stick_width*(1 - cos(this->rotate_angle*M_PI/180.0f));
          this->y_bullet = this->stick_length*sin(this->rotate_angle*M_PI/180.0f);
        }
      }

      void stickMoveDown() {
        if(this->rotate_angle>-35.0) {
          this->rotate_angle+=-1.5;
          if(this->rotate_angle<=0)
            this->x_bullet = this->stick_width*(1 - cos(this->rotate_angle*M_PI/180.0f));
          else
            this->x_bullet = this->stick_width*(1 - cos(this->rotate_angle*M_PI/180.0f));
          this->y_bullet = this->stick_length*sin(this->rotate_angle*M_PI/180.0f);
        }
      }

      void create () {

        float x_coord=0.3, y_coord=0.4, x_shift=-4, y_shift=0;

        GLfloat vertex_buffer_data [] = {
          -x_coord+x_shift,-y_coord+y_shift-1,0, // vertex 1
          -x_coord+x_shift,y_coord+y_shift+1,0, // vertex 2
          x_coord+x_shift,y_coord+y_shift,0, // vertex 3

          x_coord+x_shift,y_coord+y_shift,0, // vertex 3
          x_coord+x_shift,-y_coord+y_shift,0, // vertex 4
          -x_coord+x_shift,-y_coord+y_shift-1,0  // vertex 1
        };

        GLfloat color_buffer_data [] = {
          0,0.5,0.3, // color 1
          0,0.5,0.3, // color 2
          0,0,0, // color 3

          0,0,0, // color 3
          0,0,0, // color 4
          0,0.5,0.3  // color 1
        };

        this->x = x_coord+x_shift;
        this->y = y_coord+y_shift;
        this->x_shift = x_shift;
        this->y_shift = y_shift;

        // create3DObject creates and returns a handle to a VAO that can be used later
        this->laserObj = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

        x_coord=0.3, y_coord=0.09, x_shift=-3.7, y_shift=0;

        GLfloat vertex_buffer_data_stick [] = {
          -x_coord,-y_coord,0, // vertex 1
          -x_coord,y_coord,0, // vertex 2
          x_coord,y_coord+0.05,0, // vertex 3

          x_coord,y_coord+0.05,0, // vertex 3
          x_coord,-y_coord-0.05,0, // vertex 4
          -x_coord,-y_coord,0  // vertex 1
        };

        GLfloat color_buffer_data_stick [] = {
          0.3,0,0.3, // color 1
          0.3,0,0.3, // color 2
          0,0,0, // color 3

          0,0,0, // color 3
          0,0,0, // color 4
          0.3,0,0.3  // color 1
        };

        this->x_stick = x_coord+x_shift;
        this->y_stick = y_coord+y_shift;
        this->x_stick_shift = x_shift;
        this->y_stick_shift = y_shift;
        this->rotate_angle = 0;
        this->stick_length = 2*y_coord;
        this->stick_width = 2*x_coord;
        this->x_bullet = 0;
        this->y_bullet = 0;

        // create3DObject creates and returns a handle to a VAO that can be used later
        this->stickObj = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_stick, color_buffer_data_stick, GL_FILL);
      }

      void shoot() {

      }
};

class Basket {
   public:
      float x;
      float y;
      float x_shift;
      float width;
      float length;
      string color;
      VAO *basketObj;
      VAO *mouthObj1;
      VAO *mouthObj2;

      void moveLeft() {
        this->x+=-0.4;
        this->x_shift+=-0.4;
      }

      void moveRight() {
        this->x+=0.4;
        this->x_shift+=0.4;
      }

      void create () {

        float x_coord=0.6, y_coord=0.6, x_shift, y_shift=-3.4;
        int red=0, green=0, blue=0;

        if(this->color == "red") {
          red=1;
          x_shift=-1.0;
        }
        else {
          green=1;
          x_shift=1.0;
        }

        GLfloat vertex_buffer_data_mouth1 [] = {
          -x_coord+x_shift,-y_coord+y_shift,0, // vertex 1
          -x_coord+x_shift,y_coord+y_shift,0, // vertex 2
          x_coord+x_shift,y_coord+y_shift,0, // vertex 3

          x_coord+x_shift,y_coord+y_shift,0, // vertex 3
          x_coord+x_shift,-y_coord+y_shift,0, // vertex 4
          -x_coord+x_shift,-y_coord+y_shift,0  // vertex 1
        };

        GLfloat color_buffer_data_mouth1 [] = {
          red-0.8,green-0.8,blue, // color 1
          red,green,blue, // color 2
          red,green,blue, // color 3

          red,green,blue, // color 3
          red-0.8,green-0.8,blue, // color 4
          red-0.8,green-0.8,blue  // color 1
        };

        this->x = x_coord+x_shift;
        this->y = y_coord+y_shift;
        this->x_shift = 0;
        this->length = 2*y_coord;
        this->width = 2*x_coord;

        // create3DObject creates and returns a handle to a VAO that can be used later
        this->basketObj = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_mouth1, color_buffer_data_mouth1, GL_FILL);

        red=1;
        green=1;
        blue=1;
        
        x_coord/=2;
        y_coord=0.02;
        y_shift+=this->length/2;
        
        GLfloat vertex_buffer_data [] = {
          -x_coord+x_shift,-y_coord+y_shift,0, // vertex 1
          -x_coord+x_shift,y_coord+y_shift,0, // vertex 2
          x_coord+x_shift,y_coord+y_shift,0, // vertex 3

          x_coord+x_shift,y_coord+y_shift,0, // vertex 3
          x_coord+x_shift,-y_coord+y_shift-0.08,0, // vertex 4
          -x_coord+x_shift,-y_coord+y_shift,0  // vertex 1
        };

        GLfloat color_buffer_data [] = {
          red-0.5,green-0.5,blue-0.5, // color 1
          red-0.5,green-0.5,blue-0.5, // color 2
          red,green,blue, // color 3

          red,green,blue, // color 3
          red-1,green-1,blue-1, // color 4
          red-0.5,green-0.5,blue-0.5, // color 1
        };

        this->mouthObj1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
        
        GLfloat vertex_buffer_data_mouth2 [] = {
          -x_coord+x_shift,-y_coord+y_shift-0.08,0, // vertex 1
          -x_coord+x_shift,y_coord+y_shift,0, // vertex 2
          x_coord+x_shift,y_coord+y_shift,0, // vertex 3

          x_coord+x_shift,y_coord+y_shift,0, // vertex 3
          x_coord+x_shift,-y_coord+y_shift,0, // vertex 4
          -x_coord+x_shift,-y_coord+y_shift,0  // vertex 1
        };

        GLfloat color_buffer_data_mouth2 [] = {
          red-1,green-1,blue-1, // color 1
          red,green,blue, // color 2
          red-0.5,green-0.5,blue-0.5, // color 3

          red-0.5,green-0.5,blue-0.5, // color 3
          red-0.5,green-0.5,blue-0.5, // color 4
          red,green,blue  // color 1
        };

        this->mouthObj2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_mouth2, color_buffer_data_mouth2, GL_FILL);
      }
};

class Mirror {
   public:
      float x;
      float y;
      float x_shift;
      float y_shift;
      float width;
      float length;
      float rotate_angle;
      VAO *mirrorObj;

      void create (float x_shift, float y_shift, float rotate_angle) {

        float x_coord=0.6, y_coord=0.01;
        int red=0, green=0, blue=0;

        GLfloat vertex_buffer_data [] = {
          -x_coord,-y_coord,0, // vertex 1
          -x_coord,y_coord,0, // vertex 2
          x_coord,y_coord,0, // vertex 3

          x_coord,y_coord,0, // vertex 3
          x_coord,-y_coord,0, // vertex 4
          -x_coord,-y_coord,0  // vertex 1
        };

        GLfloat color_buffer_data [] = {
          red,green,blue, // color 1
          red,green,blue, // color 2
          red,green,blue, // color 3

          red,green,blue, // color 3
          red,green,blue, // color 4
          red,green,blue  // color 1
        };

        this->x_shift = x_shift;
        this->y_shift = y_shift;
        this->length = 2*y_coord;
        this->width = 2*x_coord;
        this->rotate_angle = rotate_angle;
        this->x = x_coord+x_shift-(this->width/2);
        this->y = y_coord+y_shift;

        // create3DObject creates and returns a handle to a VAO that can be used later
        this->mirrorObj = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_LINE);
      }
};

class Bullet {
   public:
      float x;
      float y;
      float x_shift;
      float y_shift;
      float vector_translate;
      float rotate_angle;
      float radius;
      float x_laser_shift;
      float y_laser_shift;
      bool status;
      bool reflected;
      VAO *bulletObj;

      void create(float rotate_angle) {
        int parts = 1000;
        float radius = 0.09;
        this->radius = radius;
        GLfloat vertex_buffer_data_hole[parts*9];
        GLfloat color_buffer_data_hole[parts*9];
        int i,j;
        float angle=(2*M_PI/parts);
        float current_angle = 0;
        for(i=0;i<parts;i++){
            for(j=0;j<3;j++){
                color_buffer_data_hole[i*9+j*3]=0.5;
                color_buffer_data_hole[i*9+j*3+1]=0.5;
                color_buffer_data_hole[i*9+j*3+2]=0.5;
            }
            vertex_buffer_data_hole[i*9]=0;
            vertex_buffer_data_hole[i*9+1]=0;
            vertex_buffer_data_hole[i*9+2]=0;
            vertex_buffer_data_hole[i*9+3]=radius*cos(current_angle);
            vertex_buffer_data_hole[i*9+4]=radius*sin(current_angle);
            vertex_buffer_data_hole[i*9+5]=0;
            vertex_buffer_data_hole[i*9+6]=radius*cos(current_angle+angle);
            vertex_buffer_data_hole[i*9+7]=radius*sin(current_angle+angle);
            vertex_buffer_data_hole[i*9+8]=0;
            current_angle+=angle;
        }
        this->rotate_angle=rotate_angle;
        this->vector_translate=0;
        this->status = 1;
        this->reflected = 0;
        this->bulletObj = create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data_hole, color_buffer_data_hole, GL_FILL);
      }
};

float randomFloat(float min, float max)
{
  float r = (float)rand() / (float)RAND_MAX;
  return min + r * (max - min);
}

class Brick {
   public:
      float x;
      float y;
      float y_shift;
      float length;
      float width;
      bool status;
      string color;
      VAO *brickObj;

      void moveDown() {
        this->y+=-0.07;
      }

      void vanish() {
        this->y_shift+=3000;
        this->y+=3000;
        this->status=0;
      }

      void create (float x_shift) {

        float x_coord=0.08, y_coord=0.15, y_shift=3.5;

        int color=(rand()%3), red=0, green=0, blue=0;
        switch(color){
          case 0:
            red=1;
            this->color="red";
            break; //optional
          case 1:
            green=1;
            this->color="green";
            break; //optional
          case 2:
            this->color="black";
            break; //optional
        }

        GLfloat vertex_buffer_data [] = {
          -x_coord+x_shift,-y_coord+y_shift,0, // vertex 1
          -x_coord+x_shift,y_coord+y_shift,0, // vertex 2
          x_coord+x_shift,y_coord+y_shift,0, // vertex 3

          x_coord+x_shift,y_coord+y_shift,0, // vertex 3
          x_coord+x_shift,-y_coord+y_shift,0, // vertex 4
          -x_coord+x_shift,-y_coord+y_shift,0  // vertex 1
        };

        GLfloat color_buffer_data [] = {
          red,green,blue, // color 1
          red,green,blue, // color 2
          red,green,blue, // color 3

          red,green,blue, // color 3
          red,green,blue, // color 4
          red,green,blue  // color 1
        };

        this->x = x_coord+x_shift;
        this->y = y_coord+y_shift;
        this->y_shift = 0;
        this->length = 2*y_coord;
        this->width = 2*x_coord;
        this->status = 1;

        // create3DObject creates and returns a handle to a VAO that can be used later
        this->brickObj = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
      }
};

Laser laser;

Brick bricks[100];

Basket baskets[2];

Bullet bullets[100];

Mirror mirrors[5];

std::vector<int> regenerateBrick;
std::vector<int> regenerateBullet;
//myvector.back();
//myvector.pop_back();
//myvector.push_back(myint);
//myvector.size();

int total_bricks, total_score, game_over, total_bullets, total_mirrors;

float bricks_speed, mirror_rotate_speed, mirror_trans_speed_1, mirror_trans_speed_2;

double last_update_bullet_time, current_bullet_time, curr_mirror_time, last_update_mirror_time;

bool level1, level2, level3, mirror_up_1, mirror_up_2;

void chooseCol(int brick_num) {
  int col=(rand()%2);
  switch(col){
    case 0:
      bricks[brick_num].create(randomFloat(-2.3, -1.3));
      break; //optional
    case 1:
      bricks[brick_num].create(randomFloat(0.8, 2.5));
      break; //optional
  }
}

void createBrick() {
  if(regenerateBrick.size()!=0){
    chooseCol(regenerateBrick.back());
    regenerateBrick.pop_back();
  }
  else {
    chooseCol(total_bricks);
    total_bricks++;
  }
}

void shootBullet() {
  if(regenerateBullet.size()!=0){
    bullets[regenerateBullet.back()].create(laser.rotate_angle);
    regenerateBullet.pop_back();
  }
  else {
    bullets[total_bullets].create(laser.rotate_angle);
    total_bullets++;
  }
}

void checkBrickYLimit() {
  int i;
  for(i=0;i<total_bricks&&bricks[i].status;i++) {
    if(bricks[i].y<baskets[0].y) {
      regenerateBrick.push_back(i);
      bricks[i].vanish();
    }
  }
}

void checkRedBasket() {
  int i;
  for(i=0;i<total_bricks&&bricks[i].status;i++) {
    if(bricks[i].color=="red" || bricks[i].color=="black") {
      if(bricks[i].y>=baskets[0].y && (bricks[i].y-bricks[i].length)<=baskets[0].y &&
        (baskets[0].x-baskets[0].width)<=(bricks[i].x-bricks[i].width) && baskets[0].x>=bricks[i].x) {
        
        if(bricks[i].color=="black")
          game_over=1;
        else{
          total_score+=20;
          regenerateBrick.push_back(i);
          bricks[i].vanish();
        }
      }
    }
  }
}

void checkGreenBasket() {
  int i;
  for(i=0;i<total_bricks&&bricks[i].status;i++) {
    if(bricks[i].color=="green" || bricks[i].color=="black") {
      if(bricks[i].y>=baskets[1].y && (bricks[i].y-bricks[i].length)<=baskets[1].y &&
        (baskets[1].x-baskets[1].width)<=(bricks[i].x-bricks[i].width) && baskets[1].x>=bricks[i].x) {
        
        if(bricks[i].color=="black")
          game_over=1;
        else{
          total_score+=20;
          regenerateBrick.push_back(i);
          bricks[i].vanish();
          //printf("Caught Green %d\n", i);
        }
      }
    }
  }
}

void checkBrickBulletCollision () {
  int i, j;
  float y_brick_center, x_brick_center, x_axis_check, y_axis_check;
  for(i=0;i<total_bricks&&bricks[i].status;i++) {
    y_brick_center = bricks[i].y - (bricks[i].length/2);
    x_brick_center = bricks[i].x - (bricks[i].width/2);
    for(j=0;j<total_bullets;j++) {
      x_axis_check = (bricks[i].width/2) + bullets[j].radius;
      y_axis_check = (bricks[i].length/2) + bullets[j].radius;
      if(abs(y_brick_center-bullets[j].y)<=y_axis_check&&abs(x_brick_center-bullets[j].x)<=x_axis_check) {
        if(bricks[i].color=="black")
          total_score+=10;
        regenerateBrick.push_back(i);
        bricks[i].vanish();
        break;
      }
    }
  }
}

void checkMirrorBulletCollision () {
  int i, j, k=0;
  float y_brick_center, x_brick_center, x_axis_check, y_axis_check;
  float c, m;
  for(i=0;i<total_mirrors;i++) {
    m = tan(mirrors[i].rotate_angle*M_PI/180.0f);
    c = mirrors[i].y-m*mirrors[i].x;
    for(j=0;j<total_bullets;j++) {
      if(abs(bullets[j].y-m*bullets[j].x-c)<0.07&&abs(bullets[j].x-mirrors[i].x)<=mirrors[i].width/2){
        bullets[j].rotate_angle=2*mirrors[i].rotate_angle-bullets[j].rotate_angle;
        bullets[j].reflected=1;
        bullets[j].x_laser_shift=bullets[j].x;
        bullets[j].y_laser_shift=bullets[j].y;
        bullets[j].vector_translate=0.04;
      }
    }
  }
}

void checkBulletOutOfWindow () {
  int i;
  for(i=0;i<total_bullets;i++) {
    if(abs(bullets[i].x)>25||abs(bullets[i].y)>25) {
      regenerateBullet.push_back(i);
    }
  }
}

void setRandomizedMirror () {
  curr_mirror_time = glfwGetTime();
  if((curr_mirror_time-last_update_mirror_time) >=1.5 && level3) {
    int col=rand()%2;
    float x_coord;
    if(col)
      x_coord=randomFloat(-2.3, -1.3);
    else
      x_coord=randomFloat(0.8, 2.5);
    mirrors[4].create(x_coord, randomFloat(-1.6, 2.2), randomFloat(-360, 360));
    last_update_mirror_time=curr_mirror_time;
  }
}

void checkLevel() {
  if(total_score>20 && total_score<60)
    level2=1;
  else if(total_score>60){
    level3=1;
    total_mirrors=5;
  }
}

void checkGameOver () {
  if(game_over)
    exit(0);
}

void updateGameStatus () {
  checkRedBasket();
  checkGreenBasket();
  checkGameOver();
  checkBrickBulletCollision();
  checkMirrorBulletCollision();
  checkBulletOutOfWindow();
  checkBrickYLimit();
  checkLevel();
  setRandomizedMirror();
}

/**************************
 * Customizable functions *
 **************************/

float rectangle_rot_dir = 1;
bool triangle_rot_status = false;
bool rectangle_rot_status = false;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_SPACE:
                current_bullet_time = glfwGetTime(); // Time in seconds
                if ((current_bullet_time - last_update_bullet_time) >= 1.0) {
                  last_update_bullet_time = current_bullet_time;
                  shootBullet();
                }
                break;
            case GLFW_KEY_A:
                laser.stickMoveUp();
                break;
            case GLFW_KEY_D:
                laser.stickMoveDown();
                break;
            case GLFW_KEY_S:
                laser.moveUp();
                break;
            case GLFW_KEY_F:
                laser.moveDown();
                break;
            case GLFW_KEY_LEFT:
                if(mods == GLFW_MOD_CONTROL)
                  baskets[0].moveLeft();
                if(mods == GLFW_MOD_ALT)
                  baskets[1].moveLeft();
                break;
            case GLFW_KEY_RIGHT:
                if(mods == GLFW_MOD_CONTROL)
                  baskets[0].moveRight();
                if(mods == GLFW_MOD_ALT)
                  baskets[1].moveRight();
                break;
            case GLFW_KEY_N:
                if(bricks_speed<=0.02)
                  bricks_speed+=0.001;
                break;
            case GLFW_KEY_M:
                if(bricks_speed>=0.003)
                  bricks_speed+=-0.001;
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                //triangle_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle, *testCircle;

void testPoint() {
  int parts = 15;
  float radius = 0.09;
  GLfloat vertex_buffer_data_hole[parts*9];
  GLfloat color_buffer_data_hole[parts*9];
  int i,j;
  float angle=(2*M_PI/parts);
  float current_angle = 0;
  for(i=0;i<parts;i++){
      for(j=0;j<3;j++){
          color_buffer_data_hole[i*9+j*3]=0.5;
          color_buffer_data_hole[i*9+j*3+1]=0.5;
          color_buffer_data_hole[i*9+j*3+2]=0.5;
      }
      vertex_buffer_data_hole[i*9]=0;
      vertex_buffer_data_hole[i*9+1]=0;
      vertex_buffer_data_hole[i*9+2]=0;
      vertex_buffer_data_hole[i*9+3]=radius*cos(current_angle);
      vertex_buffer_data_hole[i*9+4]=radius*sin(current_angle);
      vertex_buffer_data_hole[i*9+5]=0;
      vertex_buffer_data_hole[i*9+6]=radius*cos(current_angle+angle);
      vertex_buffer_data_hole[i*9+7]=radius*sin(current_angle+angle);
      vertex_buffer_data_hole[i*9+8]=0;
      current_angle+=angle;
  }

  testCircle = create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data_hole, color_buffer_data_hole, GL_FILL);
}

// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle (float x_shift, float y_shift, int rect_num)
{

  float x_coord=0.08, y_coord=0.15;

  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -x_coord+x_shift,-y_coord+y_shift,0, // vertex 1
    -x_coord+x_shift,y_coord+y_shift,0, // vertex 2
    x_coord+x_shift,y_coord+y_shift,0, // vertex 3

    x_coord+x_shift,y_coord+y_shift,0, // vertex 3
    x_coord+x_shift,-y_coord+y_shift,0, // vertex 4
    -x_coord+x_shift,-y_coord+y_shift,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,1, // color 1
    0,0,1, // color 2
    0,0,1, // color 3

    0,0,1, // color 3
    0,0,1, // color 4
    0,0,1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
float fall_down_speed = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

/*
  // Render the scence
  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(triangle);
*/

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  int i;

  // Draw Bricks
  for(i=0;i<total_bricks;i++) {
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateObject = glm::translate (glm::vec3(0, bricks[i].y_shift, 0));        // glTranslatef
    //glm::mat4 rotateObject = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateObject);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    bricks[i].y_shift-=bricks_speed;
    bricks[i].y -= bricks_speed;
    draw3DObject(bricks[i].brickObj);
  }

  // Draw Baskets
  for(i=0;i<2;i++) {
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateObject = glm::translate (glm::vec3(baskets[i].x_shift, 0, 0));
    Matrices.model *= (translateObject);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(baskets[i].basketObj);

    Matrices.model = glm::mat4(1.0f);
    translateObject = glm::translate (glm::vec3(baskets[i].x_shift-(baskets[i].width/4), 0, 0));
    Matrices.model *= (translateObject);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(baskets[i].mouthObj1);

    Matrices.model = glm::mat4(1.0f);
    translateObject = glm::translate (glm::vec3(baskets[i].x_shift+(baskets[i].width/4), 0, 0));
    Matrices.model *= (translateObject);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(baskets[i].mouthObj2);
  }

  // draw3DObject draws the VAO given to it using current MVP matrix

  // Draw Laser
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateLaser = glm::translate (glm::vec3(0, laser.y_shift, 0));
  Matrices.model *= (translateLaser);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(laser.laserObj);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateObject = glm::translate (glm::vec3(laser.x_stick_shift, laser.y_stick_shift, 0));        // glTranslatef
  glm::mat4 rotateObject = glm::rotate((float)(laser.rotate_angle*M_PI/180.0f), glm::vec3(0,0,1));
  Matrices.model *= (translateObject * rotateObject);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(laser.stickObj);

  // Draw bullets
  for(i=0;i<total_bullets;i++) {
    Matrices.model = glm::mat4(1.0f);
    if(!bullets[i].reflected) {
      bullets[i].x_laser_shift = laser.x_stick+laser.x_bullet;
      bullets[i].y_laser_shift = laser.y_stick-(laser.stick_length/2)+laser.y_bullet;
    }
    bullets[i].x = bullets[i].x_laser_shift+bullets[i].vector_translate*cos(bullets[i].rotate_angle*M_PI/180.0f);
    bullets[i].y = bullets[i].y_laser_shift+bullets[i].vector_translate*sin(bullets[i].rotate_angle*M_PI/180.0f);
    glm::mat4 translateBullet = glm::translate (glm::vec3(bullets[i].x, bullets[i].y, 0));    
    Matrices.model *= (translateBullet);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    bullets[i].vector_translate+=0.04;
    draw3DObject(bullets[i].bulletObj);
  }

  // Draw mirrors
  glm::mat4 rotateMirror;
  for(i=0;i<total_mirrors;i++) {
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateMirror = glm::translate (glm::vec3(mirrors[i].x_shift, mirrors[i].y_shift, 0));
    if(i<2) {
      if(i==0){
        if(level1)  
          mirrors[i].rotate_angle+=mirror_rotate_speed;
        if(level2) {
          mirrors[i].rotate_angle+=mirror_rotate_speed+3;
          mirrors[i].y-=mirror_trans_speed_1;
          if((mirrors[i].y_shift+mirror_trans_speed_1)<0.0)
            mirror_up_1=1;
          if((mirrors[i].y_shift+mirror_trans_speed_1)>2.9)
            mirror_up_1=0;
          if(!mirror_up_1)
            mirror_trans_speed_1-=0.007;
          else
            mirror_trans_speed_1+=0.007;
          translateMirror = glm::translate (glm::vec3(mirrors[i].x_shift, mirrors[i].y_shift+mirror_trans_speed_1, 0));
          mirrors[i].y+=mirror_trans_speed_1;
        }
      }
      else {
        if(level2) {
          if(mirrors[i].rotate_angle<-150) {
            mirrors[i].rotate_angle=0;
            mirror_up_2=1;
          }
          if(mirrors[i].rotate_angle>150) {
            mirrors[i].rotate_angle=0;
            mirror_up_2=0;
          }
          if(!mirror_up_2)
            mirrors[i].rotate_angle-=mirror_rotate_speed+0.7;
          else
            mirrors[i].rotate_angle+=mirror_rotate_speed+0.7;
        }
      }
    }
    rotateMirror = glm::rotate((float)(mirrors[i].rotate_angle*M_PI/180.0f), glm::vec3(0,0,1));
    Matrices.model *= (translateMirror * rotateMirror);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(mirrors[i].mirrorObj);
  }

/* TEST POINT
  Matrices.model = glm::mat4(1.0f);
  translateObject = glm::translate (glm::vec3(mirrors[0].x, mirrors[0].y, 0));
  Matrices.model *= (translateObject);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(testCircle);
*/

  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  //  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  //rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
  total_bricks=0;
  total_score=0;
  game_over=0;
  total_mirrors=4;
  total_bullets=0;
  bricks_speed=0.005;
  mirror_rotate_speed=1;
  mirror_trans_speed_1=0;
  mirror_trans_speed_2=0;
  mirror_up_1=0;
  mirror_up_2=0;
  laser.create();
  baskets[0].color="red";
  baskets[1].color="green";
  baskets[0].create();
  baskets[1].create();
  mirrors[0].create(0.2, 2.9, -30);
  mirrors[1].create(0.2, -1.7, 25);
  mirrors[2].create(3.2, 2.3, -45);
  mirrors[3].create(3.2, -1.3, 70);
  level1=1;
  level2=0;
  level3=0;
  //testPoint();
	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (1.0f, 1.0f, 1.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1000;
	int height = 600;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    last_update_bullet_time = glfwGetTime();
    last_update_mirror_time = last_update_bullet_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        // Update the game status each iter
        updateGameStatus();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 1.5) { // atleast 1.5s elapsed since last frame
            createBrick();
            last_update_time = current_time;
        }
    }

    glfwTerminate();
//    exit(EXIT_SUCCESS);
} 