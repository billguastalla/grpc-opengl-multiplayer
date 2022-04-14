/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>


#ifdef BAZEL_BUILD
#include "examples/protos/multiplayer_scene.grpc.pb.h"
#else
#include "multiplayer_scene.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using multiplayerscene::MultiplayerScene;
using multiplayerscene::Point;
using multiplayerscene::Entity;
using multiplayerscene::User;
using multiplayerscene::Quaternion;
using multiplayerscene::MoveByID;
using multiplayerscene::RotateByID;

using multiplayerscene::ColourRequest;
using multiplayerscene::EmptyResponse;

using std::chrono::system_clock;

#include <random>
#include <glm/gtc/quaternion.hpp>

glm::quat toGlmQuat(const Quaternion* quat) {
	return glm::quat{ quat->x(),quat->y(),quat->z(),quat->w() };
}
void toRpcQuat(const glm::quat& glmQ, Quaternion* rpcQuat) {
	rpcQuat->set_x(glmQ.x);
	rpcQuat->set_y(glmQ.y);
	rpcQuat->set_z(glmQ.z);
	rpcQuat->set_w(glmQ.w);
}
void rotateQuat(const Quaternion* rotor, Quaternion* orient) {
	glm::quat result{ toGlmQuat(orient) * toGlmQuat(rotor) };
	toRpcQuat(result, orient);
}

void addPoint(Point* p1, const Point* p2) {
	p1->set_x(p1->x() + p2->x());
	p1->set_y(p1->y() + p2->y());
	p1->set_z(p1->z() + p2->z());
}

std::mt19937_64 engine{};

class MultiplayerSceneImpl final : public MultiplayerScene::Service {
public:
	explicit MultiplayerSceneImpl(unsigned count = 10) {
		for (unsigned i{ 1 }; i <= count; ++i) {
			Entity e;
			e.set_id(std::to_string(i));

			Point* p{ randomPoint() };
			e.set_allocated_location(p);
			Quaternion* q{ randomOrientation() };
			e.set_allocated_orientation(q);

			e.set_colour(colourDist(engine));

			m_entities.push_back(e);
		}
	}

	Status SetEntityColour(ServerContext* context, const ColourRequest* colourRequest, EmptyResponse* resp) override {
		auto entity{
			std::find_if(m_entities.begin(), m_entities.end(),
				[colourRequest](const Entity& e) {return e.id() == colourRequest->id(); })
		};
		if (entity != m_entities.end()) {
			entity->set_colour(colourRequest->colour());
			return Status::OK;
		}
		// TODO: Read all the status codes
		return Status{ grpc::StatusCode::INVALID_ARGUMENT, "Invalid id","Attempted to change colour of an entity with an id that is not found on the server." };
	}

	Status GetEntities(ServerContext* context,
		const multiplayerscene::Point* point,
		ServerWriter<Entity>* writer) override {

		// TODO: Return only the entities within a given range.
		for (const Entity& e : m_entities)
			writer->Write(e);

		return Status::OK;
	}

	Status PushEntity(ServerContext* context, const multiplayerscene::MoveByID* move, multiplayerscene::EmptyResponse* emptyResp) override {
		auto existingEntity{ std::find_if(m_entities.begin(),m_entities.end(),[move](const Entity& e) {return move->id() == e.id(); }) };
		if (existingEntity != m_entities.end()) {
			addPoint(existingEntity->mutable_location(), &move->direction());
			return Status::OK;
		}
		return Status{ grpc::StatusCode::INVALID_ARGUMENT, "Invalid id","Attempted to push an entity with an id that is not found on the server." };
	}
	Status RotateEntity(ServerContext* context, const multiplayerscene::RotateByID* rotate, multiplayerscene::EmptyResponse* emptyResp) override {
		auto existingEntity{ std::find_if(m_entities.begin(),m_entities.end(),[rotate](const Entity& e) {return rotate->id() == e.id(); }) };
		if (existingEntity != m_entities.end()) {
			rotateQuat(&rotate->rotation(), existingEntity->mutable_orientation());
			return Status::OK;
		}
		return Status{ grpc::StatusCode::INVALID_ARGUMENT, "Invalid id","Attempted to rotate an entity with an id that is not found on the server." };
	}

