/*
 *          ScreenMapping.glsl
 *
 */

//------------------------------------------------------------------------------


-- Vertex


out vec2 vTexCoord;


/*  
  This is an OpenGL port of a trick found at #AltDevBlogADay to create a 
  triangle where the texture is mapped to the window coords as a quad. 
  It needs 3 random vertices to be sent to the graphic card, the real vertices
  position and texCoord are generated afterward.
  
  ref : http://altdevblogaday.com/2011/08/08/interesting-vertex-shader-trick/
*/
void main(void)
{
  vTexCoord.s = (gl_VertexID << 1) & 2;
  vTexCoord.t = gl_VertexID & 2;
  
  gl_Position = vec4( 2.0f*vTexCoord - 1.0f, 0.0f, 1.0f);
}


--

//------------------------------------------------------------------------------


-- Fragment

// IN
in vec2 vTexCoord;

// OUT
layout(location = 0) out vec4 fragColor;

// UNIFORM
uniform sampler2D uTexture;


void main(void)
{
  fragColor = texture( uTexture, vTexCoord);
}

