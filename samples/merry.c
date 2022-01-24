/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Demonstrates drawing a shaded 3D triangle.
 *
 * @author Scott Kuhl
 */

#include "libkuhl.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

static GLuint program = 0; /**< id value for the GLSL program */

static kuhl_geometry roof;
static kuhl_geometry ground;
static kuhl_geometry poles;

static kuhl_geometry *cow;
static kuhl_geometry *hippo;
static kuhl_geometry *lion;
static kuhl_geometry *skel;

static float cowPos[3] = {0, 0, 4};
static float hippoPos[3] = {4, 0, 0};
static float lionPos[3] = {-4, 0, 0};
static float skelPos[3] = {0, 0, -4};

static int isRight=1;
static int isRotate=1;
static int isUpDown=1;

static float up1=0, up2=0, up3=0, up4=0;
static float rotateMat[16], transMat[16], turnMat[16], scaleMat[16];

//Math Helpers
//

/**
 * Copies vector 1 into vector 2
 * 
 * @param vec3 output vector
 * @param vec1 left operand
 * @param vec2 right operand
 * @return result vector
 */
float * vectorCpy(float vec2[3], float vec1[3]){
    vec2[0] = vec1[0];
    vec2[1] = vec1[1];
    vec2[2] = vec1[2];
    return vec2;
}

/**
 * Computes vector from subbing vec2 from vec1
 * 
 * @param vec3 output vector
 * @param vec1 left operand
 * @param vec2 right operand
 * @return result vector
 */
float * vectorSub(float vec3[3], float vec1[3], float vec2[3]){
    vec3[0] = vec1[0] - vec2[0];
    vec3[1] = vec1[1] - vec2[1];
    vec3[2] = vec1[2] - vec2[2];
    return vec3;
}

/**
 * Normalizes vector with 3 dimensions
 * 
 * @param vec pixel location vector
 * @return normalized vector
 */
float * vectorNormalize(float vec[3]){
    float magnitude = sqrt(pow(vec[0],2.0) + pow(vec[1], 2.0) + pow(vec[2],2.0));
    vec[0] = vec[0] / magnitude;
    vec[1] = vec[1] / magnitude;
    vec[2] = vec[2] / magnitude;
    return vec;
}

/**
 * Computes cross product of vec1 and vec2
 * 
 * @param vec3 output vector
 * @param vec1 left operand
 * @param vec2 right operand
 * @return result vector
 */
float * vectorCrossProd(float vec3[3], float vec1[3], float vec2[3]){
	//         0  1  2
	// vec1 = [a, b ,c]
	// vec2 = [d, e, f]
	vec3[0] = (vec1[1]*vec2[2])-(vec1[2]*vec2[1]); //bf - ce (12 - 21)
	vec3[1] = (vec1[2]*vec2[0])-(vec1[0]*vec2[2]); //cd - af (20 - 02)
	vec3[2] = (vec1[0]*vec2[1])-(vec1[1]*vec2[0]); //ae - bd (01 - 10)
	return vec3;
}

/**
 * Computes normal given two vertices on a triangle
 * 
 * @param vec4 output vector
 * @param vec1 vector 1
 * @param vec2 vector 2
 * @param vec3 vector 3
 * @return result vector
 */
float * triNormal(float vec4[3], float vec1[3], float vec2[3], float vec3[3]){
	float temp1[3], temp2[3];
	vectorSub(temp1, vec1, vec2);
	vectorSub(temp2, vec1, vec3);
	vectorCrossProd(vec4, temp1, temp2);
	vectorNormalize(vec4);
	return vec4;
}

/* Called by GLFW whenever a key is pressed. */
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/* If the library handles this keypress, return */
	if (kuhl_keyboard_handler(window, key, scancode, action, mods))
		return;

	/* Action can be press (key down), release (key up), or repeat
	   (key is held down, the delay until the first repeat and the
	   rate at which repeats fire may depend on the OS.)
	   
	   Here, we ignore any press events so the event will only
	   happen when the key is released or it will happen repeatedly
	   when the key is held down.
	*/
	if(action == GLFW_PRESS)
		return;

	/* Custom key handling code below: 
	   For a list of keys, see: https://www.glfw.org/docs/latest/group__keys.html  */
	if(key == GLFW_KEY_SPACE)
	{
		printf("swapping rotation\n");
		isRight = !isRight;
	}
	if(key == GLFW_KEY_U)
	{
		printf("toggling up/down motion\n");
		isUpDown = !isUpDown;
	}
	if(key == GLFW_KEY_S)
	{
		printf("toggling rotation\n");
		isRotate = !isRotate;
	}
}

