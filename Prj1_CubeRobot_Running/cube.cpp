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

// glm::mat4가 4*4 행렬을 정의할 수 있게 함.
glm::mat4 projectMat;
glm::mat4 viewMat;

// 벡터를 저장할 수 있게 함. 
glm::vec4 pvMatrixID; // vertex shader에서 mPVM으로 넘겨줄 uniform variable의 ID

GLuint pvmMatrixID;

float rotAngle = 0.0f;  // 큐브를 돌리기 위해 로테이션 각을 계산
int isDrawingCar = false;

float armAngle = 0.0f; // 상완의 회전 각도

float legAngle[2] = { 0.0f, 0.0f }; // 왼쪽과 오른쪽 다리의 각도
float kneeAngle[2] = { 0.0f, 0.0f }; // 무릎의 각도

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
	// vertex buffer object를 만들어 GPU로 전송
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

	// Load shaders and use the resulting shader program
	// shader를 로드해서 program ID로 연결해줌.
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

	pvmMatrixID = glGetUniformLocation(program, "mPVM"); // 하나의 오브젝트에 대해 동일한 transformation 적용

	// projection matrix와 view matirx
	projectMat = glm::perspective(glm::radians(65.0f), 1.0f, 0.1f, 100.0f);  // 라디안 이용.
	// Z축을 윗방향이라 설정 했을 때, Y를 증가시키면 왼쪽으로 회전. X를 증가시키면 멀어보임.
	viewMat = glm::lookAt(glm::vec3(3, 1, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
	// 처음 코드:(glm::vec3(0, 0, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0) : 0, 0, 2에서 원점을 바라봄. y축이 윗방향

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

	float legZOffset = -0.5f; // 힙 조인트의 z 좌표
	float upperLegLength = 0.6f; // 상부 다리의 길이
	float lowerLegLength = 0.4f; // 하부 다리의 길이

	// 위쪽 y, 오른쪽 x, view쪽은 z
	// 뒤로 보내려면 x를 증가시키면 됨. 오른쪽으로 가려면 y를 증가시키고, 아래쪽으로 가려면 z를 negative
	// Leg
	uppperLegPos[0] = glm::vec3(0, 0.15, -0.5); // left upper leg 
	uppperLegPos[1] = glm::vec3(0, -0.15, -0.5); // right upper leg
	lowerLegPos[0] = glm::vec3(0, 0.15, -1); // left lower leg
	lowerLegPos[1] = glm::vec3(0, -0.15, -1); // right lower leg
	// Arm
	ArmPos[0] = glm::vec3(0, 0.5, 0.2); // left upper arm (로봇 기준)
	ArmPos[1] = glm::vec3(0, -0.5, 0.2); // right upper arm

	// forearm의 회전 각도
	float forearmAngle = glm::radians(-45.0f); // -45도를 라디안으로 변환

	// body
	modelMat = glm::scale(carMat, glm::vec3(0.7, 0.8, 0.9)); // 초기 코드 (1, 0.6, 1.2), scailing, y가 몸통 가로폭
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// head
	modelMat = glm::translate(carMat, glm::vec3(0, 0, 0.7));  // 초기 코드 (0, 0, 0.2), tranlation, 순서 : Projection * View * CarMat * Translation * Scailing * vertex
	modelMat = glm::scale(modelMat, glm::vec3(0.5, 0.5, 0.4));  // 
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// 상부 다리
	for (int i = 0; i < 2; i++)
	{
		// 상부 다리 변환 행렬
		glm::mat4 modelMat_upperLeg = carMat;
		// 다리 위치로 이동
		modelMat_upperLeg = glm::translate(modelMat_upperLeg, uppperLegPos[i]);
		// 상부 다리 회전 중심을 힙 쪽으로 이동 (z 방향으로 상부 다리의 절반 길이만큼)
		float upperLegOffset = 0.5f * upperLegLength;
		modelMat_upperLeg = glm::translate(modelMat_upperLeg, glm::vec3(0, 0, upperLegOffset));
		// 상부 다리 회전 적용
		modelMat_upperLeg = glm::rotate(modelMat_upperLeg, legAngle[i], glm::vec3(0, 1, 0));
		// 회전 중심 위치 복귀
		modelMat_upperLeg = glm::translate(modelMat_upperLeg, glm::vec3(0, 0, -upperLegOffset));
		// 스케일 적용 전 행렬 저장
		glm::mat4 modelMat_upperLegNoScale = modelMat_upperLeg;
		// 상부 다리 스케일 조절
		modelMat_upperLeg = glm::scale(modelMat_upperLeg, glm::vec3(0.25, 0.25, upperLegLength));
		// 상부 다리 그리기
		pvmMat = projectMat * viewMat * modelMat_upperLeg;
		glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);

		// 하부 다리 변환 행렬
		glm::mat4 modelMat_lowerLeg = modelMat_upperLegNoScale;
		// 상부 다리의 끝(무릎 위치)으로 이동
		modelMat_lowerLeg = glm::translate(modelMat_lowerLeg, glm::vec3(0, 0, -upperLegLength));
		// 하부 다리 회전 중심을 무릎 쪽으로 이동 (z 방향으로 하부 다리의 절반 길이만큼)
		float lowerLegOffset = 0.5f * lowerLegLength;
		modelMat_lowerLeg = glm::translate(modelMat_lowerLeg, glm::vec3(0, 0, lowerLegOffset));
		// 하부 다리 회전 적용
		modelMat_lowerLeg = glm::rotate(modelMat_lowerLeg, kneeAngle[i], glm::vec3(0, 1, 0));
		// 회전 중심 위치 복귀
		modelMat_lowerLeg = glm::translate(modelMat_lowerLeg, glm::vec3(0, 0, -lowerLegOffset));
		// 하부 다리 스케일 조절
		modelMat_lowerLeg = glm::scale(modelMat_lowerLeg, glm::vec3(0.25, 0.25, lowerLegLength));
		// 하부 다리 그리기
		pvmMat = projectMat * viewMat * modelMat_lowerLeg;
		glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	}


	float zOffset = 0.2f; // 회전 중심을 얼마나 위로 올릴지 결정

	// === Left Upper Arm ===
	glm::mat4 modelMat_upperArm_Left = carMat;
	// Upper Arm 위치 이동
	modelMat_upperArm_Left = glm::translate(modelMat_upperArm_Left, ArmPos[0]);
	// 회전 중심을 z축 방향으로 위로 이동
	modelMat_upperArm_Left = glm::translate(modelMat_upperArm_Left, glm::vec3(0, 0, zOffset));
	// Upper Arm 회전 적용
	modelMat_upperArm_Left = glm::rotate(modelMat_upperArm_Left, armAngle, glm::vec3(0, 1, 0));
	// 회전 중심 복귀
	modelMat_upperArm_Left = glm::translate(modelMat_upperArm_Left, glm::vec3(0, 0, -zOffset));
	// Upper Arm 스케일 조절
	glm::mat4 modelMat_upperArm_Left_Scaled = glm::scale(modelMat_upperArm_Left, glm::vec3(0.2, 0.2, 0.5));
	// Upper Arm 그리기
	pvmMat = projectMat * viewMat * modelMat_upperArm_Left_Scaled;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// === Left Lower Arm ===
	glm::mat4 modelMat_lowerArm_Left = modelMat_upperArm_Left;
	// Upper Arm의 끝으로 이동 (Upper Arm의 길이는 스케일링 된 값 사용)
	float upperArmLength = 0.5f; // Upper Arm의 z축 스케일 값
	// Upper Arm의 끝으로 이동
	modelMat_lowerArm_Left = glm::translate(modelMat_lowerArm_Left, glm::vec3(0.2, 0, -upperArmLength));
	// Lower Arm 회전 적용 (필요한 경우)
	modelMat_lowerArm_Left = glm::rotate(modelMat_lowerArm_Left, forearmAngle, glm::vec3(0, 1, 0));
	// Lower Arm 스케일 조절
	modelMat_lowerArm_Left = glm::scale(modelMat_lowerArm_Left, glm::vec3(0.2, 0.2, 0.5));
	// Lower Arm 그리기
	pvmMat = projectMat * viewMat * modelMat_lowerArm_Left;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// === Right Upper Arm ===
	glm::mat4 modelMat_upperArm_Right = carMat;
	// Upper Arm 위치 이동
	modelMat_upperArm_Right = glm::translate(modelMat_upperArm_Right, ArmPos[1]);
	// 회전 중심을 z축 방향으로 위로 이동
	modelMat_upperArm_Right = glm::translate(modelMat_upperArm_Right, glm::vec3(0, 0, zOffset));
	// Upper Arm 회전 적용 (반대 방향)
	modelMat_upperArm_Right = glm::rotate(modelMat_upperArm_Right, armAngle, glm::vec3(0, -1, 0));
	// 회전 중심 복귀
	modelMat_upperArm_Right = glm::translate(modelMat_upperArm_Right, glm::vec3(0, 0, -zOffset));
	// Upper Arm 스케일 조절
	glm::mat4 modelMat_upperArm_Right_Scaled = glm::scale(modelMat_upperArm_Right, glm::vec3(0.2, 0.2, 0.5));
	// Upper Arm 그리기
	pvmMat = projectMat * viewMat * modelMat_upperArm_Right_Scaled;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// === Right Lower Arm ===
	glm::mat4 modelMat_lowerArm_Right = modelMat_upperArm_Right;
	// Upper Arm의 끝으로 이동
	modelMat_lowerArm_Right = glm::translate(modelMat_lowerArm_Right, glm::vec3(0.2, 0, -upperArmLength));
	// Lower Arm 회전 적용 (필요한 경우)
	modelMat_lowerArm_Right = glm::rotate(modelMat_lowerArm_Right, forearmAngle, glm::vec3(0, 1, 0));
	// Lower Arm 스케일 조절
	modelMat_lowerArm_Right = glm::scale(modelMat_lowerArm_Right, glm::vec3(0.2, 0.2, 0.5));
	// Lower Arm 그리기
	pvmMat = projectMat * viewMat * modelMat_lowerArm_Right;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

}


