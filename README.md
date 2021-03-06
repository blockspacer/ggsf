<h1>GGSF</h1>
<h2>Intro</h2>
<p>GGSF is an <a href="https://en.wikipedia.org/wiki/Entity_component_system">ECS</a>-based framework for (currently 2D) game
development (using <a href="https://github.com/skypjack/entt">EnTT</a>). It is also the file extension of the accompanying scene file that uses <a href="https://en.wikipedia.org/wiki/JSON">JSON</a>.
</p>

<h2>Scenes</h2>
<p>A scene is deployed as a Zip archive. There is only one Zip per scene and all the assets for the scene (currently) must be in the archive.
The Zip must contain at least "scene.ggsf" which is one big JSON object that contains three other objects "settings", "entities"
and "templates". Settings must be an object containing name/value pairs. Entities and templates must be an array of objects.</p>

<h3>Settings</h3>
<p>The settings object contains basic information about the scene including which 
Systems (the 'S' in ECS) to initialize for the scene, and the order to call
specific SystemInterface functions. Available settings:</p>
<table border="1"><tr><th>Name</th><th>Function</th></tr>
<tr><td>clear-color</td><td>The color with which to fill the screen each frame before any other rendering. array of 4 floats.</td></tr>
	<tr><td>script-file</td><td>The name of the script file to use for this scene</td></tr>
<tr><td>systems</td><td>A list of Systems that will be used in this scene</td></tr>
<tr><td>begin-calls</td><td>A list of Systems that need their begin function called and in the order given</td></tr>
<tr><td>activate-calls</td><td>A list of Systems that need their activate function called, in the order given</td></tr>
<tr><td>update-calls</td><td>A list of Systems that need their update function called and in the order given</td></tr>
<tr><td>render-calls</td><td>A list of Systems that will be rendering, and called in the order given</td></tr>
<tr><td>system-settings</td><td>A list of objects that contain name/object pairs, where the object 
	contains name/value pairs similar to these settings, but for the listed system</td></tr>
</table>
<hr>
<h3>Entities</h3>
<p>Each Entity (the 'E' in ECS) is represented by a single JSON object. 
Each name/value pair in the JSON object represents a Component; the 'C' in ECS.
The name is the name of the Component to create for the entity. The value is any valid
JSON type, including an object itself if the Component has many pieces of data and/or 
creation parameters.</p>
<p>The available Components depend on which Systems have been initialized for the current scene. However, some
Components are always available, and some of those are special. These are:</p>
<table border="1"><tr><th>Name</th><th>Function</th></tr>
<tr><td>id</td><td>In scripts and other entities, the id instances of this entity will be known by (getting an entity by id also requires an index)</td></tr>
<tr><td>Transform2D</td><td>Transform Components (this and the eventual 3D counterpart) automatically create instances of the entity with different transforms. 
eg how to place more than one of an object into the scene without having to copy/paste. Each triplet of floats will get interpreted as x, y, and angle (degrees)
for one instance.</td></tr></td>
<tr><td>tags</td><td>Some Components just act as a flag or category marker with no data. As an artifact of JSON, rather 
than require an unused value for all Components, the Tags Component takes an array of the tag names.</td></tr>
</table>
<hr>
<h3>Templates</h3>
<p>Templates are basically prototype entities that can be dynamically spawned into the scene later on via scripting.
The main difference between templates and entities json-wise, is that templates use a "template-id" component instead of regular "id".
In fact, do not use "id" in a template (in the future the system may convert "id" to "template-id" automatically, but don't
depend on it). The TransformND components must not be used in a template. The template-related script functions are:</p>
<table border="1">
<tr><th>Name</th><th>Purpose</th></tr>
<tr><td>findTemplate(string template-id)</td><td>Takes a template id and returns the template if found, otherwise nil.</td></tr>
<tr><td>template:spawnAtWithID(entity/id, x, y, angle)</td><td>Spawns a template under an entity ID either given
by example of an entity or a string ID. Returns new Entity.</td></tr>
<tr><td>template:spawnAt(x, y, angle)</td><td>Spawns a new entity based on the template and returns it</td></tr>

</table>

