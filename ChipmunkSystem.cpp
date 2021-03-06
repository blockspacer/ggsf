#include "pch.h"
#include "ChipmunkSystem.h"
#include <chipmunk\chipmunk.h>
#include <glm/glm.hpp>

void update_body(cpBody* body, GGScene* scene)
{   // passed as the iterator function to cpSpaceEachBody
	auto ud = cpBodyGetUserData(body);
	entt::entity E = (entt::entity)(u64)ud;
	cpVect p = cpBodyGetPosition(body);
	float angle =(float) cpBodyGetAngle(body);
	Transform2D t((float)p.x, (float)p.y, angle * (180.0/3.1415926));
	scene->entities.assign_or_replace<Transform2D>(E, t);
	return;
}

void ChipmunkSystem::update(GGScene* scene)
{
	step += scene->last_frame_time;
	if (step > std::chrono::milliseconds(100))
	{
		step = std::chrono::milliseconds(0);
		cpSpaceStep(world, 1.0f / 60.0f);
		cpSpaceEachBody(world, (cpSpaceBodyIteratorFunc)update_body, (void*)scene);
	}
	while(step >= std::chrono::milliseconds(16) )
	{
		cpSpaceStep(world, 1.0f / 60.0f);
		cpSpaceEachBody(world, (cpSpaceBodyIteratorFunc)update_body, (void*)scene);
		step -= std::chrono::milliseconds(16);
	}
	
	return;
}

ChipmunkSystem::ChipmunkSystem()
{
	world = cpSpaceNew();
	cpSpaceSetGravity(world, cpVect{ 0, 100 });
	return;
}

void ChipmunkSystem::register_components(GGScene* scen)
{
	scene = scen;
	scene->components.insert(std::make_pair("physics2d", new Physics2DComponent(this)));
	return;
}

bool Physics2DComponent::create_instance(GGScene* scene, entt::registry& reg, entt::entity E, nlohmann::json& J)
{
	int type = 0; //default dynamic
	cpBody* body = nullptr;
	float moment = 0.0f;
	float total_mass = 0.0f;
	int total_shapes = 0;
	std::vector<float> segments;
	std::vector<float> circles;

	if (J.contains("type") && J["type"].is_string())
	{
		std::string t = J["type"].get<std::string>();
		if (t == "static")
		{
			type = 1;
		} else if (t == "kinematic") {
			type = 2;
		} else if (t == "dynamic") {
			type = 0;
		}
		//todo: log error for something else (other than "dynamic"
	}

	if (J.contains("segments") && J["segments"].is_array())
	{
		segments = J["segments"].get<std::vector<float>>();
		// looks like moment of inertia for a segment is zero
		// so might not actually need this.
		total_shapes = (int)segments.size() / 6;
		for (int i = 0; i < segments.size() / 6; ++i)
		{
			cpVect a{ segments[i * 6 + 2], segments[i * 6 + 3] };
			cpVect b{ segments[i * 6 + 4], segments[i * 6 + 5] };
			moment +=(float) cpMomentForSegment(total_mass += segments[i * 6], a, b, 0);
		}
	}

	if (J.contains("circles") && J["circles"].is_array())
	{
		circles = J["circles"].get<std::vector<float>>();
		total_shapes = (int)circles.size() / 5;
		for (int i = 0; i < circles.size() / 5; ++i)
		{
			cpVect p{ circles[i * 5 + 3], circles[i * 5 + 4] };
			moment +=(float) cpMomentForCircle(total_mass += circles[i * 5], 0, circles[i * 5 + 4], p);
		}
	}

	if (type > 0)
	{
		body = cpSpaceGetStaticBody(chip->space());
	} else {
		float mass = total_mass;
		if (J.contains("mass"))
		{
			mass = J["mass"].get<float>();
		}

		body = cpBodyNew(mass, moment);
		cpSpaceAddBody(chip->space(), body);
		Transform2D t;
		if (reg.has<Transform2D>(E)) t = reg.get<Transform2D>(E);
		cpBodySetPosition(body, cpVect{ t.x, t.y });
		cpBodySetAngle(body, t.angle * (3.1415926/180.0));
	}

	for (int i = 0; i < segments.size() / 6; ++i)
	{
		cpVect a{ segments[i * 6 + 2], segments[i * 6 + 3] };
		cpVect b{ segments[i * 6 + 4], segments[i * 6 + 5] };

		cpShape* s = cpSegmentShapeNew(body, a, b, 0);
		cpSpaceAddShape(chip->space(), s);
		cpShapeSetFriction(s, segments[i * 6 + 1]);
	}

	for (int i = 0; i < circles.size() / 5; ++i)
	{
		cpVect p{ circles[i * 5 + 2], circles[i * 5 + 3] };
		cpShape* s = cpCircleShapeNew(body, circles[i * 5 + 4], p);
		cpSpaceAddShape(chip->space(), s);
		cpShapeSetFriction(s, circles[i * 5 + 1]);
	}

	if (type == 0)
	{
		cpBodySetUserData(body, (cpDataPointer)E);
		reg.assign_or_replace<Phys2D>(E, body);
	} else {
		// nothing for static items currently
	}
	
	return true;
}

SystemInterface* ChipmunkSystem_factory()
{
	return new ChipmunkSystem;
}

void ggentity_addVelocity(GGEntity* ent, const glm::vec2& vel)
{
	if (!ent) return;
	cpBody* body = get_member_or(*ent->reg, ent->E, &Phys2D::body, (cpBody*)nullptr);
	if (!body) return;
	cpBodySetVelocity(body, cpBodyGetVelocity(body) + cpVect{ vel.x,vel.y });
	return;
}

void ggentity_setVelocity(GGEntity* ent, const glm::vec2& vel)
{
	if (!ent) return;
	cpBody* body = get_member_or(*ent->reg, ent->E, &Phys2D::body, (cpBody*)nullptr);
	if (!body) return;
	cpBodySetVelocity(body, cpVect{ vel.x,vel.y });
	return;
}

void ChipmunkSystem::init_scripting(GGScene* scene)
{
	auto& lua = scene->lua;
	OutputDebugStringA("\n\nTHIS SHOULD NOT HAPPEN!\n\n");
	lua["Entity"]["setVelocity"] = ggentity_setVelocity;
	lua["Entity"]["addVelocity"] = ggentity_addVelocity;

	return;
}


REGISTER_SYSTEM(Chipmunk)
