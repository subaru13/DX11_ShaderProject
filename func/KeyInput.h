#pragma once
#include "FrameworkConfig.h"
#include <map>
#include <Windows.h>
#include <memory>

class KeyManager final
{
public:
	struct KeyObject
	{
		short l;
		short n;
	};
private:
	std::map<char, std::shared_ptr<KeyObject>> keys;
	KeyManager() = default;
public:

	static KeyManager* instance()
	{
		static KeyManager ins;
		return &ins;
	}

	void update()
	{
		if (keys.empty())return;
		for (auto it = keys.begin(); it != keys.end();)
		{
			if (it->second.use_count() > 1)
			{
				it->second->l = it->second->n;
				it->second->n = ::GetKeyState(static_cast<int>(it->first));
				++it;
			}
			else
			{
				it = keys.erase(it);
			}
		}
	}

	std::shared_ptr<KeyObject>& addKey(char key)
	{
		auto it = keys.find(key);
		if (it == keys.end())
		{
			it = keys.emplace(key, std::make_shared<KeyObject>()).first;
		}
		return it->second;
	}

	~KeyManager()
	{
		keys.clear();
	}
};


class Key final
{
private:
	Key& operator=(Key&) = delete;
	const char key;
	std::shared_ptr<KeyManager::KeyObject> obj;
	short trg()const { return (~obj->l) & obj->n; }
	short rel()const { return obj->l & (~obj->n); }
public:
	Key(Key& k) :key(k.key), obj(k.obj) {}
	Key(char key) :key(key), obj(KeyManager::instance()->addKey(key)) {}

	//キーが押されたタイミングでtrue
	bool down()const
	{
		return trg() & 0x80;
	}
	//キーが押され続けている間true
	bool hold()const
	{
		return obj->n & 0x80;
	}
	//キーが離されたタイミングでtrue
	bool up()const
	{
		return rel() & 0x80;
	}

	const char& getLabel()const { return key; }

	~Key()
	{
		obj.reset();
	}
};

class Mouse final
{
private:
	POINT pos;
	Key lButton;
	Key rButton;
	Key cButton;
	Mouse() :lButton(MK_LBUTTON), rButton(MK_RBUTTON), cButton(VK_MBUTTON), pos() {}
public:
	void update(HWND hwnd)
	{
		GetCursorPos(&pos);
		ScreenToClient(hwnd, &pos);
		RECT rc;
		GetClientRect(hwnd, &rc);
		float screenW = (float)(rc.right - rc.left);
		float screenH = (float)(rc.bottom - rc.top);
		pos.x = static_cast<LONG>((float)pos.x * ((float)SCREEN_WIDTH / screenW));
		pos.y = static_cast<LONG>((float)pos.y * ((float)SCREEN_HEIGHT / screenH));
	}

	const POINT& getPos()const { return pos; }
	const Key& getL()const { return lButton; }
	const Key& getR()const { return rButton; }
	const Key& getC()const { return cButton; }

	static Mouse* instance()
	{
		static Mouse ins;
		return &ins;
	}
};