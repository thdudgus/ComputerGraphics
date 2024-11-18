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
#include "texture.hpp"
#include "sphere.h"

GLuint SPHvao;
GLuint vao;

// glm::mat4가 4*4 행렬을 정의할 수 있게 함.
glm::mat4 projectMat;
glm::mat4 viewMat;
glm::mat4 modelMat;

// sphere
glm::mat4 SPHprojectMat;             // 투영 행렬
glm::mat4 SPHviewMat;                // 뷰 행렬
glm::mat4 SPHmodelMat = glm::mat4(1.0f); // 모델 행렬, 기본값은 단위 행렬
GLuint SPHprojectMatrixID;           // 투영 행렬의 셰이더 위치 ID
GLuint SPHviewMatrixID;              // 뷰 행렬의 셰이더 위치 ID
GLuint SPHmodelMatrixID;             // 모델 행렬의 셰이더 위치 ID
Sphere SPHsphere(50, 50);            // 구체 생성, 50x50의 세부 수준으로 초기화
GLuint SPHTexture;
GLuint Texture;

// 벡터를 저장할 수 있게 함. 
glm::vec4 pvMatrixID; // vertex shader에서 mPVM으로 넘겨줄 uniform variable의 ID

GLuint pvmMatrixID;
GLuint projectMatrixID;           // 투영 행렬의 셰이더 위치 ID
GLuint viewMatrixID;              // 뷰 행렬의 셰이더 위치 ID
GLuint modelMatrixID;

float updown = 0.0f;

float rotAngle = 0.0f;  // 큐브를 돌리기 위해 로테이션 각을 계산
int isDrawingCar = false;

float armAngle = 0.0f; // upper arm의 회전 각도

float legAngle[2] = { 0.0f, 0.0f }; // 왼쪽과 오른쪽 다리의 각도
float kneeAngle[2] = { 0.0f, 0.0f }; // 무릎의 각도

typedef glm::vec4  color4;
typedef glm::vec4  point4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

