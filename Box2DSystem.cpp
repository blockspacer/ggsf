#include "pch.h"
#include <algorithm>
#include "GGScene.h"
#include <Box2D/Box2D.h>
#include "Box2DSystem.h"

const float PI = 3.14159265f;

bool Box2DComponent::create_instance(GGScene*, entt::registry& reg, entt::entity E, nlohmann::json&)
{
	B2DPhysics& phys = reg.get<B2DPhysics>(E);
	Transform2D trans = reg.get<Transform2D>(E);
	b2BodyDef def;
	def.angle = trans.angle * (PI / 180.0f);
	def.position.Set(trans.x / sys->factor(), trans.y / sys->factor());
	
	switch (phys.type)
	{
	case 0: def.type = b2BodyType::b2_dynamicBody; break;
	case 1: def.type = b2BodyType::b2_staticBody;  break;
	case 2: def.type = b2BodyType::b2_kinematicBody; break;
	}
	def.userData = (void*)E;

	phys.body = sys->world()->CreateBody(&def);
	
	b2FixtureDef fixdef;
	for (b2Shape* S : phys.shapes)
	{
		fixdef.shape = S; //S->Clone(&cloneHelper);
		fixdef.density = 1;
		fixdef.restitution = phys.restitution;
		fixdef.friction = phys.friction;
		phys.body->CreateFixture(&fixdef);
	}

	//phys.body->SetTransform(def.position, trans.angle * (PI / 180));

	return true;
}

void Box2DComponent::add(GGScene* scene, entt::registry& reg, entt::entity E, nlohmann::json& J)
{
	B2DPhysics phys;
	phys.friction = (J["friction"].is_number()) ? J["friction"].get<float>() : 1.0f;
	phys.restitution = (J["restitution"].is_number()) ? J["restitution"].get<float>() : 0.3f;
	phys.mass = (J["mass"].is_number()) ? J["mass"].get<float>() : 1.0f;
	phys.type = 0;
	phys.body = nullptr;

	if (J["type"].is_string())
	{
		std::string t = J["type"].get<std::string>();
		if (t == "dynamic") phys.type = 0;
		else if (t == "static") phys.type = 1;
		else if (t == "kinematic") phys.type = 2;
	}

	if (nlohmann::json boxes = J["box"]; boxes.is_array())
	{
		std::vector<float> sm = boxes.get<std::vector<float>>();
		auto* shap = new b2PolygonShape;
		shap->SetAsBox((sm[0] / 2) / sys->factor(), (sm[1] / 2) / sys->factor());
		phys.shapes.push_back(shap);
	}

	if (nlohmann::json pnts = J["polygon"]; pnts.is_array())
	{
		std::vector<float> sm = pnts.get<std::vector<float>>();
		for_each(std::begin(sm), std::end(sm), [=](float& i) { i /= sys->factor(); });
		auto* shap = new b2PolygonShape;
		shap->Set((b2Vec2*)sm.data(), sm.size() / 2);
		phys.shapes.push_back(shap);
	}
	
	if (nlohmann::json circs = J["circles"]; circs.is_array())
	{
		std::vector<float> sm = circs.get<std::vector<float>>();
		for_each(std::begin(sm), std::end(sm), [=](float& i) { i /= sys->factor(); });
		for (int i = 0; i < sm.size() / 3; ++i)
		{
			auto* shap = new b2CircleShape;
			shap->m_p.Set(sm[i * 3], sm[i * 3 + 1]);
			shap->m_radius = sm[i * 3 + 2];
			phys.shapes.push_back(shap);
		}
	}

	reg.assign<B2DPhysics>(E, phys);

	return;
}

void Box2DSystem::update(GGScene* scene)
{
	step += scene->last_frame_time;
	if (step > std::chrono::milliseconds(100))
	{
		step = std::chrono::milliseconds(0);
		m_world->Step(1.0f / 60.0f, 6, 2);
	}
	while (step >= std::chrono::milliseconds(16))
	{
		m_world->Step(1.0f / 60.0f, 6, 2);
		step -= std::chrono::milliseconds(16);
	}

	auto B = m_world->GetBodyList();

	while (B)
	{
		auto temp = B;
		auto UD = B->GetUserData();
		B = B->GetNext();
		if (!UD) continue;

		entt::entity E = (entt::entity)(u64)UD;
		if (!scene->entities.has<Transform2D>(E)) continue;

		auto pos = temp->GetPosition();
		auto ang = temp->GetAngle() * (180.0f / PI);
		scene->entities.replace<Transform2D>(E, pos.x * scale_factor, pos.y * scale_factor, ang);
	}

	return;
}

void Box2DSystem::setting(GGScene*, const std::string& stt, std::variant<std::string, int, float> val)
{
	if (stt == "scale-factor")
	{
		if (std::holds_alternative<int>(val))
		{
			scale_factor =(float) std::get<int>(val);
		} else if (std::holds_alternative<float>(val)) {
			scale_factor = std::get<float>(val);
		}
	}
	return;
}

void Box2DSystem::register_components(GGScene* scene)
{
	scene->components.insert(std::make_pair("physics2d", new Box2DComponent(this)));
	return;
}

Box2DSystem::Box2DSystem() : step(0), scale_factor(30.0f), m_world(new b2World({0, 9.0f}))
{
	return;
}

SystemInterface* Box2DSystem_factory() { return new Box2DSystem; }

REGISTER_SYSTEM(Box2D)