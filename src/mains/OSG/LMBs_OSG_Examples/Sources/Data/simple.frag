/******************************************************************************\
* simple.frag                                                                  *
* An absurdly simple fragment shader. It simply sets the fragment color to     *
* "pure blue". Not very exciting, but this is the first I wrote :-)            *
* Leandro Motta Barros                                                         *
\******************************************************************************/

void main()
{
/*   gl_FragColor = vec4 (0.0, 0.0, 1.0, 1.0);  */
   gl_FragColor = vec4 (1.0, 0.0, 1.0, 1.0);
}
