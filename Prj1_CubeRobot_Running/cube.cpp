//
// Display a color cube
//
// Colors are assigned to each vertex and then the rasterizer interpolates
//   those colors across the triangles.  We us an orthographic projection
//   as the default projetion.

#include "cube.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

// glm::mat4�� 4*4 ����� ������ �� �ְ� ��.
glm::mat4 projectMat;
glm::mat4 viewMat;

// ���͸� ������ �� �ְ� ��. 
glm::vec4 pvMatrixID; // vertex shader���� mPVM���� �Ѱ��� uniform variable�� ID

GLuint pvmMatrixID;

float rotAngle = 0.0f;  // ť�긦 ������ ���� �����̼� ���� ���
int isDrawingCar = false;

float armAngle = 0.0f; // ����� ȸ�� ����

float legAngle[2] = { 0.0f, 0.0f }; // ���ʰ� ������ �ٸ��� ����
float kneeAngle[2] = { 0.0f, 0.0f }; // ������ ����

typedef glm::vec4  color4;
typedef glm::vec4  point4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
	point4(-0.5, -0.5, 0.5, 1.0),
	point4(-0.5, 0.5, 0.5, 1.0),
	point4(0.5, 0.5, 0.5, 1.0),
	point4(0.5, -0.5, 0.5, 1.0),
	point4(-0.5, -0.5, -0.5, 1.0),
	point4(-0.5, 0.5, -0.5, 1.0),
	point4(0.5, 0.5, -0.5, 1.0),
	point4(0.5, -0.5, -0.5, 1.0)
};

// RGBA colors
color4 vertex_colors[8] = {
	color4(0.0, 0.0, 0.0, 1.0),  // black
	color4(0.0, 1.0, 1.0, 1.0),   // cyan
	color4(1.0, 0.0, 1.0, 1.0),  // magenta
	color4(1.0, 1.0, 0.0, 1.0),  // yellow
	color4(1.0, 0.0, 0.0, 1.0),  // red
	color4(0.0, 1.0, 0.0, 1.0),  // green
	color4(0.0, 0.0, 1.0, 1.0),  // blue
	color4(1.0, 1.0, 1.0, 1.0)  // white
};

