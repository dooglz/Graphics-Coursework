#include "Particles.h"
#include "main.h"
#include "libENUgraphics\graphics_framework.h"

using namespace std;
using namespace std::chrono;
using namespace glm;
using namespace graphics_framework;

// Maximum number of particles
static const unsigned int MAX_PARTICLES = 100;

// A particle
struct particle {
  vec3 position;
  vec3 velocity;
  vec2 lifetime;
};

// Particles in the system
static particle particles[MAX_PARTICLES];

// The transform feedback buffers
static GLuint transform_feedbacks[2];
// The particle buffers
static GLuint particle_buffers[2];

// Effects
static effect particle_eff;

// Current buffer to perform the physics update to
static unsigned int front_buf = 0;
// Buffer to render to
static unsigned int back_buf = 1;

void InitParticles() {
  default_random_engine rand(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
  uniform_real_distribution<float> dist;

  // Generate transform feedback buffers on GPU
  glGenTransformFeedbacks(2, transform_feedbacks);
  // Generate particle buffers on GPU
  glGenBuffers(2, particle_buffers);

  // *******************
  // Initilise particles
  for (unsigned int i = 0; i < MAX_PARTICLES; ++i) {
    float fi = (((float)i) / ((float)MAX_PARTICLES)) * 2.0f* pi<float>();
    particles[i].position = vec3(0,3.0,0);
    particles[i].velocity = vec3(2.0f*sinf(fi), 2.0f, 2.0f*cosf(fi));
    float life = (dist(rand)* 10.0f) + 2.0f;
    particles[i].lifetime = vec2(0.0f, life);
  } 
  // ************************
  // Allocate the two buffers
  for (unsigned int i = 0; i < 2; i++) {
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transform_feedbacks[i]);
    glBindBuffer(GL_ARRAY_BUFFER, particle_buffers[i]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particles), particles, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particle_buffers[i]);
  }


  particle_eff.add_shader("shaders\\particle.vert", GL_VERTEX_SHADER);
  particle_eff.add_shader("shaders\\particle.geom", GL_GEOMETRY_SHADER);
  particle_eff.add_shader("shaders\\null.frag", GL_FRAGMENT_SHADER);
  particle_eff.build();

  // Use the particle effect
  renderer::bind(particle_eff);

  // Define names of output data to capture from geometry shader
  const GLchar *attrib_names[3] = {"position_out", "velocity_out","lifetime_out"};
  // Set these on the feedback
  glTransformFeedbackVaryings(particle_eff.get_program(), 3, attrib_names, GL_INTERLEAVED_ATTRIBS);

  // Have to relink the program after this
  glLinkProgram(particle_eff.get_program());
  // Check if error
  if (CHECK_GL_ERROR) {
    // Display error
    std::cerr << "Problem linking program" << std::endl;
  }
}

void UpdateParticles(const float delta_time) {

  static bool first_frame = true;

  // Bind particle effect
  renderer::bind(particle_eff);

  // Set the delta_time uniform
  glUniform1f(particle_eff.get_uniform_location("delta_time"), delta_time);

  // Tell OpenGL we aren't rendering
  glEnable(GL_RASTERIZER_DISCARD);

  // *******************************
  // Bind the buffers for use
  // - buffer is front buf
  // - transform feeback is back buf
  // *******************************
  glBindBuffer(GL_ARRAY_BUFFER, particle_buffers[front_buf]);
  glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transform_feedbacks[back_buf]);

  // Define how our data looks like to the shader
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(particle), 0);                 // position
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(particle), (const GLvoid*)12); // velocity
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(particle), (const GLvoid*)24); // lifetime

  // ******************************
  // Perform the transform feedback
  // ******************************
  glBeginTransformFeedback(GL_POINTS);
  if (first_frame)
  {
    //do a straight copy
    glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);
    first_frame = false;
  }
   // Otherwise perform the draw
  else{
    glDrawTransformFeedback(GL_POINTS, transform_feedbacks[front_buf]);
  }

  // End the transform feedback
  glEndTransformFeedback();

  // Disable the vertex attribute arrays
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);

  glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  // Switch on rendering again
  glDisable(GL_RASTERIZER_DISCARD);

  if (CHECK_GL_ERROR) {
    std::cerr << "Particle Update program" << std::endl;
  }
}
void RenderParticles() {
  // Bind the effect
  renderer::bind(gfx->simpleEffect);
  // Set the MVP matrix
  auto M = mat4(1.0f);
  auto V = gfx->activeCam->get_view();
  auto P = gfx->activeCam->get_projection();
  auto MVP = P * V * M;
  glUniformMatrix4fv(gfx->simpleEffect.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
  // Set the colour uniform
  glUniform4fv(gfx->simpleEffect.get_uniform_location("colour"), 1, value_ptr(vec4(1.0f, 0.0f, 0.0f, 1.0f)));

  // Bind the back particle buffer for rendering
  glBindBuffer(GL_ARRAY_BUFFER, particle_buffers[back_buf]);

  // Describe the data we are interested in (just position)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(particle), 0);  // position

  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  //glPointSize(10.0f);

  // Perform the render by drawing the transform feedback
  glDrawTransformFeedback(GL_POINTS, transform_feedbacks[back_buf]);

  // Disable vertex attribute array
  glDisableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  // Swap front and back buffers
  front_buf = back_buf;
  back_buf = (back_buf + 1) & 0x1;
}
void ShutdownParticles() {}