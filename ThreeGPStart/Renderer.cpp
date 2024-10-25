#include "Renderer.h"
#include "Camera.h"

Renderer::Renderer()
{
}

// On exit must clean up any OpenGL resources e.g. the program, the buffers
Renderer::~Renderer()
{
	// TODO: clean up any memory used including OpenGL objects via glDelete* calls
	glDeleteProgram(m_program);
	glDeleteBuffers(1, &m_VAO);
}

// Use IMGUI for a simple on screen GUI
void Renderer::DefineGUI()
{
	// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	{
		ImGui::Begin("RTG");                    // Create a window called "RTG" and append into it.

		ImGui::Text("Visibility.");             // Display some text (you can use a format strings too)	

		ImGui::Checkbox("Wireframe", &m_wireframe);	// A checkbox linked to a member variable

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::End();
	}
}

// Load, compile and link the shaders and create a program object to host them
bool Renderer::CreateProgram()
{
	// Create a new program (returns a unqiue id)
	m_program = glCreateProgram();

	// Load and create vertex and fragment shaders
	GLuint vertex_shader{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/vertex_shader.glsl") };
	GLuint fragment_shader{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/fragment_shader.glsl") };
	if (vertex_shader == 0 || fragment_shader == 0)
		return false;

	// Attach the vertex shader to this program (copies it)
	glAttachShader(m_program, vertex_shader);

	// The attibute 0 maps to the input stream "vertex_position" in the vertex shader
	// Not needed if you use (location=0) in the vertex shader itself
	//glBindAttribLocation(m_program, 0, "vertex_position");

	// Attach the fragment shader (copies it)
	glAttachShader(m_program, fragment_shader);

	glLinkProgram(m_program);

	// Done with the originals of these as we have made copies
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	// Link the shaders, checking for errors
	if (!Helpers::LinkProgramShaders(m_program))
		return false;

	return !Helpers::CheckForGLError();
}

// Load / create geometry into OpenGL buffers	
bool Renderer::InitialiseGeometry()
{
	// Load and compile shaders into m_program
	if (!CreateProgram())
		return false;

	// Define cube vertices (8 vertices)
	float vertices[] = {
		-0.5f, -0.5f, -0.5f, // 0
		 0.5f, -0.5f, -0.5f, // 1
		 0.5f,  0.5f, -0.5f, // 2
		-0.5f,  0.5f, -0.5f, // 3
		-0.5f, -0.5f,  0.5f, // 4
		 0.5f, -0.5f,  0.5f, // 5
		 0.5f,  0.5f,  0.5f, // 6
		-0.5f,  0.5f,  0.5f  // 7  

	};

	// Define cube indices (12 triangles)
	GLuint indices[] = {
		0, 1, 2, 2, 3, 0, // front
		1, 5, 6, 6, 2, 1, // right
		5, 4, 7, 7, 6, 5, // back
		4, 0, 3, 3, 7, 4, // left
		3, 2, 6, 6, 7, 3, // top
		4, 5, 1, 1, 0, 4  // bottom  

	};

	// Create VBO, EBO and VAO
	GLuint VBO, EBO, m_VAO;
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &m_VAO);

	// Bind VAO
	glBindVertexArray(m_VAO);

	// Bind  and fill VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//CLEAR BIND
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Bind and fill EBO  

	

	//BIND ELEMENTS!!!
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	//CLEAR BIND
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
	//BIND VAO
		//glBindBuffer(GL_ARRAY_BUFFER, VAO);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Set vertex attribute pointers

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Unbind VAO
	glBindVertexArray(0);

	// Store VAO and number of elements for rendering
	m_numElements = sizeof(indices);
	//Not hardcode those!

	// Check for errors
	Helpers::CheckForGLError();

	return true;
}

// Render the scene. Passed the delta time since last called.
void Renderer::Render(const Helpers::Camera& camera, float deltaTime)
{
	/*
	// Configure pipeline settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Wireframe mode controlled by ImGui
	if (m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Clear buffers from previous frame
	glClearColor(0.0f, 0.0f, 0.0f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO: Compute viewport and projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	
	//glm::mat4 view = glm::lookAt(camera.GetPosition(), camera.GetLookVector(), camera.GetUpVector());
		
	//glDrawElements(GL_TRIANGLES, m_numElements, GL_UNSIGNED_INT, 0);

	// TODO: Compute camera view matrix and combine with projection matrix for passing to shader
	glm::mat4 view = glm::lookAt(camera.GetPosition(), camera.GetLookVector()+camera.GetPosition(), camera.GetUpVector());
	//Look at our viewmatrix code - look for glm::lookat!

	glm::mat4 combined = projection * view;

	// TODO: Send the combined matrix to the shader in a uniform
	glUseProgram(m_program);
	GLuint combined_loc = glGetUniformLocation(m_program, "combined");
	glUniformMatrix4fv(combined_loc, 1, GL_FALSE, glm::value_ptr(combined));

	// TODO: render each mesh. Send the correct model matrix to the shader in a uniform

	GLuint model_loc = glGetUniformLocation(m_program, "model"); // Assuming you have "model_matrix" in your shader


	//Make sure uniforms match what we are sending to the shaders!

	// You'll need to calculate the model matrix for each mesh here (e.g., using glm::translate, glm::rotate, glm::scale)
	glm::mat4 model = glm::mat4(1.0f); // Identity matrix for now
	glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, m_numElements, GL_UNSIGNED_INT, 0);

	// Always a good idea, when debugging at least, to check for GL errors each frame
	// Helpers::CheckForGLError();*/

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, m_wireframe ? GL_LINE : GL_FILL);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(camera.GetPosition(), camera.GetLookVector() + camera.GetPosition(), camera.GetUpVector());
	glm::mat4 combined = projection * view;

	glUseProgram(m_program);
	glUniformMatrix4fv(glGetUniformLocation(m_program, "combined"), 1, GL_FALSE, glm::value_ptr(combined));

	glm::mat4 model = glm::mat4(1.0f); // Default model for now
	glUniformMatrix4fv(glGetUniformLocation(m_program, "model"), 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_numElements, GL_UNSIGNED_INT, 0);
}