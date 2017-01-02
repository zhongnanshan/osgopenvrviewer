
#include <osg/Node>
#include <osg/Geode>
#include <osg/Vec4>
#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/TexEnvCombine>
#include <osg/MatrixTransform>
#include <osg/Image>
#include <osgDB/ReadFile>

osg::Geode* createPlanet(double radius, const std::string& name, const osg::Vec4& color, const std::string& textureName)
{
	// create a container that makes the sphere drawable
	osg::Geometry *sPlanetSphere = new osg::Geometry();

	{
		// set the single colour so bind overall
		osg::Vec4Array* colours = new osg::Vec4Array(1);
		(*colours)[0] = color;
		sPlanetSphere->setColorArray(colours, osg::Array::BIND_OVERALL);


		// now set up the coords, normals and texcoords for geometry
		unsigned int numX = 100;
		unsigned int numY = 50;
		unsigned int numVertices = numX*numY;

		osg::Vec3Array* coords = new osg::Vec3Array(numVertices);
		sPlanetSphere->setVertexArray(coords);

		osg::Vec3Array* normals = new osg::Vec3Array(numVertices);
		sPlanetSphere->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);

		osg::Vec2Array* texcoords = new osg::Vec2Array(numVertices);
		sPlanetSphere->setTexCoordArray(0, texcoords);
		sPlanetSphere->setTexCoordArray(1, texcoords);

		double delta_elevation = osg::PI / (double)(numY - 1);
		double delta_azim = 2.0*osg::PI / (double)(numX - 1);
		float delta_tx = 1.0 / (float)(numX - 1);
		float delta_ty = 1.0 / (float)(numY - 1);

		double elevation = -osg::PI*0.5;
		float ty = 0.0;
		unsigned int vert = 0;
		unsigned j;
		for (j = 0;
			j<numY;
			++j, elevation += delta_elevation, ty += delta_ty)
		{
			double azim = 0.0;
			float tx = 0.0;
			for (unsigned int i = 0;
				i<numX;
				++i, ++vert, azim += delta_azim, tx += delta_tx)
			{
				osg::Vec3 direction(cos(azim)*cos(elevation), sin(azim)*cos(elevation), sin(elevation));
				(*coords)[vert].set(direction*radius);
				(*normals)[vert].set(direction);
				(*texcoords)[vert].set(tx, ty);
			}
		}

		for (j = 0;
			j<numY - 1;
			++j)
		{
			unsigned int curr_row = j*numX;
			unsigned int next_row = curr_row + numX;
			osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(GL_QUAD_STRIP);
			for (unsigned int i = 0;
				i<numX;
				++i)
			{
				elements->push_back(next_row + i);
				elements->push_back(curr_row + i);
			}
			sPlanetSphere->addPrimitiveSet(elements);
		}
	}


	// set the object color
	//sPlanetSphere->setColor( color );

	// create a geode object to as a container for our drawable sphere object
	osg::Geode* geodePlanet = new osg::Geode();
	geodePlanet->setName(name);

	if (!textureName.empty())
	{
		osg::Image* image = osgDB::readImageFile(textureName);
		if (image)
		{
			osg::Texture2D* tex2d = new osg::Texture2D(image);
			tex2d->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
			tex2d->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
			geodePlanet->getOrCreateStateSet()->setTextureAttributeAndModes(0, tex2d, osg::StateAttribute::ON);

			// reset the object color to white to allow the texture to set the colour.
			//sPlanetSphere->setColor( osg::Vec4(1.0f,1.0f,1.0f,1.0f) );
		}
	}

	// add our drawable sphere to the geode container
	geodePlanet->addDrawable(sPlanetSphere);

	return(geodePlanet);

}// end SolarSystem::createPlanet

osg::Geode* createPlanet(double radius, const std::string& name, const osg::Vec4& color, const std::string& textureName1, const std::string& textureName2)
{
	osg::Geode* geodePlanet = createPlanet(radius, name, color, textureName1);

	if (!textureName2.empty())
	{
		osg::Image* image = osgDB::readImageFile(textureName2);
		if (image)
		{
			osg::StateSet* stateset = geodePlanet->getOrCreateStateSet();

			osg::TexEnvCombine* texenv = new osg::TexEnvCombine;

			texenv->setCombine_RGB(osg::TexEnvCombine::INTERPOLATE);
			texenv->setSource0_RGB(osg::TexEnvCombine::PREVIOUS);
			texenv->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
			texenv->setSource1_RGB(osg::TexEnvCombine::TEXTURE);
			texenv->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);
			texenv->setSource2_RGB(osg::TexEnvCombine::PRIMARY_COLOR);
			texenv->setOperand2_RGB(osg::TexEnvCombine::SRC_COLOR);

			stateset->setTextureAttribute(1, texenv);
			osg::Texture2D* tex2d = new osg::Texture2D(image);
			tex2d->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
			tex2d->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
			stateset->setTextureAttributeAndModes(1, tex2d, osg::StateAttribute::ON);
		}
	}

	return(geodePlanet);

}// end SolarSystem::createPlanet

class RotateCallback : public osg::NodeCallback {

public:

	RotateCallback() : osg::NodeCallback(), enabled_(true), velocityScale(0.1) {}

	void operator()(osg::Node* node, osg::NodeVisitor *nv)
	{
		osg::MatrixTransform *xform = dynamic_cast<osg::MatrixTransform *>(node);
		if (xform && enabled_) {
			double t = nv->getFrameStamp()->getSimulationTime();
			xform->setMatrix(osg::Matrix::rotate(velocityScale * t, osg::Vec3(0, 0, 1)));
		}
		traverse(node, nv);
	}

	bool enabled_;
	double velocityScale;
};

osg::Node* createEarth()
{
	osg::Group* root = new osg::Group;

	osg::ClearNode* clearNode = new osg::ClearNode;
	clearNode->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	root->addChild(clearNode);

	std::string mapEarth = "Images/land_shallow_topo_2048.jpg";
	std::string mapEarthNight = "Images/land_ocean_ice_lights_2048.jpg";

	osg::Node* earth = createPlanet(100.0f, "Earth", osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f), mapEarth, mapEarthNight);

	osg::MatrixTransform* earthRotate = new osg::MatrixTransform;
	earthRotate->addUpdateCallback(new RotateCallback());
	earthRotate->addChild(earth);

	return earthRotate;
}
