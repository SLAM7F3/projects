/******************************************************************************\
* uniform.frag                                                                 *
* A simple fragment shader that receives a 'float time' uniform from the OSG   *
* program. It also shows how to use a 'varying' variable that is set by the    *
* vertex shader (but this is not OSG-related).                                 *
* Leandro Motta Barros                                                         *
\******************************************************************************/

// This variable is set by the OSG program. Notice that OSG automatically
// sets a 'osg_FrameTime' uniform variable that does exactly the same thing as
// the 'time' variable manually set in this example.
uniform float time;

// This comes from the vertex shader. The vertex shader sets the 'rgb' value for
// each vertex. Then, during rasterization, the 'rgb' values are automatically
// interpolated and the (interpolated) value is made available
// to the fragment shader as a 'rgb' varying variable. (This is just the normal
// behavior of varying variables; it does not have anything to do with OSG.)
varying vec3 rgb;


void main()
{
   // This vertex shader does a very simple color animation. The fragment color
   // is the mixing of two colors: 'rgb' (which comes from the vertex shader)
   // and gray (defined below). And the contribution of each color to the final
   // color varies with 'time'.
   const vec3 gray50 = vec3(0.5, 0.5, 0.5);
   gl_FragColor = vec4 (mix(rgb, gray50, abs(sin(time*1.234))), 1.0);
}