//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices
int Index = 0;
void
quad(int a, int b, int c, int d)
{
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a];  Index++;
	colors[Index] = vertex_colors[b]; points[Index] = vertices[b];  Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c];  Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a];  Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c];  Index++;
	colors[Index] = vertex_colors[d]; points[Index] = vertices[d];  Index++;
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube()
{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);
}

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
	colorcube();

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	// vertex buffer object�� ����� GPU�� ����
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

	// Load shaders and use the resulting shader program
	// shader�� �ε��ؼ� program ID�� ��������.
	GLuint program = InitShader("src/vshader.glsl", "src/fshader.glsl");
	glUseProgram(program);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition"); // vertex position
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program, "vColor");  // vertex color
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points)));

	pvmMatrixID = glGetUniformLocation(program, "mPVM"); // �ϳ��� ������Ʈ�� ���� ������ transformation ����

	// projection matrix�� view matirx
	projectMat = glm::perspective(glm::radians(65.0f), 1.0f, 0.1f, 100.0f);  // ���� �̿�.
	// Z���� �������̶� ���� ���� ��, Y�� ������Ű�� �������� ȸ��. X�� ������Ű�� �־��.
	viewMat = glm::lookAt(glm::vec3(3, 1, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
	// ó�� �ڵ�:(glm::vec3(0, 0, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0) : 0, 0, 2���� ������ �ٶ�. y���� ������

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

//----------------------------------------------------------------------------

void drawCar(glm::mat4 carMat)
{
	glm::mat4 modelMat, pvmMat;
	glm::vec3 uppperLegPos[2];
	glm::vec3 lowerLegPos[2];
	glm::vec3 ArmPos[2];

	float legZOffset = -0.5f; // �� ����Ʈ�� z ��ǥ
	float upperLegLength = 0.6f; // ��� �ٸ��� ����
	float lowerLegLength = 0.4f; // �Ϻ� �ٸ��� ����

	// ���� y, ������ x, view���� z
	// �ڷ� �������� x�� ������Ű�� ��. ���������� ������ y�� ������Ű��, �Ʒ������� ������ z�� negative
	// Leg
	uppperLegPos[0] = glm::vec3(0, 0.15, -0.5); // left upper leg 
	uppperLegPos[1] = glm::vec3(0, -0.15, -0.5); // right upper leg
	lowerLegPos[0] = glm::vec3(0, 0.15, -1); // left lower leg
	lowerLegPos[1] = glm::vec3(0, -0.15, -1); // right lower leg
	// Arm
	ArmPos[0] = glm::vec3(0, 0.5, 0.2); // left upper arm (�κ� ����)
	ArmPos[1] = glm::vec3(0, -0.5, 0.2); // right upper arm

	// forearm�� ȸ�� ����
	float forearmAngle = glm::radians(-45.0f); // -45���� �������� ��ȯ

	// body
	modelMat = glm::scale(carMat, glm::vec3(0.7, 0.8, 0.9)); // �ʱ� �ڵ� (1, 0.6, 1.2), scailing, y�� ���� ������
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// head
	modelMat = glm::translate(carMat, glm::vec3(0, 0, 0.7));  // �ʱ� �ڵ� (0, 0, 0.2), tranlation, ���� : Projection * View * CarMat * Translation * Scailing * vertex
	modelMat = glm::scale(modelMat, glm::vec3(0.5, 0.5, 0.4));  // 
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// ��� �ٸ�
	for (int i = 0; i < 2; i++)
	{
		// ��� �ٸ� ��ȯ ���
		glm::mat4 modelMat_upperLeg = carMat;
		// �ٸ� ��ġ�� �̵�
		modelMat_upperLeg = glm::translate(modelMat_upperLeg, uppperLegPos[i]);
		// ��� �ٸ� ȸ�� �߽��� �� ������ �̵� (z �������� ��� �ٸ��� ���� ���̸�ŭ)
		float upperLegOffset = 0.5f * upperLegLength;
		modelMat_upperLeg = glm::translate(modelMat_upperLeg, glm::vec3(0, 0, upperLegOffset));
		// ��� �ٸ� ȸ�� ����
		modelMat_upperLeg = glm::rotate(modelMat_upperLeg, legAngle[i], glm::vec3(0, 1, 0));
		// ȸ�� �߽� ��ġ ����
		modelMat_upperLeg = glm::translate(modelMat_upperLeg, glm::vec3(0, 0, -upperLegOffset));
		// ������ ���� �� ��� ����
		glm::mat4 modelMat_upperLegNoScale = modelMat_upperLeg;
		// ��� �ٸ� ������ ����
		modelMat_upperLeg = glm::scale(modelMat_upperLeg, glm::vec3(0.25, 0.25, upperLegLength));
		// ��� �ٸ� �׸���
		pvmMat = projectMat * viewMat * modelMat_upperLeg;
		glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);

		// �Ϻ� �ٸ� ��ȯ ���
		glm::mat4 modelMat_lowerLeg = modelMat_upperLegNoScale;
		// ��� �ٸ��� ��(���� ��ġ)���� �̵�
		modelMat_lowerLeg = glm::translate(modelMat_lowerLeg, glm::vec3(0, 0, -upperLegLength));
		// �Ϻ� �ٸ� ȸ�� �߽��� ���� ������ �̵� (z �������� �Ϻ� �ٸ��� ���� ���̸�ŭ)
		float lowerLegOffset = 0.5f * lowerLegLength;
		modelMat_lowerLeg = glm::translate(modelMat_lowerLeg, glm::vec3(0, 0, lowerLegOffset));
		// �Ϻ� �ٸ� ȸ�� ����
		modelMat_lowerLeg = glm::rotate(modelMat_lowerLeg, kneeAngle[i], glm::vec3(0, 1, 0));
		// ȸ�� �߽� ��ġ ����
		modelMat_lowerLeg = glm::translate(modelMat_lowerLeg, glm::vec3(0, 0, -lowerLegOffset));
		// �Ϻ� �ٸ� ������ ����
		modelMat_lowerLeg = glm::scale(modelMat_lowerLeg, glm::vec3(0.25, 0.25, lowerLegLength));
		// �Ϻ� �ٸ� �׸���
		pvmMat = projectMat * viewMat * modelMat_lowerLeg;
		glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	}


	float zOffset = 0.2f; // ȸ�� �߽��� �󸶳� ���� �ø��� ����

	// === Left Upper Arm ===
	glm::mat4 modelMat_upperArm_Left = carMat;
	// Upper Arm ��ġ �̵�
	modelMat_upperArm_Left = glm::translate(modelMat_upperArm_Left, ArmPos[0]);
	// ȸ�� �߽��� z�� �������� ���� �̵�
	modelMat_upperArm_Left = glm::translate(modelMat_upperArm_Left, glm::vec3(0, 0, zOffset));
	// Upper Arm ȸ�� ����
	modelMat_upperArm_Left = glm::rotate(modelMat_upperArm_Left, armAngle, glm::vec3(0, 1, 0));
	// ȸ�� �߽� ����
	modelMat_upperArm_Left = glm::translate(modelMat_upperArm_Left, glm::vec3(0, 0, -zOffset));
	// Upper Arm ������ ����
	glm::mat4 modelMat_upperArm_Left_Scaled = glm::scale(modelMat_upperArm_Left, glm::vec3(0.2, 0.2, 0.5));
	// Upper Arm �׸���
	pvmMat = projectMat * viewMat * modelMat_upperArm_Left_Scaled;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// === Left Lower Arm ===
	glm::mat4 modelMat_lowerArm_Left = modelMat_upperArm_Left;
	// Upper Arm�� ������ �̵� (Upper Arm�� ���̴� �����ϸ� �� �� ���)
	float upperArmLength = 0.5f; // Upper Arm�� z�� ������ ��
	// Upper Arm�� ������ �̵�
	modelMat_lowerArm_Left = glm::translate(modelMat_lowerArm_Left, glm::vec3(0.2, 0, -upperArmLength));
	// Lower Arm ȸ�� ���� (�ʿ��� ���)
	modelMat_lowerArm_Left = glm::rotate(modelMat_lowerArm_Left, forearmAngle, glm::vec3(0, 1, 0));
	// Lower Arm ������ ����
	modelMat_lowerArm_Left = glm::scale(modelMat_lowerArm_Left, glm::vec3(0.2, 0.2, 0.5));
	// Lower Arm �׸���
	pvmMat = projectMat * viewMat * modelMat_lowerArm_Left;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// === Right Upper Arm ===
	glm::mat4 modelMat_upperArm_Right = carMat;
	// Upper Arm ��ġ �̵�
	modelMat_upperArm_Right = glm::translate(modelMat_upperArm_Right, ArmPos[1]);
	// ȸ�� �߽��� z�� �������� ���� �̵�
	modelMat_upperArm_Right = glm::translate(modelMat_upperArm_Right, glm::vec3(0, 0, zOffset));
	// Upper Arm ȸ�� ���� (�ݴ� ����)
	modelMat_upperArm_Right = glm::rotate(modelMat_upperArm_Right, armAngle, glm::vec3(0, -1, 0));
	// ȸ�� �߽� ����
	modelMat_upperArm_Right = glm::translate(modelMat_upperArm_Right, glm::vec3(0, 0, -zOffset));
	// Upper Arm ������ ����
	glm::mat4 modelMat_upperArm_Right_Scaled = glm::scale(modelMat_upperArm_Right, glm::vec3(0.2, 0.2, 0.5));
	// Upper Arm �׸���
	pvmMat = projectMat * viewMat * modelMat_upperArm_Right_Scaled;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// === Right Lower Arm ===
	glm::mat4 modelMat_lowerArm_Right = modelMat_upperArm_Right;
	// Upper Arm�� ������ �̵�
	modelMat_lowerArm_Right = glm::translate(modelMat_lowerArm_Right, glm::vec3(0.2, 0, -upperArmLength));
	// Lower Arm ȸ�� ���� (�ʿ��� ���)
	modelMat_lowerArm_Right = glm::rotate(modelMat_lowerArm_Right, forearmAngle, glm::vec3(0, 1, 0));
	// Lower Arm ������ ����
	modelMat_lowerArm_Right = glm::scale(modelMat_lowerArm_Right, glm::vec3(0.2, 0.2, 0.5));
	// Lower Arm �׸���
	pvmMat = projectMat * viewMat * modelMat_lowerArm_Right;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

}


void display(void)
{
	glm::mat4 worldMat, pvmMat;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ȸ���� �ϴ� matrix. ���� matrix�� �ѱ�.
	// worldMat = glm::rotate(glm::mat4(1.0f), rotAngle, glm::vec3(1.0f, 1.0f, 0.0f)); // ���� matrix : glm::mat4(1.0f), ȸ���� : glm::vec3(1.0f, 1.0f, 0.0f)

	// ȸ�� ��ȯ�� �����Ͽ� worldMat�� ���� ��ķ� ����.
	worldMat = glm::mat4(1.0f); // ���� ���
	drawCar(worldMat);

	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void idle()
{
	static int prevTime = glutGet(GLUT_ELAPSED_TIME);
	int currTime = glutGet(GLUT_ELAPSED_TIME);

	if (abs(currTime - prevTime) >= 20)
	{
		float t = currTime - prevTime;
		rotAngle += glm::radians(t * 360.0f / 10000.0f);

		float timeInSeconds = currTime / 1000.0f;
		float maxAngleDegrees = 60.0f;
		float frequency = 8.0f;  // �ӵ� ����

		// �� ���� ���
		armAngle = glm::radians(maxAngleDegrees * sin(frequency * timeInSeconds));

		// �ٸ� ���� ���
		float maxLegAngleDegrees = 45.0f;

		// ���ʰ� ������ �ٸ��� ������ �ݴ� �������� ����
		legAngle[0] = glm::radians(maxLegAngleDegrees * sin(frequency * timeInSeconds));
		legAngle[1] = -legAngle[0];

		float maxKneeAngleDegrees = 60.0f; // �ִ� ���� ������ 45���� ����

		// ���밪�� ����Ͽ� ������ �� �������θ� ȸ���ϵ��� ����
		kneeAngle[0] = glm::radians(maxKneeAngleDegrees * abs(sin(frequency * timeInSeconds)));
		kneeAngle[1] = glm::radians(maxKneeAngleDegrees * abs(sin(frequency * timeInSeconds + glm::pi<float>())));

		prevTime = currTime;
		glutPostRedisplay();
	}
}


//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033:  // Escape key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	case '1':
		// Side view
		viewMat = glm::lookAt(
			glm::vec3(0, -3, 0),   // side view
			glm::vec3(0, 0, 0),    // �ٶ󺸴� ����
			glm::vec3(0, 0, 1));   // �� ����
		glutPostRedisplay();
		break;
	case '2':
		// Over-the-shoulder view
		viewMat = glm::lookAt(
			glm::vec3(1, -1, 1),   // over the shoulder view
			glm::vec3(0, 0, 0),    // �ٶ󺸴� ����
			glm::vec3(0, 0, 2));   // �� ����
		glutPostRedisplay();
		break;
	case '3':
		// Front view
		viewMat = glm::lookAt(
			glm::vec3(3, 0, 0.5),    // front view
			glm::vec3(0, 0, 0),    // �ٶ󺸴� ����
			glm::vec3(0, 0, 1));   // �� ����
		glutPostRedisplay();
		break;
	}
}

//----------------------------------------------------------------------------

void resize(int w, int h)
{
	float ratio = (float)w / (float)h;
	glViewport(0, 0, w, h);

	projectMat = glm::perspective(glm::radians(65.0f), ratio, 0.1f, 100.0f);  // �� �ڵ尡 ������ â ũ�⿡ ���� aspect ratio�� �ٲ�� ��.

	glutPostRedisplay();
}

//----------------------------------------------------------------------------

int
main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("Color Car");

	glewInit();
	glutKeyboardFunc(keyboard);
	init();

	glutDisplayFunc(display);

	glutReshapeFunc(resize);
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}