glm::vec2 texCoords[NumVertices];  // texture mapping을 위한 벡터
// texture 이미지가 2D 이미지라 2차원벡터
glm::vec4 normals[NumVertices];  // lighting을 위한 벡터
// 각 vertex마다 색을 입히기 위함.


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
    // normal vector 계산.
    glm::vec4 normal = glm::vec4(glm::cross(glm::vec3(vertices[b] - vertices[a]), glm::vec3(vertices[d] - vertices[a])), 0);
    // 각 vertex마다 업데이트.
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a];  normals[Index] = normal;  texCoords[Index] = glm::vec2(0, 0); Index++;
    colors[Index] = vertex_colors[b]; points[Index] = vertices[b];  normals[Index] = normal;  texCoords[Index] = glm::vec2(1, 0); Index++;
    colors[Index] = vertex_colors[c]; points[Index] = vertices[c];  normals[Index] = normal;  texCoords[Index] = glm::vec2(1, 1); Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a];  normals[Index] = normal;  texCoords[Index] = glm::vec2(0, 0); Index++;
    colors[Index] = vertex_colors[c]; points[Index] = vertices[c];  normals[Index] = normal;  texCoords[Index] = glm::vec2(1, 1); Index++;
    colors[Index] = vertex_colors[d]; points[Index] = vertices[d];  normals[Index] = normal;  texCoords[Index] = glm::vec2(0, 1); Index++;
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

    GLuint shadeModeID;               // 셰이딩 모드의 셰이더 위치 ID
    GLuint textureModeID;             // 텍스처 모드의 셰이더 위치 ID

    // Create a vertex array object
    // VAO 생성

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create and initialize a buffer object
    // vertex buffer object를 만들어 GPU로 전송
    // VPO 만들어 gpu로 전송
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals) + sizeof(texCoords), NULL, GL_STATIC_DRAW);
    //glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    //glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

    // glBufferSubData 함수는 glBufferData로 할당된 버퍼에 데이터를 업로드
    // 0 위치에 sphere.verts 배열 데이터를 업로드
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    // 정점 배열 뒤쪽 vertSize 위치에 sphere.normals 배열 데이터를 업로드
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals);
    // 법선 벡터 배열 뒤쪽 vertSize + normalSize 위치에 sphere.texCoords 배열 데이터를 업로드
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals), sizeof(texCoords), texCoords);


    // Load shaders and use the resulting shader program
    // shader를 로드해서 program ID로 연결해줌.
    GLuint program = InitShader("src/vshader.glsl", "src/fshader.glsl");
    glUseProgram(program);

    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation(program, "vPosition"); // vertex position
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vNormal = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

    //GLuint vColor = glGetAttribLocation(program, "vColor");  // vertex color
    //glEnableVertexAttribArray(vColor);
    //glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points) + sizeof(normals)));

    // 투영 행렬을 설정하여 셰이더에 전달
    projectMatrixID = glGetUniformLocation(program, "mProject");
    projectMat = glm::perspective(glm::radians(65.0f), 1.0f, 0.1f, 100.0f);
    glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);

    // 뷰 행렬 설정 및 셰이더에 전달
    viewMatrixID = glGetUniformLocation(program, "mView");
    viewMat = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);

    // 모델 행렬 설정 및 셰이더에 전달
    modelMatrixID = glGetUniformLocation(program, "mModel");
    modelMat = glm::mat4(1.0f);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);

    pvmMatrixID = glGetUniformLocation(program, "mPVM"); // 하나의 오브젝트에 대해 동일한 transformation 적용
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &(projectMat * viewMat * modelMat)[0][0]);

    // 텍스처 로드 및 적용
    Texture = loadBMP_custom("stone_surface_texture.bmp");
    GLuint TextureID = glGetUniformLocation(program, "sphereTexture");

    // 텍스처를 텍스처 유닛 0에 바인딩하고 셰이더에 전달
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture);
    glUniform1i(TextureID, 0);


    // projection matrix와 view matirx
    projectMat = glm::perspective(glm::radians(65.0f), 1.0f, 0.1f, 100.0f);  // 라디안 이용.
    // Z축을 윗방향이라 설정 했을 때, Y를 증가시키면 왼쪽으로 회전. X를 증가시키면 멀어보임.
    viewMat = glm::lookAt(glm::vec3(3, 1, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
    // 처음 코드:(glm::vec3(0, 0, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0) : 0, 0, 2에서 원점을 바라봄. y축이 윗방향

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    // sphere

    glGenVertexArrays(1, &SPHvao);
    glBindVertexArray(SPHvao);

    GLuint SPHbuffer;
    glGenBuffers(1, &SPHbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, SPHbuffer);

    int SPHvertSize = sizeof(SPHsphere.verts[0]) * SPHsphere.verts.size();
    int SPHnormalSize = sizeof(SPHsphere.normals[0]) * SPHsphere.normals.size();
    int SPHtexSize = sizeof(SPHsphere.texCoords[0]) * SPHsphere.texCoords.size();
    glBufferData(GL_ARRAY_BUFFER, SPHvertSize + SPHnormalSize + SPHtexSize, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, SPHvertSize, SPHsphere.verts.data());
    glBufferSubData(GL_ARRAY_BUFFER, SPHvertSize, SPHnormalSize, SPHsphere.normals.data());
    glBufferSubData(GL_ARRAY_BUFFER, SPHvertSize + SPHnormalSize, SPHtexSize, SPHsphere.texCoords.data());

    GLuint SPHprogram = InitShader("src/vshader.glsl", "src/fshader.glsl");
    glUseProgram(SPHprogram);
    GLuint SPHvPosition = glGetAttribLocation(SPHprogram, "vPosition");
    glEnableVertexAttribArray(SPHvPosition);
    glVertexAttribPointer(SPHvPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint SPHvNormal = glGetAttribLocation(SPHprogram, "vNormal");
    glEnableVertexAttribArray(SPHvNormal);
    glVertexAttribPointer(SPHvNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHvertSize));

    GLuint SPHvTexCoord = glGetAttribLocation(SPHprogram, "vTexCoord");
    glEnableVertexAttribArray(SPHvTexCoord);
    glVertexAttribPointer(SPHvTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHvertSize + SPHnormalSize));

    SPHprojectMatrixID = glGetUniformLocation(SPHprogram, "mProject");
    SPHprojectMat = glm::perspective(glm::radians(65.0f), 1.0f, 0.1f, 100.0f);
    glUniformMatrix4fv(SPHprojectMatrixID, 1, GL_FALSE, &SPHprojectMat[0][0]);

    SPHviewMatrixID = glGetUniformLocation(SPHprogram, "mView");
    SPHviewMat = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(SPHviewMatrixID, 1, GL_FALSE, &SPHviewMat[0][0]);

    SPHmodelMatrixID = glGetUniformLocation(SPHprogram, "mModel");
    SPHmodelMat = glm::mat4(1.0f);
    glUniformMatrix4fv(SPHmodelMatrixID, 1, GL_FALSE, &SPHmodelMat[0][0]);

    SPHTexture = loadBMP_custom("wood.bmp");
    GLuint SPHTextureID = glGetUniformLocation(SPHprogram, "sphereTexture");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, SPHTexture);
    glUniform1i(SPHTextureID, 0);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

//----------------------------------------------------------------------------

