/****************************************************************************
Copyright (c) 2013-2014 Auticiel SAS

http://www.fennex.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************///

#ifndef __FenneX__GraphicLayer__
#define __FenneX__GraphicLayer__

#include "cocos2d.h"
USING_NS_CC;
#include "Pausable.h"
#include "Scene.h"
#include "RawObject.h"
#include "CustomObject.h"
#include "Image.h"
#include "LabelTTF.h"
#include "Panel.h"
#include "InputLabel.h"
#include "DropDownList.h"
#include "CustomBaseNode.h"
#include "FenneXMacros.h"

NS_FENNEX_BEGIN

class GraphicLayer : public Ref, public Pausable
{
public:
    static GraphicLayer* sharedLayer();
    
    /**********************************************************************************
     Methods to create objects
     *********************************************************************************/
    
    /*
     Optional values for all objects :
     - X (float) default 0
     - Y (float) default 0
     - Scale (float) default 1
     - Zindex (int) default 0
     - Name (string) default imageFile or spriteSheetFile
     - EventName (String) default empty
     - Panel (Panel ID) default none
     - EventInfos (ValueMao) default nothing
     - Visible (bool) default visible
     - Opacity (int) default 255 (should range between 0-255)
     */
    
    Image* createImage(std::string imageFile, ValueMap values);
    Image* createAnimatedImage(std::string spriteSheetFile, int capacity, ValueMap values);
    Image* createImageFromSprite(Sprite* sprite, Panel* parent);
    
    CustomObject* createCustomObject(Node* delegate, ValueMap values);
    CustomObject* createCustomObjectFromNode(Node* node, Panel* parent);
    
    /* FontFile must be formatted as FontnameSizeColor (example : Verdana30Black, recognized colors are : Black, White, Gray)
     Additional optional values :
     - Dimensions: both DimX (float) and DimY (float) default 0, 0, which translates to NO_MAX_LINE_WIDTH
     - TextFormat (int as enum TextFormat), requires DimX and DimY, default  AlignCenter
     Do not use scale if you use dimensions, or it won't work properly
     */
    LabelTTF* createLabelTTF(std::string label, std::string fontFile, ValueMap values);
    LabelTTF* createLabelTTFromLabel(Label* cocosLabel, Panel* parent);
    
    /* There is no createInputLabel with values, because it is not yet used. Feel free to create it if you need it 
     */
    InputLabel* createInputLabelFromScale9Sprite(ui::Scale9Sprite* cocosSprite, Panel* parent);
    
    Panel* createPanel(std::string name, ValueMap values);
    Panel* createPanelFromNode(Node* cocosNode, Panel* parent);
    //Special case : when you want the panel to be the host for a .ccbi file, the Node is already created
    Panel* createPanelWithNode(std::string name, Node* panelNode, int zOrder = 0);
    
    
    /* There is no createDropDownList with values, because it is not yet used. Feel free to create it if you need it
     */
    DropDownList* createDropDownListFromSprite(Sprite* sprite, Panel* parent);
    
    //Duplicate an object, currently supported : Image (not animated), LabelTTF, Panel (including its children, by recursive call)
    //Apart from specific type infos, copied properties are : Position, Zindex, Panel, Name, EventName, EventInfos, Visible, Scale
    RawObject* duplicateObject(RawObject* otherObject);
    
    
    /**********************************************************************************
     Methods to interact with Panels
     *********************************************************************************/
    
    //Leave panel at NULL to remove it from any panel it may be on
    RawObject* placeObject(RawObject* obj, Panel* panel = NULL);
    
    //Remove will put objects from panel on the manager, use destroy if you want to destroy them
    void removeObjectFromPanel(RawObject* obj, Panel* panel);
    void removeAllObjectsFromPanel(Panel* panel);
    
    //Get the parent panel of an object, or NULL if the object has no parent
    Panel* getContainingPanel(RawObject* obj);
    
    /**********************************************************************************
     Methods to destroy objects
     *********************************************************************************/
    
    //Destroy an object of the manager
    //It is not possible to remove an object without destroying it, as only the manager should store RawObject references
    void destroyObject(RawObject* obj);
    //Destroy an array of objects.
    CC_DEPRECATED_ATTRIBUTE void destroyObjects(CCArray* array); //use next method instead
    void destroyObjects(Vector<RawObject*> array);
    
    //Convenience method for DelayedDispatcher::funcAfterDelay method. Will check the type before passing.
    void destroyObjectEvent(EventCustom* event);
    void destroyObjectsEvent(EventCustom* event);
    
    /**********************************************************************************
     Methods to retrieve objects
     *********************************************************************************/
    
    RawObject* getById(int id);
    
    CCArray* getChildren() { return storedObjects; }
    //Different ways of querying objects
    //The first object found is always returned to avoid managing arrays, or nil if no result
    //Cache is a way to tell GraphicLayer to cache this particular object for the current scene, so that subsequent call will be very fast
    RawObject* firstObjectWithName(CCString* name, bool cache = false);
    RawObject* firstObjectWithName(std::string name, bool cache = false);
    RawObject* firstObjectWithNameInPanel(std::string name, Panel* panel);
    RawObject* firstObjectAtPosition(Vec2 position);
    RawObject* objectAtIndex(int index);
    
