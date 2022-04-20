# grpc-opengl-multiplayer

## Introduction

This is a very quickly written demo of a multiplayer scene, using gRPC and OpenGL.
The server listens for clients to connect and as they create a session it provides them
with user ids and a list of entities that have been generated in the world. 
The client tells the server where its camera position is, and the scene tells each 
client where the other users are.

The fun is in the fact that you can see other users moving around on the screen.

![](https://billguastalla.com/binaries/multiplayer-scene/videos/quick-demo-2022-04-20-14-45-22-brief.gif)

The longer version of this demo video is [here](https://billguastalla.com/binaries/multiplayer-scene/videos/quick-demo-2022-04-20-14-45-22.mp4).

## Usage

Tested only on windows 10 x64, VS2022, using VCPKG.
You must modify the path to gRPC in the top line of the CMakeLists.txt file in src/
to point to your installation of gRPC.

Then do the usual
```
	git clone https://github.com/billguastalla/grpc-opengl-multiplayer.git
	cd grpc-opengl-multiplayer
	mkdir build
	cd build
	cmake .. .
```

## Things that were borrowed

The gRPC service was adapted from google's [examples](https://grpc.io/docs/languages/cpp/basics/). Some graphics utilities have been imported
from my own software [visualisations](https://github.com/billguastalla/visualisations) via cmake, and some of these are also modified from Joey de Vries' 
tutorials on computer graphics.

## Future work

Functionality to rotate and move objects has been descibed in the service definition (.proto)
but not implemented in the client/server code.