void drawCar(glm::mat4 carMat)
{
    glm::mat4 pvmMat;
    glm::vec3 uppperLegPos[2];
    glm::vec3 ArmPos[2];

    float legZOffset = -0.5f; // 힙 조인트의 z 좌표
    float upperLegLength = 0.6f; // 상부 다리의 길이
    float lowerLegLength = 0.4f; // 하부 다리의 길이

    // 위쪽 y, 오른쪽 x, view쪽은 z
    // 뒤로 보내려면 x를 증가시키면 됨. 오른쪽으로 가려면 y를 증가시키고, 아래쪽으로 가려면 z를 negative
    // Leg position
    uppperLegPos[0] = glm::vec3(0, 0.15, -0.7); // left upper leg 
    uppperLegPos[1] = glm::vec3(0, -0.15, -0.7); // right upper leg
    // Arm position
    ArmPos[0] = glm::vec3(0, 0.5, 0.1); // left upper arm (로봇 기준)
    ArmPos[1] = glm::vec3(0, -0.5, 0.1); // right upper arm

    // forearm의 회전 각도
    float forearmAngle = glm::radians(-45.0f); // -45도를 라디안으로 변환

    // body
    modelMat = glm::translate(carMat, glm::vec3(0, 0, 0 + updown * 0.5)); // 위아래 이동 추가
    modelMat = glm::scale(modelMat, glm::vec3(0.7, 0.8, 0.9)); // 초기 코드 (1, 0.6, 1.2), scailing, y가 몸통 가로폭
    pvmMat = projectMat * viewMat * modelMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // head
    glBindTexture(GL_TEXTURE_2D, SPHTexture);  // vao 바인드 적용해서 아래 코드들로 그리고
    glBindVertexArray(SPHvao);
    modelMat = glm::translate(carMat, glm::vec3(0, 0, 0.7 + updown * 0.5));  // 초기 코드 (0, 0, 0.2), tranlation, 순서 : Projection * View * CarMat * Translation * Scailing * vertex
    modelMat = glm::scale(modelMat, glm::vec3(0.4, 0.4, 0.4));  // 
    pvmMat = projectMat * viewMat * modelMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, SPHsphere.verts.size());
    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, Texture);   // vao 바인드 해제.

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

    // 자동차 렌더링
    worldMat = glm::mat4(1.0f);
    drawCar(worldMat);

    // 구체 렌더링
    //glBindVertexArray(SPHvao); // 구체의 VAO를 바인딩
    //glUniformMatrix4fv(SPHmodelMatrixID, 1, GL_FALSE, &SPHmodelMat[0][0]);
    //glDrawArrays(GL_TRIANGLES, 0, SPHsphere.verts.size());


    // VAO를 자동차로 다시 설정
    glBindVertexArray(vao); // 자동차의 VAO를 다시 바인딩
    glutSwapBuffers();
}

void SPHresize(int w, int h)
{
    float SPHratio = (float)w / (float)h;
    glViewport(0, 0, w, h);

    SPHprojectMat = glm::perspective(glm::radians(65.0f), SPHratio, 0.1f, 100.0f);
    glUniformMatrix4fv(SPHprojectMatrixID, 1, GL_FALSE, &SPHprojectMat[0][0]);
    glutPostRedisplay();
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
        float maxAngleDegrees = 60.0f;  // 팔 최대 각도
        float frequency = 10.0f;  // 속도 조절

        // 팔 각도 계산
        armAngle = glm::radians(maxAngleDegrees * sin(frequency * timeInSeconds));

        // 다리 최대 각도 계산
        float maxLegAngleDegrees = 60.0f;

        // 왼쪽과 오른쪽 다리의 각도를 반대 위상으로 설정
        legAngle[0] = glm::radians(maxLegAngleDegrees * sin(frequency * timeInSeconds));
        legAngle[1] = -legAngle[0];

        float maxKneeAngleDegrees = 60.0f; // 최대 무릎 각도를 45도로 설정

        // 절대값을 사용하여 무릎이 한 방향으로만 회전하도록 설정
        kneeAngle[0] = glm::radians(maxKneeAngleDegrees * abs(sin(frequency * timeInSeconds)));
        kneeAngle[1] = glm::radians(maxKneeAngleDegrees * abs(sin(frequency * timeInSeconds + glm::pi<float>())));

        // 머리 위 아래 계산
        float headAmplitude = 0.1f; // Maximum displacement
        updown = headAmplitude * sin(frequency * timeInSeconds);

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
            glm::vec3(-3, 1, 1),   // over the shoulder view
            glm::vec3(0, 0, 0),    // 바라보는 지점
            glm::vec3(0, 0, 1));   // 업 벡터
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
    glutReshapeFunc(SPHresize);
    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}