    //The resulting array should be released after being used
    CCArray* allObjectsWithName(CCString* name);
    CCArray* allObjectsWithName(std::string name);
    CCArray* allObjectsWithNameInPanel(std::string name, Panel* panel);
    CCArray* allObjectsStartingWithString(CCString* name);
    CCArray* allObjectsStartingWithString(std::string name);
    CCArray* allObjectsAtPosition(Vec2 position);
    CCArray* allActionnableObjects();
    CCArray* allObjects(const std::function<bool(RawObject*)>& filter);
    
    CCArray* allVisibleObjectsAtPosition(Vec2 position);
    
    CCArray* allPanelsWithName(std::string name);
    CCArray* allPanelsWithName(CCString* name);
    Panel* firstPanelWithName(std::string name);
    Panel* firstPanelWithName(CCString* name);
    Panel* firstVisiblePanelWithName(CCString* name);
    
    
    /**********************************************************************************
     Methods to get position/scale relative to world instead of local
     *********************************************************************************/
    
    Vec2 getPositionRelativeToObject(Vec2 point, RawObject* obj);
    Vec2 getRealPosition(RawObject* obj);
    Vec2 getCenterRealPosition(RawObject* obj);
    float getRealScale(RawObject* obj);
    float getRealScaleX(RawObject* obj);
    float getRealScaleY(RawObject* obj);
    
    /**********************************************************************************
     Miscellaneous methods
     *********************************************************************************/
    
    //Return true if the position is inside the object
    bool collision(Vec2 position, RawObject* obj);
    
    //Return true if at least some part of the object is on screen
    bool isOnScreen(RawObject* obj, cocos2d::Size size = cocos2d::Size(0, 0));
    
    //Return true if obj1 is in front of obj2
    bool isInFront(RawObject* obj1, RawObject* obj2);
    
    //The universal clock
    float getClock() { return clock; }
    
    //Return true if the object is known by the layer. Does not take into account objects being added/removed
    bool containsObject(RawObject* obj);
    
    //Should be by superclass called to perform object touch action, return YES to claim touch
    bool touchObject(RawObject* obj, bool event, Vec2 position);
    
    //change child local z-order, and update storedObjects
    void reorderChild(RawObject* child, int zOrder);
    
    //On resume app, some platforms (Android so far) need to refresh render texture because the openGL context is dropped and the render textures are invalid
    void refreshRenderTextures(Ref* obj);
    
    /**********************************************************************************
     Methods that are used by FenneX other classes. You should not call any directly
     *********************************************************************************/
    //Return an available ID and increment the counter for next one
    int getNextId();
    
    //Used when loading an entire scene from CCB
    void useBaseLayer(Layer* otherLayer);
    
    //Specify on which scene it should be rendered
    void renderOnLayer(Scene* destination);
    
    //Should be called to detach it before being render on another layer
    void stopRenderOnLayer(Scene* destination, bool cleanup);
    
    //Call this method to transmit a touch to it's children
    //The touch will be transmitted to each child which have position in it's bounding box
    //Return true if at least one child claimed it
    //if event is true, it will post a notification according the object. Else, it will ask for informations about the object
    bool touchAtPosition(Vec2 position, bool event);
    
    //Run update on all children, then add/remove children. Also update clock
    virtual void update(float deltaTime);
    
    //Stop the layer and remove it from its parent
    virtual void stop();
    
private:
    //Initialization, use init method instead of a constructor.
    void init();
    
    //Remove all objects
    void clear();
    
    //Since GraphicLayer is a singleton, it should never be dealloc. It should be stopped instead
    ~GraphicLayer();
    
    //Actually add the object to the Layer
    void addObject(RawObject* obj, int z = 0);
    void setObjectFields(RawObject* obj, ValueMap values);
    
    //Used to internally reorder storedObjects after updating an object z-order
    void reorderChildrenOfPanel(Panel* panel);
    
    //Helper method to load ccb infos into an object
    void loadBaseNodeAttributes(CustomBaseNode* node, RawObject* obj);
    
    //Array containing all objects. They are sort according their zOrder (ascending)
    CCArray* storedObjects;
    //Array containing panels, for easier retrieval when using Panel specific methods
    CCArray* storedPanels;
    //Allow to easily find the parent of any object
    std::map<int, Panel*> childsParents;//keys are objects ID, values are Panel
    
    //Lock add/remove during updating, and instead use the following vectors, which will be used to add/remove at the end of updating
    bool isUpdating;
    Vector<RawObject*> objectsToAdd;
    std::vector<int> objectsToAddZindex;
    std::vector<Panel*> objectsToAddPanel;
    Vector<RawObject*> objectsToRemove; //only contains RawObjects directly
    
    //the actual rendering layer for graphic objects stored
    Layer* layer;
    
    //The parent of the layer, to remove the layer when stopping
    Scene* relatedScene;
    
    //The time is counted separately from update method, so that everything is on the same page
    float clock;
    //Next available ID that any new object can take. getNextId will automatically increment it
    int nextAvailableId;
};

NS_FENNEX_END

#endif /* defined(__FenneX__GraphicLayer__) */
