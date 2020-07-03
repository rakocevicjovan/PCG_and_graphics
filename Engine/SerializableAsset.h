#pragma once


class SerializableAsset
{
public:

	SerializableAsset() = delete;

	virtual void Serialize() = 0;
};