	Status InitialiseUser(ServerContext* context,
		const multiplayerscene::EmptyRequest* emptyReq,
		multiplayerscene::User* user) override {

		user->set_colour(randomColour());
		user->set_id(std::to_string(lastUserID));

		user->set_allocated_location(randomPoint());
		user->set_allocated_orientation(randomOrientation());

		std::string uidstr{ std::to_string(lastUserID) };
		lastUserID++;
		user->set_id(uidstr);
		m_users.push_back(*user);
		std::cout << "User created with id:" << uidstr << "\n";
		return Status::OK;
	}
	Status TerminateUser(ServerContext* context,
		const multiplayerscene::UserID* uid,
		multiplayerscene::EmptyResponse* emptyResp) override {

		std::string uidstr{ uid->id() };
		auto user{ std::find_if(m_users.begin(),m_users.end(),[uidstr](const User& u)
			{ return u.id() == uidstr; }) };
		if (user != m_users.end()) {
			m_users.erase(user);
			std::cout << "User terminated with id:" << uidstr;
			return Status::OK;
		}
		return Status{ grpc::StatusCode::INVALID_ARGUMENT, "Invalid id","Attempted to terminate session of a user that was not found on the server." };
	}

	Status ModifyUser(ServerContext* context, const multiplayerscene::User* user, multiplayerscene::EmptyResponse* emptyResp) override {
		auto existingUser{ std::find_if(m_users.begin(),m_users.end(),[user](const User& u) {return user->id() == u.id(); }) };
		if (existingUser != m_users.end()) {
			existingUser->mutable_location()->set_x(user->location().x());
			existingUser->mutable_location()->set_y(user->location().y());
			existingUser->mutable_location()->set_z(user->location().z());
			existingUser->mutable_orientation()->set_x(user->orientation().x());
			existingUser->mutable_orientation()->set_y(user->orientation().y());
			existingUser->mutable_orientation()->set_z(user->orientation().z());
			existingUser->mutable_orientation()->set_w(user->orientation().w());
			existingUser->set_colour(user->colour());
			return Status::OK;
		}
		return Status{ grpc::StatusCode::INVALID_ARGUMENT, "Invalid id","Attempted to modify a user that was not found on the server." };
	}

	Status GetUsers(ServerContext* context, const multiplayerscene::EmptyRequest* emptyReq, ServerWriter<User>* writer) override {
		for (const auto& user : m_users)
			writer->Write(user);
		return Status::OK;
	}


private:
	std::vector<Entity> m_entities;
	std::vector<User> m_users;
	int lastUserID{ 0 };

	std::mutex mu_;


	Point* randomPoint() const {
		Point* p{ new Point };
		p->set_x(dist(engine));
		p->set_y(dist(engine));
		p->set_z(dist(engine));
		return p;
	}
	Quaternion* randomOrientation() const {
		Quaternion* q{ new Quaternion };
		float x{ dist(engine) }, y{ dist(engine) }, z{ dist(engine) }, w{ dist(engine) };
		float total{ std::sqrt((x * x) + (y * y) + (z * z) + (w * w)) };
		x /= total; y /= total; z /= total; w /= total;
		q->set_x(x); q->set_y(y); q->set_z(z); q->set_w(w);
		return q;
	}
	uint32_t randomColour() const {
		return colourDist(engine);
	}
	std::uniform_real_distribution<float> dist{ -10.,10. };
	std::uniform_int_distribution<uint32_t> colourDist{ 0, std::numeric_limits<uint32_t>::max() }; // 32-bit colour
};

void RunServer() {
	std::string server_address("0.0.0.0:50051");
	MultiplayerSceneImpl service{};

	ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	std::unique_ptr<Server> server{ builder.BuildAndStart() };
	std::cout << "Server listening on " << server_address << std::endl;
	server->Wait();
}

int main(int argc, char** argv) {
	RunServer();
	return 0;
}
