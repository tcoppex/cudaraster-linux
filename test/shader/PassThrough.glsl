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

// UNIFORM
uniform vec3 uColor = vec3( 1.0f, 0.0f, 0.0f);

void main()
{  
  fragColor = vec4( uColor, 1.0f);
}

