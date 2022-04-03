#include "world.h"

void Group::addModel(Model model) {
    Group::models.push_back(model);
}

vector<Model> Group::getModels() {
    return Group::models;
}

void Group::addTranslate(Translate translate) {
    Group::translates.push_back(translate);
}

vector<Translate> Group::getTranslate() {
    return Group::translates;
}

void Group::addRotate(Rotate rotate) {
    Group::rotates.push_back(rotate);
}

vector<Rotate> Group::getRotate() {
    return Group::rotates;
}

void Group::addScale(Scale scale) {
    Group::scales.push_back(scale);
}

vector<Scale> Group::getScale() {
    return Group::scales;
}

Color Group::getColor() {
    return Group::color;
}

void Group::setColor(Color c) {
    Group::color = c;
}

void Group::addGroup(Group group) {
    Group::subGroups.push_back(group);
}

vector<Group> Group::getGroups() {
    return Group::subGroups;
}

void Group::translate(float x, float y, float z) {
    for(Model model : Group::models)
        for(Patch patch : model.getPatches())
            for(Point point : patch.getPoints()) {
                point.setX(point.getX() + x);
                point.setY(point.getY() + y);
                point.setZ(point.getZ() + z);
            }
            
    for(Group group : Group::subGroups)
        group.translate(x, y, z);
}

void Group::rotate(float angle, float x, float y, float z) {
    for(Model model : Group::models)
        for(Patch patch : model.getPatches())
            for(Point point : patch.getPoints()) {
                if(x == 1){
                    float yy = point.getY()*cosf(angle) - point.getZ()*sinf(angle);
                    float zz = point.getY()*sinf(angle) + point.getZ()*cosf(angle);
                    point.setX(point.getX());
                    point.setY(yy);
                    point.setZ(zz);
                }
                if(y == 1){
                    float xx = point.getX()*cosf(angle) + point.getZ()*sinf(angle);
                    float zz = - point.getX()*sinf(angle) + point.getZ()*cosf(angle);
                    point.setX(xx);
                    point.setY(point.getY());
                    point.setZ(zz);
                }
                if(z == 1){
                    float xx = point.getX()*cosf(angle) - point.getY()*sinf(angle);
                    float yy = point.getX()*sinf(angle) + point.getY()*cosf(angle);
                    point.setX(xx);
                    point.setY(yy);
                    point.setZ(point.getZ());
                }
            }
        
    for(Group group : Group::subGroups)
        group.rotate(angle, x, y, z);
}

void Group::scale(float x, float y, float z) {
    for(Model model : Group::models)
        for(Patch patch : model.getPatches())
            for(Point point : patch.getPoints()) {
                point.setX(point.getX() * x);
                point.setY(point.getY() * y);
                point.setZ(point.getZ() * z);
            }
            
    for(Group group : Group::subGroups)
        group.scale(x, y, z);
}

Camera World::getCamera() {
    return World::camera;
}

Group* World::getGroup() {
    return World::group;
}

void World::addModel(Model model) {
    World::group->addModel(model);
}

int World::parseXML(string path, string filename) {
    string file = path + filename;
    cout << "Loading " << file << "." << endl;
    XMLDocument doc;
    if(!doc.LoadFile(file.c_str())){
        XMLElement * cameraElem = doc.FirstChildElement("world")->FirstChildElement("camera");
        XMLElement * groupElem = doc.FirstChildElement("world")->FirstChildElement("group");
        
        World::parseCamera(cameraElem);
        Group *g = (new Group());
        World::parseGroup(path, groupElem, g);
        World::group = g;

        cout << "Finished loading " << file << "." << endl;
        return 1;
    }
    cout << "Failed to load " << file << "." << endl;
    return 0;
}

