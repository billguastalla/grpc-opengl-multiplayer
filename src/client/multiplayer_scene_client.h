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
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <thread>

#include "../helper.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#ifdef BAZEL_BUILD
#include "examples/protos/multiplayer_scene.grpc.pb.h"
#else
#include "multiplayer_scene.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using multiplayerscene::MultiplayerScene;

using multiplayerscene::Entity;
using multiplayerscene::Point;
using multiplayerscene::ColourRequest;
using multiplayerscene::EmptyResponse;

//Feature MakeFeature(const std::string& name, long latitude, long longitude) {
//  Feature f;
//  f.set_name(name);
//  f.mutable_location()->CopyFrom(MakePoint(latitude, longitude));
//  return f;
//}

std::string colourToString(int32_t colour) {
	return std::string{
	"{" + std::to_string(colour & 255u) + ","
		+ std::to_string((colour & (255u << 8)) >> 8) + ","
		+ std::to_string((colour & (255u << 16)) >> 16) + ","
		+ std::to_string((colour & (255u << 24)) >> 24) + "}" };
}

class MultiplayerSceneClient {
public:
	MultiplayerSceneClient(std::shared_ptr<Channel> channel)
		: stub_(MultiplayerScene::NewStub(channel)) {
	}

	void PrintEntities() {
		Entity entity;
		Point point;
		ClientContext context;

		std::unique_ptr<ClientReader<Entity> > reader(
			stub_->GetEntities(&context, point));
		while (reader->Read(&entity)) {
			std::cout << "Entity: \tid:" << entity.id() << "\tat position:"
				<< "{" << entity.location().x() << "," << entity.location().y() << "," << entity.location().z() << "}"
				<< "  with \tcolour:" <<
				colourToString(entity.colour()) << "\n";
		}
		Status status = reader->Finish();
		if (status.ok()) {
			std::cout << "GetEnities rpc succeeded." << std::endl;
		}
		else {
			std::cout << "GetEnities rpc failed." << std::endl;
		}
	}

	std::vector<Entity> GetEntities(const Point & userLocation) {
		ClientContext context;
		std::unique_ptr<ClientReader<Entity> > reader(
			stub_->GetEntities(&context, userLocation));
		std::vector<Entity> result{};
		Entity e{};
		while (reader->Read(&e))
			result.push_back(e);
		return result;
	}

	void ChangeColour(std::string id, uint32_t colour) {
		ColourRequest col;
		col.set_id(id);
		col.set_colour(colour);
		EmptyResponse resp;

		ClientContext ctx;
		Status status{ stub_->SetEntityColour(&ctx, col, &resp) };
		if (status.ok())
			std::cout << "Changed colour of entity " << id << " to " << colourToString(colour) << ".\n";
		else
			std::cout << "Colour change of entity " << id << " failed.\n";
	}

private:
	std::unique_ptr<MultiplayerScene::Stub> stub_;
};

