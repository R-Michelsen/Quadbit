#pragma once
#include <EASTL/vector.h>
#include <EASTL/type_traits.h>

#include <glm/gtx/compatibility.hpp>

#include "Defines.h"

enum class VisibleFaces {
	North = 1,		// 000001
	South = 1 << 1,   // 000010
	East = 1 << 2,   // 000100
	West = 1 << 3,	// 001000
	Top = 1 << 4,	// 010000
	Bottom = 1 << 5	// 100000
};
inline VisibleFaces operator | (VisibleFaces lhs, VisibleFaces rhs) {
	using type = eastl::underlying_type_t<VisibleFaces>;
	return static_cast<VisibleFaces>(static_cast<type>(lhs) | static_cast<type>(rhs));
}
inline VisibleFaces& operator |= (VisibleFaces& lhs, VisibleFaces rhs) {
	lhs = lhs | rhs;
	return lhs;
}
inline VisibleFaces operator & (VisibleFaces lhs, VisibleFaces rhs) {
	using type = eastl::underlying_type_t<VisibleFaces>;
	return static_cast<VisibleFaces>(static_cast<type>(lhs) & static_cast<type>(rhs));
}
inline VisibleFaces& operator &= (VisibleFaces& lhs, VisibleFaces rhs) {
	lhs = lhs & rhs;
	return lhs;
}
inline VisibleFaces operator ~ (const VisibleFaces a) {
	using type = eastl::underlying_type_t<VisibleFaces>;
	return static_cast<VisibleFaces>(~static_cast<type>(a));
}

enum class FillType {
	Empty,
	Solid
};

enum class Region {
	MagicaVoxel,
	Forest
};

struct Voxel {
	FillType fillType;
	VisibleFaces visibleFaces;
	Region region;
	glm::float3 colour;
};

struct VoxelBlockUpdateTag : Quadbit::EventTagComponent {};
struct MeshGenerationUpdateTag : Quadbit::EventTagComponent {};
struct MeshReadyTag : Quadbit::EventTagComponent {};

struct VoxelVertex {
	glm::vec3 pos;
	glm::vec3 col;
};

struct VoxelBlockComponent {
	// Could be stored in an rray but might overflow stack size
	// Heap allocated in a vector seems like the best option
	VoxelBlockComponent() {
		voxels.resize(VOXEL_BLOCK_SIZE);
	}
	eastl::vector<Voxel> voxels;

	// Vectors necessary for mesh generation
	eastl::vector<VoxelVertex> vertices;
	eastl::vector<uint32_t> indices;
};

const eastl::vector<VoxelVertex> cubeVertices = {
{{-1.0f, -1.0f, 1.0f},	{1.0f, 0.0f, 0.0f}},
{{1.0f, -1.0f, 1.0f},	{0.0f, 1.0f, 0.0f}},
{{1.0f, 1.0f, 1.0f},	{0.0f, 0.0f, 1.0f}},
{{-1.0f, 1.0f, 1.0f},	{1.0f, 1.0f, 0.0f}},

{{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
{{1.0f, -1.0f, -1.0f},	{0.0f, 1.0f, 0.0f}},
{{1.0f, 1.0f, -1.0f},	{0.0f, 0.0f, 1.0f}},
{{-1.0f, 1.0f, -1.0f},	{1.0f, 1.0f, 0.0f}}
};

const eastl::vector<uint32_t> cubeIndices = {
	0, 1, 2,
	2, 3, 0,
	1, 5, 6,
	6, 2, 1,
	7, 6, 5,
	5, 4, 7,
	4, 0, 3,
	3, 7, 4,
	4, 5, 1,
	1, 0, 4,
	3, 2, 6,
	6, 7, 3
};

struct PlayerTag {};