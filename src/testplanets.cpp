#include <osg/Node>
#include <osgViewer/Viewer>

osg::Node* createEarth();

void main()
{
	osgViewer::Viewer viewer;

	viewer.setSceneData( createEarth() );

	viewer.run();
}