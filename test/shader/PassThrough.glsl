/*
 *          PassThrough.glsl
 *
 */

//------------------------------------------------------------------------------


-- Vertex


// IN
layout(location = 0) in vec4 inPosition;


// UNIFORM
uniform mat4 uModelViewProjMatrix;


void main()
{
  gl_Position = uModelViewProjMatrix * inPosition;
}


--

//------------------------------------------------------------------------------


-- Fragment

// OUT
layout(location = 0) out vec4 fragColor;


void main()
{
  vec3 red_color = vec3( 1.0f, 0.0f, 0.0f);
  fragColor = vec4( red_color, 1.0f);
}

