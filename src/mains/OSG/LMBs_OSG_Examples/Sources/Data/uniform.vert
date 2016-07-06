/******************************************************************************\
* uniform.vert                                                                 *
* A simple vertex shader that receives a 'float time' uniform from the OSG     *
* program. It also shows how to set a 'varying' variable that can be used by   *
* the fragment shader (but this is not OSG-related).                           *
* Leandro Motta Barros                                                         *
\******************************************************************************/

// This variable is set by the OSG program. Notice that OSG automatically
// sets a 'osg_FrameTime' uniform variable that does exactly the same thing as
// the 'time' variable manually set in this example.
uniform float time;

// This is set by the shader for every vertex. Then, during rasterization, it
// is automatically interpolated and the (interpolated) value is made available
// to the fragment shader. (This is just the normal behavior of varying
// variables; it does not have anything to do with OSG.)
varying vec3 rgb;


void main()
{
   // We set the 'rgb' varying to be the same as the absolute value of the
   // vertex position. In the fragment shader, this will be used to set the
   // fragment color. Since in this example the geometry is a sphere centered in
   // the origin with radius equal to 1.0, we never get any component above
   // 1.0. 'abs()' is used to avoid negative components in color values (I don't
   // know what the OpenGL specification says about colors with negative
   // components).
   rgb = abs (vec3 (gl_Vertex.x, gl_Vertex.y, gl_Vertex.z));

   // This vertex shader does a very simple animation: it just scales the
   // rendered object, by multiplying the incoming vertices by a value between
   // 0.0 and 1.0. This value varies with time following a sinusoidal function.
   // The uniform 'time' variable, passed from OSG, contains the time, in
   // seconds, elapsed since the application started.
   vec4 v = gl_Vertex;
   v.xyz *= (1.0 + sin(time)) * 0.5; // Update x, y and z. Don't touch 'w'.

   gl_Position = gl_ModelViewProjectionMatrix * v;
}