/** Draws the 3D scene. */
void display()
{
	/* Render the scene once for each viewport. Frequently one
	 * viewport will fill the entire screen. However, this loop will
	 * run twice for HMDs (once for the left eye and once for the
	 * right). */
	viewmat_begin_frame();
	for(int viewportID=0; viewportID<viewmat_num_viewports(); viewportID++)
	{
		//view port
		viewmat_begin_eye(viewportID);
		int viewport[4];
		viewmat_get_viewport(viewport, viewportID);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
		glEnable(GL_SCISSOR_TEST);
		glClearColor(.2,.2,.2,0); // set clear color to grey
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		glEnable(GL_DEPTH_TEST); // turn on depth testing
		kuhl_errorcheck();

		// view & projection matrix
		float viewMat[16], perspective[16];
		viewmat_get(viewMat, perspective, viewportID);

		//rotation
		float angle = fmod(glfwGetTime()*45, 360);
		dgr_setget("angle", &angle, sizeof(GLfloat));
		
		if(isRotate){
			if(isRight){
				mat4f_rotateAxis_new(rotateMat, angle, 0,1,0);
			} else {
				mat4f_rotateAxis_new(rotateMat, angle, 0,-1,0);
			}
		}
		
		//model view
		float modelview[16];
		mat4f_mult_mat4f_many(modelview, viewMat, rotateMat, NULL);

		/* Tell OpenGL which GLSL program the subsequent
		 * glUniformMatrix4fv() calls are for. */
		kuhl_errorcheck();
		glUseProgram(program);
		kuhl_errorcheck();
		
		/* Send the perspective projection matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   perspective); // value
		/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   modelview); // value
		kuhl_errorcheck();

		//draw geometry
		kuhl_geometry_draw(&roof);
		kuhl_geometry_draw(&ground);
		kuhl_geometry_draw(&poles);

		//draw models
		if (isUpDown) {
			up1 = (1+sin(glfwGetTime()+3.6))/2;
		}
		mat4f_translate_new(transMat, cowPos[0], cowPos[1]+up1, cowPos[2]);
		mat4f_mult_mat4f_many(modelview, viewMat, rotateMat, transMat, NULL);
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   modelview); // value
		kuhl_errorcheck();
		kuhl_geometry_draw(cow);
		kuhl_errorcheck();

		if (isUpDown) {
			up2 = (1+sin(glfwGetTime()+1.2))/2;
		}
		mat4f_translate_new(transMat, hippoPos[0], hippoPos[1]+up2, hippoPos[2]);
		mat4f_rotateAxis_new(turnMat, 90, 0, 1, 0);
		mat4f_mult_mat4f_many(modelview, viewMat, rotateMat, transMat, turnMat, NULL);
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   modelview); // value
		kuhl_errorcheck();
		kuhl_geometry_draw(hippo);
		kuhl_errorcheck();

		if (isUpDown) {
			up3 = (1+sin(glfwGetTime()+3.6))/2;
		}
		mat4f_translate_new(transMat, lionPos[0], lionPos[1]+up3, lionPos[2]);
		mat4f_rotateAxis_new(turnMat, -90, 0, 1, 0);
		mat4f_mult_mat4f_many(modelview, viewMat, rotateMat, transMat, turnMat, NULL);
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   modelview); // value
		kuhl_errorcheck();
		kuhl_geometry_draw(lion);
		kuhl_errorcheck();
		
		if (isUpDown) {
			up4 = (1+sin(glfwGetTime()+2.4))/2;
		}
		mat4f_translate_new(transMat, skelPos[0], skelPos[1]+up4+.4, skelPos[2]);
		mat4f_rotateAxis_new(turnMat, -90, 1, 0, 0);
		mat4f_scale_new(scaleMat, .03, .03, .03);
		float extra[16];
		mat4f_rotateAxis_new(extra, -90, 0, 1, 0);
		mat4f_mult_mat4f_many(modelview, viewMat, rotateMat, transMat, turnMat, scaleMat, extra, NULL);
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   modelview); // value
		kuhl_errorcheck();
		kuhl_geometry_draw(skel);
		kuhl_errorcheck();

		glUseProgram(0); // stop using a GLSL program.
		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

}

/**
 * Generate a flat rectangle in 3D space
 * 
 */
void init_geometryPoles(kuhl_geometry *geom, GLuint prog)
{
	kuhl_geometry_new(geom, prog, 16, GL_TRIANGLES);

	//vertices
	float off = 0.025;
	GLfloat vertexPositions[] = {-off, 0, -4, //back
	                             off, 0, -4,
	                             -off, 5, -4,
	                             off, 5, -4,
								 -off, 0, 4, //front
	                             off, 0, 4,
	                             -off, 5, 4,
	                             off, 5, 4,
								 -4-off, 0, 0, //left
	                             -4+off, 0, 0,
	                             -4-off, 5, 0,
	                             -4+off, 5, 0,
								 4-off, 0, 0, //right
	                             4+off, 0, 0,
	                             4-off, 5, 0,
	                             4+off, 5, 0,};
								 
	kuhl_geometry_attrib(geom, vertexPositions, 3, "in_Position", KG_WARN);

	//normals
	GLfloat normalData[] = {0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1};
	kuhl_geometry_attrib(geom, normalData, 3, "in_Normal", KG_WARN);
	
	//indices
	GLuint indexData[] = { 0, 1, 2,  
	                       0, 2, 3,
	                       4, 5, 6,
	                       4, 6, 7,
						   8, 9, 10,
	                       8, 10, 11,
						   12, 13, 14,
	                       12, 14, 15 };
	kuhl_geometry_indices(geom, indexData, 24);

	kuhl_errorcheck();
}

/**
 * Generate a flat hex in 3D space
 * 
 * @param height distance from y value 0
 * @param cenHeight distance center point is from outside point y values
 */
void init_geometryHex(kuhl_geometry *geom, GLuint prog, int height, int cenHeight){
	kuhl_geometry_new(geom, prog, 18, // num vertices
	                  GL_TRIANGLES); // primitive type

	GLfloat vertexPositions[54];
	float rad = 5;
	float val = 0;
	float shift = (M_PI * 2) / 6;
	for (int i = 0; i < 54; i+=9) {
		//center point
		vertexPositions[i] = 0; //x1
		vertexPositions[i+1] = height+cenHeight; //y1
		vertexPositions[i+2] = 0; //z1
		//first outer point
		vertexPositions[i+3] = rad*cos(val); //x2
		vertexPositions[i+4] = height; //y2
		vertexPositions[i+5] = rad*sin(val); //z2
		//second outer point
		vertexPositions[i+6] = rad*cos(val+shift); //x3
		vertexPositions[i+7] = height; //y3
		vertexPositions[i+8] = rad*sin(val+shift); //z3
		val += shift;
	}
								
	kuhl_geometry_attrib(geom, vertexPositions, // data
	                     3, // number of components (x,y,z)
	                     "in_Position", // GLSL variable
	                     KG_WARN); // warn if attribute is missing in GLSL program?

	/* The normals for each vertex */
	GLfloat normalData[54];
	for (int i = 0; i < 54; i+=9) {
		//calculate normal
		float norm[3];
		float v1[] = {vertexPositions[i], vertexPositions[i+1], vertexPositions[i+2]};
		float v2[] = {vertexPositions[i+3], vertexPositions[i+4], vertexPositions[i+5]};
		float v3[] = {vertexPositions[i+6], vertexPositions[i+7], vertexPositions[i+8]};
		triNormal(norm, v1, v2, v3);

		//center point
		normalData[i] = norm[0]; //x1
		normalData[i+1] = norm[1]; //y1
		normalData[i+2] = norm[2]; //z1
		//first outer point
		normalData[i+3] = norm[0]; //x2
		normalData[i+4] = norm[1]; //y2
		normalData[i+5] = norm[2]; //z2
		//second outer point
		normalData[i+6] = norm[0]; //x3
		normalData[i+7] = norm[1]; //y3
		normalData[i+8] = norm[2]; //z3

		val += shift;
	}
	kuhl_geometry_attrib(geom, normalData, 3, "in_Normal", KG_WARN);
}

int main(int argc, char** argv)
{
	kuhl_ogl_init(&argc, argv, 512, 512, 32, 4);
	glfwSetKeyCallback(kuhl_get_window(), keyboard);
	program = kuhl_create_program("triangle-shade.vert", "triangle-shade.frag");
	glUseProgram(program);
	kuhl_errorcheck();
	glUniform1i(kuhl_get_uniform("red"), 0);
	kuhl_errorcheck();
	glUseProgram(0);

	//create merry go round parts
	init_geometryHex(&roof, program, 5, 1);
	init_geometryHex(&ground, program, 0, 0);
	init_geometryPoles(&poles, program);

	// Load models
	cow = kuhl_load_model("../models/merry/cow.ply", NULL, program, NULL);
	hippo = kuhl_load_model("../models/merry/hippo.ply", NULL, program, NULL);
	lion = kuhl_load_model("../models/merry/lion.ply", NULL, program, NULL);
	skel = kuhl_load_model("../models/merry/skeleton.ply", NULL, program, NULL);

	dgr_init();     /* Initialize DGR based on config file. */
	float initCamPos[3]  = {0,0,10}; // location of camera
	float initCamLook[3] = {0,0,0}; // a point the camera is facing at
	float initCamUp[3]   = {0,1,0}; // a vector indicating which direction is up
	viewmat_init(initCamPos, initCamLook, initCamUp);
	
	//print help
	printf("U - Toggle up/down motion\nSPACE - Toggle rotation direction\nS - Toggle rotation");

	while(!glfwWindowShouldClose(kuhl_get_window()))
	{
		display();
		kuhl_errorcheck();

		/* process events (keyboard, mouse, etc) */
		glfwPollEvents();
	}
	exit(EXIT_SUCCESS);
}
