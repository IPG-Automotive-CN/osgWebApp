#ifndef VBOSETUPVISITOR_HPP_
#define VBOSETUPVISITOR_HPP_
#include <osg/NodeVisitor>
#include <osg/Geode>

// BEGIN FEATURE VBO
// This class forces the use of VBO.
class VBOSetupVisitor : public osg::NodeVisitor
{
    public:
        VBOSetupVisitor() :
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) { }
        virtual void apply(osg::Geode &geode)
        {
            for (unsigned int i = 0; i < geode.getNumDrawables(); ++i)
            {
                osg::Geometry *geom =
                    dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
                if (geom)
                {
                    geom->setUseVertexBufferObjects(true);
                }
            }
            NodeVisitor::apply(geode);
        }
};
#endif