// ==========================================================================
// Program STRIPE uses the GTS library to convert the input surface
// into triangle strips and outputs a Geomview representation of the
// result.

// Usage:  stripe [OPTION] < file.gts > file.off

// ==========================================================================
// Last updated on 3/2/07
// ==========================================================================

#include <stdlib.h>
#include <locale.h>
#include "config.h"
#include <getopt.h>
#include "gts.h"

int main (int argc, char * argv[])
{

// Read in surface:

   GtsSurface* s = gts_surface_new (gts_surface_class (),
                                    gts_face_class (),
                                    gts_edge_class (),
                                    gts_vertex_class ());
   GtsFile* fp = gts_file_new (stdin);
   if (gts_surface_read (s, fp)) 
   {
      fputs ("stripe: file on standard input is not a valid GTS file\n", 
             stderr);
      fprintf (stderr, "stdin:%d:%d: %s\n", fp->line, fp->pos, fp->error);
      return 1; /* failure */
   }
   gts_file_destroy (fp);

   gts_surface_print_stats (s, stderr);
   GSList* strips = gts_surface_strip (s);

   GtsRange l;
   gts_range_init (&l);
   GSList* i = strips;
   while (i) 
   {
      gts_range_add_value (&l, g_slist_length (
         static_cast<GSList*>(i->data)));
      i = i->next;
   }
   gts_range_update (&l);
   fprintf (stderr, "# Strips: %d\n#   length : ", l.n);
   gts_range_print (&l, stderr);
   fputc ('\n', stderr);
   
   puts ("LIST {\n");
   i = strips;
   while (i) 
   {
      GList* j = static_cast<GList*>(i->data);
      GtsTriangle * oldt = NULL;
      GtsColor c;

      c.r = rand ()/(gdouble) RAND_MAX;
      c.g = rand ()/(gdouble) RAND_MAX;
      c.b = rand ()/(gdouble) RAND_MAX;
      while (j) {
         GtsTriangle* t = static_cast<GtsTriangle*>(j->data);
         GtsPoint
            * p1 = GTS_POINT (GTS_SEGMENT (t->e1)->v1),
            * p2 = GTS_POINT (GTS_SEGMENT (t->e1)->v2),
            * p3 = GTS_POINT (gts_triangle_vertex (t));

         printf ("OFF 3 1 3\n%g %g %g\n%g %g %g\n%g %g %g\n3 0 1 2 %g %g %g\n",
                 p1->x, p1->y, p1->z,
                 p2->x, p2->y, p2->z,
                 p3->x, p3->y, p3->z,
                 c.r, c.g, c.b);
         if (oldt) {
            GtsSegment * cs = GTS_SEGMENT (gts_triangles_common_edge (t, oldt));
            GtsPoint
               * op1 = GTS_POINT (GTS_SEGMENT (oldt->e1)->v1),
               * op2 = GTS_POINT (GTS_SEGMENT (oldt->e1)->v2),
               * op3 = GTS_POINT (gts_triangle_vertex (oldt));
            
            printf ("VECT 1 3 0 3 0 %g %g %g %g %g %g %g %g %g\n",
                    (op1->x + op2->x + op3->x)/3.,
                    (op1->y + op2->y + op3->y)/3.,
                    (op1->z + op2->z + op3->z)/3.,
                    (GTS_POINT (cs->v1)->x + GTS_POINT (cs->v2)->x)/2.,
                    (GTS_POINT (cs->v1)->y + GTS_POINT (cs->v2)->y)/2.,
                    (GTS_POINT (cs->v1)->z + GTS_POINT (cs->v2)->z)/2.,
                    (p1->x + p2->x + p3->x)/3.,
                    (p1->y + p2->y + p3->y)/3.,
                    (p1->z + p2->z + p3->z)/3.);
         }
         oldt = t;
         j = j->next;
      }
      i = i->next;
   }
   puts ("}\n");

   return 0; /* success */
}

