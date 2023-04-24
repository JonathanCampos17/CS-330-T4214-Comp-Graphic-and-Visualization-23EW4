#pragma once


#include <GLEW/include/GL/glew.h>

#include <glm/glm.hpp>

class Meshes
{
	// Stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbos[2];     // Handles for the vertex buffer objects
		GLuint nVertices;	// Number of vertices for the mesh
		GLuint nIndices;    // Number of indices for the mesh
	};

public:

	GLMesh gTorusMesh;
	GLMesh gCylinderMesh;
	GLMesh gBoxMesh;
	GLMesh gPyramid4Mesh;
	GLMesh gPlaneMesh;
	GLMesh gSphereMesh;

public:
	void CreateMeshes();
	void DestroyMeshes();

private:

	void UCreateCylinderMesh(GLMesh& mesh);
	void UCreateBoxMesh(GLMesh& mesh);
	void UCreatePlaneMesh(GLMesh& mesh);
	void UCreateTorusMesh(GLMesh& mesh);
	void UCreatePyramid4Mesh(GLMesh& mesh);
	void UCreateSphereMesh(GLMesh& mesh);

	void UDestroyMesh(GLMesh& mesh);


	void CalculateTriangleNormal(glm::vec3 px, glm::vec3 py, glm::vec3 pz);
};