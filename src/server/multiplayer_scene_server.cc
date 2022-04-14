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
using multiplayerscene::ColourRequest;
using multiplayerscene::EmptyResponse;

using std::chrono::system_clock;

#include <random>
class MultiplayerSceneImpl final : public MultiplayerScene::Service {
public:
	explicit MultiplayerSceneImpl(unsigned count = 10) {
		std::mt19937_64 engine;
		std::uniform_real_distribution<float> dist{ -10.,10. };
		std::uniform_int_distribution<uint32_t> colourDist{ 0, std::numeric_limits<uint32_t>::max()}; // 32-bit colour
		for (unsigned i{ 1 }; i <= count; ++i) {
			Entity e;
			e.set_id(std::to_string(i));

			Point* p{ new Point };
			p->set_x(dist(engine));
			p->set_y(dist(engine));
			p->set_z(dist(engine));
			e.set_allocated_location(p);

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

	//Status GetFeature(ServerContext* context, const Point* point,
	//                  Feature* feature) override {
	//  feature->set_name(GetFeatureName(*point, m_entities));
	//  feature->mutable_location()->CopyFrom(*point);
	//  return Status::OK;
	//}
	Status GetEntities(ServerContext* context,
		const multiplayerscene::Point* point,
		ServerWriter<Entity>* writer) override {

		// TODO: Return only the entities within a given range.
		for (const Entity& e : m_entities)
			writer->Write(e);

		return Status::OK;
	}
private:
	std::vector<Entity> m_entities;
	std::mutex mu_;
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