<hr>
<h3>Systems</h3>
<p>Currently the following Systems are available:</p>
<table border="1"><tr><th>Name</th><th>Purpose</th></tr>
<tr><td>Audio</td><td>Loads <a href="https://github.com/space222/sogg">Sogg</a> sound files into entities so the id can be referenced by scripts that play effects and music</td></tr>
<tr><td>Render2D</td><td>Provides 2D rendering; registers components for loading images, referencing and cropping from atlases,
and animation.</td></tr>
<tr><td>Chipmunk</td><td><em>Deprecated</em>. A limited 2D physics system based on <a href="https://chipmunk-physics.net/">Chipmunk2D</a>.</td></tr>
<tr><td>Box2D</td><td>The 2D physics system based on (go figure) <a href="https://box2d.org/">Box2D</a>.</td></tr>
<tr><td>ID</td><td>Required system, managing entity/template IDs. Will be automatically included if not listed in settings</td></tr>
</table>
<hr>
<h3>Audio</h3>
<p>The Audio system provides the "sound-item" Component, which takes a filename (string) of the sound file to load.
An Entity using the "sound-item" component must also have an 'id' component, otherwise there will be no way to reference
the data elsewhere to get it to play. The system setting "default-music" may be given the id of the an entity with the
"sound-item" component. The referenced component will play (and loop continuously) once the system's "begin" function 
has been called. All other uses of the Audio system require scripting.</p>
<p>Must be included in "begin-calls" global setting in order to use the "default-music" setting. There are no ordering requirements.</p>
<hr>
<h3>Render2D</h3>
<p>The Render2D system provides rendering of 2D graphics. The "image" component loads an image file. The "image-ref" is required
to be used by another entity referencing the id of the (entity with the) loaded image. "image-ref" either takes just the id, or an
object with "id" and "crop" properties. "crop" takes an array of 4 integers that represent x,y,widty,height of the area to use. 
The "animation2d" component declares an animation based on croping the first frame and assuming "num-frames" of horizontal tiling.
The "fps" properties defines speed. "anim2d-ref" component takes a single "id" property referencing the entity containing the 
animation2d.
</p>
<p>Must be included in "render-calls" global setting. Has no ordering requirements.</p>
<hr>
<h3>Box2D</h3>
<p>The Box2D system is a simplistic 2D physics implementation using Box2D. Box2D requires a scale factor as it operates with
real units (eg meters) rather than pixels, however the numbers in the ggsf Entities section are all in pixels. The scale factor
to use defaults to 30.0, but use the system-setting "scale-factor" to change it (not usually necessar). 
The "physics2d" component adds a physics body to an entity.
</p>
<p>The non-shape properties of the "physics2d" component are friction, restitution, and mass. All take a float. All the shape
properties take an array of floats. "segments" and "polygon" are points (even though it's a flat array). "circles" takes 
x, y, radius (, repeating because, again, flat json array). "box" takes a width and height <em>only</em>. Take note that "box" and
"polygon" are singular; while "box" takes specifically two floats, the number of vertices of "polygon" is determined by the size of
the array. The "type" property defaults to dynamic, but also supports "static" and "kinematic".</p>
<p>Box2D adds many of the rigidbody functions basically directly to the GGSF/Lua "Entity" type (eg addVelocity). Units are pixels
in the scene file, but will be Box2D units (meters, forces, all the things) everywhere else except functions that modify the position
of a rigidbody directly which will still take pixels and convert accordingly. When using 2d physics, the physics system "owns" the
entity transform, and the rendering system's functions will no longer work for 'warping' the object. Use the physics system's 
transform altering functions only for entities that have a physics component.</p>
<hr>
<h3>Chipmunk</h3>
<p>The Chipmunk system is deprecaded. Use Box2D instead.</p>
<p>The Chipmunk system is a simplistic 2D physics implementation using Chipmunk2D. The "physics2d" component adds physics to an
entity. It takes an object of parameters. "type" is either zero for dynamic (default), one for static, and two for kinematic.
The rest of the parameters are the shapes that can be added to the body, "segments", "circles", "boxes", and "polygon" (singular).
Each one (except polygon) is an array of floats interpreted as multiple shapes. Each one starts with mass and friction. 
The segment takes two points (x1,y1,x2,y2). The circle takes radius, x, y. The polygon takes points; since its only one polygon number of
points is detected based on array size. Boxes are x,y,width,height. All points are relative to the body.</p>
<p>The Chipmunk system does its work in the create_instance function due to Chipmunk2D requiring a body be 
created <em>before</em> the shapes to be added to it. If the type is dynamic, a new body will be created for each 
instance created by Transform2D and the body given that transform data. If the type is static/kinematic, the shapes are added to the 
default static body provided by Chipmunk; in this case the shape points must be world coordinates. The entity instance is
inserted into dynamic body's UserData.</p>
<p>Must be included in "update-calls" global setting. Has no ordering requirements. GGScene always calls updates before renders.</p>
<hr>
<h2>System Interface / Creating Systems</h2>
<p>Through abuse of static initialization in C++, Systems can be created and added to GGSF with just a recompile; no need to change any
existing source files. View the class SystemInterface in GGScene.h to see the full picture. In a header, create a class that must
be called SystemNameInterface, where SystemName is the name of your system. In the source file, create a factory function 
called "SystemNameInterface_factory" that takes no parameters and returns a SystemInterface pointer. Then make sure the macro
REGISTER_SYSTEM(SystemName) gets called (last thing in the source file is a good place). Whatever you used for SystemName will get
used (case sensitive) for global settings ("systems", "x-calls", etc).</p>
<p>System Interface functions:</p>
<table border="1"><tr><th>Function</th><th>Purpose</th></tr>
<tr><td>register_components</td><td>Add components to be usable by the scene</td></tr>
<tr><td>init_scripting</td><td>Add types and functions to the scene's Lua state</td></tr>
<tr><td>begin</td><td>called after a scene has been initially loaded (probably could have been called 'init')</td></tr>
<tr><td>activate</td><td>called when a scene that had been previously loaded is about to be the current scene</td></tr>
<tr><td>update</td><td>called each frame, but is conceptually <em>not</em> a render pass</td></tr>
<tr><td>render</td><td>called each frame, but <em>is</em> rendering</td></tr>
<tr><td>setting</td><td>called after system intialization with the system-settings from the scene parse</td></tr>
</table>
<hr>
<h3>Component Interface / Creating Components</h3>
<p>When a SystemInterface adds a ComponentInterface to the scene, the component is then available
to the scene file by the name set by the System. Unlike SystemInterface, there is no restriction
that the name of the component interface class match the string used here. Do not add the ComponentInterface object to an entity. A different data-only (or data-mostly) struct should be used as the actual component(s). The component interface consists mainly of two functions (see GGScene.h for the full signatures):
</p>
<p>add: This function does an initial parse of the JSON for the component, and either builds the
necessary entity component(s) itself or adds a transitional component for the impending create_instance 
call. It is possible that all the necessary work is done in 'add' and the component will simply be 
duplicated by ggsf into all instances.</p>
<p>create_instance: This function uses a transition component in order to create the 
final component for a particular entity instance. If this only needs to be done once, return false 
and ggsf will stop calling create_instance for further instances (otherwise return true). A JSON object reference
is passed to this function, however this parameter is no longer used and will not be populated when
create_instance is called to spawn a template.</p>

<hr>
<h2>GGSF C++ Library</h2>
<p>Much of the library consists of GGScene that manages most things. The function GGScene::loadFile
should be called using std::async so that the future can be polled while the main program also
displays a loading screen. A GGLoadProgress must be passed to loadFile. This will allow the developer to see how many bytes are ultimately loaded, and if this is known when development of a scene is complete to set GGLoadProgress::Expected in order to obtain a percentage during loading
if desired. When an OpenGL backend is implemented, a GGUploadQueue will also need to be passed to loadFile due to GL being single threaded (simpler than context sharing). The current Direct3D backend requires the device be concurrent-create capable. If GGScene::loadFile returns nullptr,
some type of failure occurred; better error info to be implemented.</p>
<p>Before entering a game-loop with a scene, call begin(). GGScene has a member with the clear-color it loaded from global settings, and then update() (which may return a value indicating that a new scene should be loaded), and render() should be called in
that order. The rest is up to the developer.</p>
<hr>
<h2>Toplevel Scene List</h2>
<p>The Toplevel Scene List (TSL) is (once again) a JSON formatted file (scenes.list by default) that at its most
basic lists the available scenes and maps script IDs to Zip archives. The TSL consists of one large JSON object
containing at least "scenes" that takes an array of scene objects and "settings" which takes an object of ... settings. 
A scene object takes the following properties: "id", "file", and "expected-size". "id" and "file" are 
self-explanatory strings. "expected-size" provides a way for the system to calculate a percent-loaded when 
rendering a loading screen.</p>
<p>The settings object's properties must include "start-scene" which takes the id of the initial scene to run on
startup. Also available is "load-first" which takes a list of scenes (by id) to load (which will 
include "start-scene" even if not listed).</p>
<p>The TSL may also eventually provide a list of loading screens to cycle between, as well as startup splash
screens.</p>
</p>
<hr>
<h2>Scripting / Lua</h2>
<p>Scripting in GGSF uses Lua; it is accessed via functions in GGScene. Systems add functions and types 
to the scene's scripting state. The global setting "script-file" will load a file that contains all the 
scripting used in the scene. A function "update()", if it exists, will be the first thing called by 
GGScene::update(). In fact, the scene calls "begin", "activate", and "update" from the script if they 
exist <em>before</em> processing the SystemInterface calls. It is possible to do an entire scene or 
even the whole game's logic in the update script function, instead of C++.</p>
<p>To a large extent, the available script functions depend on the Systems used by the scene. Any System
add free functions to the environment as well as members to the "Entity" class. Sometimes names may collide.
Systems should use as descriptive names as possible (eg Render related systems using 2D/3D suffixes on
transform-related things). To further mitigate this potential problem, Systems are encouraged to also
make the same functions available as part of a "System class type" Lua table.</p>

<hr>
<h2>Building GGSF / Demo</h2>
<p>The "demo" / App.cpp depends on a "blank core app" C++ project. You'll need libvorbis/libogg for 
the Audio system. Chipmunk should be self-explanatory. Nuklear and its system will likely get removed
 in favor of a different game-settings-UI system, so delete those 3 files. The rest is headers like 
<a href="https://github.com/nlohmann/json">nlohmann/json</a> , <a href="https://github.com/thephd/sol2">Sol3</a> , 
and EnTT which you'll need to add to include path. stb_image 
(by <a href="https://github.com/nothings/stb">Sean Barrett</a>) and 
<a href="https://github.com/richgel999/miniz">miniz</a> which are included directly. 
The HLSL require Shader Model 5. Set them to create headers/bytecode variables based on file name.</p>


</body>