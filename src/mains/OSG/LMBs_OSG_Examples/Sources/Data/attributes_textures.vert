/******************************************************************************\
* attributes_textures.vert                                                     *
* A simple vertex shader that receives an 'attribute float' variable from the  *
* OSG program. It also shows how to get the texture coordinates that come from *
* the OSG program.                                                             *
* Leandro Motta Barros                                                         *
\******************************************************************************/

// This comes from the OSG program.
attribute float vertexTexRatio;

// This will be set by this vertex shader; will be equal to the 'vertexTexRatio'
// that comes from the OSG program. As is the case with all 'varying' variables,
// its values will be automatically interpolated during rasterization and made
// available to the fragment shader.
varying float texRatio;

void main()
{
   // The vertex position is no different that the one that would be calculated
   // by fixed functionality.
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

   // The 'attribute' variable coming from OSG will be effectively used by the
   // fragment shader. The way to somehow access it from there is passing it via
   // a 'varying' variable.
   texRatio = vertexTexRatio;

   // Pass the texture coordinates coming from the OSG program to the fragment
   // shader; conceptually, this is the same thing done with the "custom"
   // 'attribute', in the previous line.
   gl_TexCoord[0] = gl_MultiTexCoord0;
   gl_TexCoord[1] = gl_MultiTexCoord1;
}
