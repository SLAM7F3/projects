/******************************************************************************\
* NodeVisitor                                                                  *
* Using the 'NodeVisitor' subclass to find all scene graph 'Drawable's.        *
* Leandro Motta Barros                                                         *
\******************************************************************************/

#include <iostream>
#include <osg/NodeVisitor>
#include <osgDB/ReadFile>


// This is our 'NodeVisitor' subclass. A visitor will traverse the scene graph
// (or a sub-scene graph), executing some user-defined code when visiting each
// node. In this example, our goal is to find all 'Drawable's of a scene graph.
class MyVeryOwnVisitor: public osg::NodeVisitor
{
   public:
      // Notice that we pass 'TRAVERSE_ALL_CHILDREN' to the
      // 'NodeVisitor' constructor, because we want to traverse all
      // children. Other options, like 'TRAVERSE_NONE' and
      // 'TRAVERSE_ACTIVE_CHILDREN' are available, too (check the
      // 'enum osg::NodeVisitor::TraversalMode' for all the
      // possibilities.
      MyVeryOwnVisitor()
         : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
      { }

      // We are interested in 'Drawable's, but 'Drawable's are not
      // nodes and therefore won't be visited. But since all
      // 'Drawable's are attached to 'Geode's, we'll visit all
      // 'Geode's and take the 'Drawable's directly from them.  So,
      // what we do is to implement the overload of the 'apply' method
      // which takes a 'Geode' as parameter; this will be called for
      // every 'Geode' visited during the traversal.
      virtual void apply (osg::Geode& node)
      {
         std::cout << "Hey! It's a geode! And its name is '"
                   << node.getName() << "'\n";

         for (unsigned i = 0; i < node.getNumDrawables(); ++i)
         {
            osg::Drawable* drawable = node.getDrawable (i);
            osg::Geometry* geom = drawable->asGeometry();

            // Print a little message for each drawable. Also, try to
            // "convert" it to an 'osg::Geometry', and print the
            // obtained pointer.
            std::cout << "   A drawable! asGeometry() = " << geom << '\n';
         }
      }

      // Since we are only interested in 'Geode's, the overload above
      // would suffice. But let's also implement the version that
      // takes an 'osg::Node' as parameter, so that an important
      // detail can be explained.  This function will be called for
      // every node (actually, for every node that doesn't have a more
      // specific overload -- in this example, it will be called for
      // every node that is not a 'Geode'). Well, the point is that
      // 'Group's are 'Node's, so this function will be called for
      // 'Group's. So, we have to explicitly tell OSG to keep
      // traversing the scene graph by calling 'traverse
      // (node)'. (Otherwise, the traversal would not descend beyond
      // this point.)  If this explanation was good enough, it should
      // be clear by now that if we were overloading the versions of
      // 'apply()' that take an 'osg::Group' (or some of its
      // subclasses, like 'osg::Transform') as parameter, we should
      // also call 'traverse (node)'.  Finally, just to clarify: the
      // default implementations of the different 'apply()' overloads
      // already call 'traverse (node)', so you don't have to
      // re-implement them unless you need to perform something in
      // addition to this.
      virtual void apply (osg::Node& node)
      {
         std::cout << "It's just a boring, non-geode node.\n";
         traverse (node); // don't forget this!
      }
};



int main (int argc, char* argv[])
{
   // Check command-line parameters
   if (argc != 2)
   {
      std::cerr << "Usage: " << argv[0] << " <model file>\n";
      exit (1);
   }

   // Load the model
   osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(argv[1]);

   if (!loadedModel)
   {
      std::cerr << "Problem opening '" << argv[1] << "'\n";
      exit (1);
   }

   // Traverse the scene graph using our own 'NodeVisitor' subclass. Notice
   // that we must call 'osg::Node::accept()' to do the traversal, not
   // 'osg::NodeVisitor::traverse()'.
   MyVeryOwnVisitor mvonv;
   loadedModel->accept(mvonv);
}
