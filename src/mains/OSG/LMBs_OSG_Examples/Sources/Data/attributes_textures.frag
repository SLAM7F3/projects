/******************************************************************************\
* attributes_textures.frag                                                     *
* A simple fragment shader that sets the fragment color as a mixing of the     *
* color of two textures. The two textures come from the OSG program as         *
* 'uniform sampler2D' variables, and the weight for each texture in the mixing *
* comes from the vertex shader as a 'varying' variable.                        *
* Leandro Motta Barros                                                         *
\******************************************************************************/

// This comes from the vertex shader. If you look there, you'll see that this
// value actually comes originally from the OSG program, as an 'attribute'
// variable (but the fragment shader is ignorant about this fact).
// The 'texRatio' defines the weight of each texture when they are blended.
varying float texRatio;

// These two 'sampler2D's come from the OSG program. They provide access to the
// texture data of the two textures involved.
uniform sampler2D firstTex;
uniform sampler2D secondTex;

void main()
{
   // First, we get the color of the two textures defined in the OSG program.
   vec4 firstColor = texture2D (firstTex, gl_TexCoord[0].st);
   vec4 secondColor = texture2D (secondTex, gl_TexCoord[1].st);

   // Then, we obtain the final fragment color by mixing them. The 'texRatio'
   // variable is used to determine the "weight" of each texture.
   gl_FragColor = mix (firstColor, secondColor, texRatio);
}