void World::parseCamera(XMLElement * elem) {
    float posX, posY, posZ;
    XMLElement * positionElem = elem->FirstChildElement("position");
    positionElem->QueryFloatAttribute("x", &posX);
    positionElem->QueryFloatAttribute("y", &posY);
    positionElem->QueryFloatAttribute("z", &posZ);
    World::camera.setPosition(new Point(posX, posY, posZ));

    float lookAtX, lookAtY, lookAtZ;
    XMLElement * lookAtElem = elem->FirstChildElement("lookAt");
    lookAtElem->QueryFloatAttribute("x", &lookAtX);
    lookAtElem->QueryFloatAttribute("y", &lookAtY);
    lookAtElem->QueryFloatAttribute("z", &lookAtZ);
    World::camera.setLookAt(new Point(lookAtX, lookAtY, lookAtZ));

    float upX, upY, upZ;
    XMLElement * upElem = elem->FirstChildElement("up");
    upElem->QueryFloatAttribute("x", &upX);
    upElem->QueryFloatAttribute("y", &upY);
    upElem->QueryFloatAttribute("z", &upZ);
    World::camera.setUp(new Point(upX, upY, upZ));

    float fov, near, far;
    XMLElement * projectionElem = elem->FirstChildElement("projection");
    projectionElem->QueryFloatAttribute("fov", &fov);
    projectionElem->QueryFloatAttribute("near", &near);
    projectionElem->QueryFloatAttribute("far", &far);
    World::camera.setFov(fov);
    World::camera.setNear(near);
    World::camera.setFar(far);
}

void World::parseGroup(string path, XMLElement * elem, Group *g) {
    // Load models
    for(XMLElement * child = elem->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
        printf("%s\n", child->Value());
        if(strcmp(child->Value(),"models") == 0){
            parseModels(path, child, g);
        }
    }

    // Load sub-groups
    for(XMLElement * child = elem->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
       //printf("%s\n", child->Value());
        if(strcmp(child->Value(),"group") == 0) {
            Group *subGroup = (new Group());
            parseGroup(path, child, subGroup);
            g->addGroup(*subGroup);
        }
    }

    // Load Transforms
    for(XMLElement * child = elem->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
        if(strcmp(child->Value(),"transform") == 0)
            World::parseTransform(child, g);
    }
}

void World::parseModels(string path, XMLElement * elem, Group * g) {
    for(XMLElement * child = elem->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
        if(strcmp(child->Value(),"model") == 0) {
            string filename = child->Attribute("file");
            Model model;
            cout << path << filename << " model loaded." << endl;
            model.readModel(path + filename);
            g->addModel(model);
        }
    }
}

void World::parseTransform(XMLElement * elem, Group *g) {
    for(XMLElement * child = elem->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
        if(strcmp(child->Value(), "rotate") == 0) {
            float angle, x, y, z;
            child->QueryFloatAttribute("angle", &angle);
            child->QueryFloatAttribute("x", &x);
            child->QueryFloatAttribute("y", &y);
            child->QueryFloatAttribute("z", &z);
            g->addRotate(*(new Rotate(angle, x, y, z)));
        } else if(strcmp(child->Value(), "translate") == 0) {
            float x, y, z;
            child->QueryFloatAttribute("x", &x);
            child->QueryFloatAttribute("y", &y);
            child->QueryFloatAttribute("z", &z);
            g->addTranslate(*(new Translate(x, y, z)));
        } else if(strcmp(child->Value(), "scale") == 0) {
            float x, y, z;
            child->QueryFloatAttribute("x", &x);
            child->QueryFloatAttribute("y", &y);
            child->QueryFloatAttribute("z", &z);
            g->addScale(*(new Scale(x, y, z)));
        } else if(strcmp(child->Value(), "color") == 0) {
            float r, gg, b;
            child->QueryFloatAttribute("r", &r);
            child->QueryFloatAttribute("g", &gg);
            child->QueryFloatAttribute("b", &b);
            g->setColor(*(new Color(r, gg, b)));
        }
    }
}

void Model::drawModel(Color color) {
    for(Patch patch : getPatches()) {
        vector<Point> primitives = patch.getPoints();
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < primitives.size(); i++) {
            // glColor3f(static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
            //           static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
            //           static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
            
            Point point = primitives[i];
            glColor3f(color.getR(), color.getG(), color.getB());
            glVertex3f(point.getX(), point.getY(), point.getZ());
        }
        glEnd();
    }
}
