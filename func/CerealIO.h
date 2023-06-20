#pragma once
#include <DirectXMath.h>
#include <fstream>
#include <algorithm>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/common.hpp>
#include <cereal/archives/json.hpp>

namespace DirectX
{
	template<class T>
	void serialize(T& archive, XMFLOAT2& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y)
		);
	}

	template<class T>
	void serialize(T& archive, XMFLOAT3& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z)
		);
	}

	template<class T>
	void serialize(T& archive, XMFLOAT4& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z),
			cereal::make_nvp("w", v.w)
		);
	}

	template<class T>
	void serialize(T& archive, XMFLOAT4X4& m)
	{
		archive(
			cereal::make_nvp("_11", m._11), cereal::make_nvp("_12", m._12), cereal::make_nvp("_13", m._13), cereal::make_nvp("_14", m._14),
			cereal::make_nvp("_21", m._21), cereal::make_nvp("_22", m._22), cereal::make_nvp("_23", m._23), cereal::make_nvp("_24", m._24),
			cereal::make_nvp("_31", m._31), cereal::make_nvp("_32", m._32), cereal::make_nvp("_33", m._33), cereal::make_nvp("_34", m._34),
			cereal::make_nvp("_41", m._41), cereal::make_nvp("_42", m._42), cereal::make_nvp("_43", m._43), cereal::make_nvp("_44", m._44)
		);
	}
}

template<class... Args>
void Serialize(const char* filepath, Args&&... args)
{
	std::ofstream ofs{ filepath ,std::ios::out };
	if (ofs)
	{
		cereal::JSONOutputArchive archive{ ofs };
		archive(std::forward<Args>(args)...);
	}
}

template<class... Args>
void Deserialize(const char* filepath, Args&&... args)
{
	std::ifstream ifs{ filepath ,std::ios::in };
	if (ifs)
	{
		cereal::JSONInputArchive archive{ ifs };
		archive(std::forward<Args>(args)...);
	}
}