void display(void)
{
	glm::mat4 worldMat, pvmMat;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 회전을 하는 matrix. 기존 matrix를 넘김.
	// worldMat = glm::rotate(glm::mat4(1.0f), rotAngle, glm::vec3(1.0f, 1.0f, 0.0f)); // 이전 matrix : glm::mat4(1.0f), 회전축 : glm::vec3(1.0f, 1.0f, 0.0f)

	// 회전 변환을 제거하여 worldMat을 단위 행렬로 설정.
	worldMat = glm::mat4(1.0f); // 단위 행렬
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
		float frequency = 8.0f;  // 속도 조절

		// 팔 각도 계산
		armAngle = glm::radians(maxAngleDegrees * sin(frequency * timeInSeconds));

		// 다리 각도 계산
		float maxLegAngleDegrees = 45.0f;

		// 왼쪽과 오른쪽 다리의 각도를 반대 위상으로 설정
		legAngle[0] = glm::radians(maxLegAngleDegrees * sin(frequency * timeInSeconds));
		legAngle[1] = -legAngle[0];

		float maxKneeAngleDegrees = 60.0f; // 최대 무릎 각도를 45도로 설정

		// 절대값을 사용하여 무릎이 한 방향으로만 회전하도록 설정
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
			glm::vec3(0, 0, 0),    // 바라보는 지점
			glm::vec3(0, 0, 1));   // 업 벡터
		glutPostRedisplay();
		break;
	case '2':
		// Over-the-shoulder view
		viewMat = glm::lookAt(
			glm::vec3(1, -1, 1),   // over the shoulder view
			glm::vec3(0, 0, 0),    // 바라보는 지점
			glm::vec3(0, 0, 2));   // 업 벡터
		glutPostRedisplay();
		break;
	case '3':
		// Front view
		viewMat = glm::lookAt(
			glm::vec3(3, 0, 0.5),    // front view
			glm::vec3(0, 0, 0),    // 바라보는 지점
			glm::vec3(0, 0, 1));   // 업 벡터
		glutPostRedisplay();
		break;
	}
}

//----------------------------------------------------------------------------

void resize(int w, int h)
{
	float ratio = (float)w / (float)h;
	glViewport(0, 0, w, h);

	projectMat = glm::perspective(glm::radians(65.0f), ratio, 0.1f, 100.0f);  // 이 코드가 없으면 창 크기에 따라 aspect ratio가 바뀌게 됨.

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