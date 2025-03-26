#pragma once
#include <string>
#include <vector>

#include <anime/animation.h>

class Notify
{
public:
	Notify(std::string text, std::string title = "Notify");
	std::string GetText();
	std::string GetTitle();
	float GetSpawnTime();
	float GetLifeValue(float maxLifeTime);
	Animation animation = Animation(.1f);
private:
	std::string text;
	std::string title;
	float spawnTime = 0;
};


inline Notify::Notify(std::string text, std::string title)
{
	this->text = text;
	this->title = title;
	spawnTime = Animation::GetTime();
	animation.Start(Animation::forward);
}

inline std::string Notify::GetText()
{
	return this->text;
}

inline std::string Notify::GetTitle()
{
	return this->title;
}

inline float Notify::GetSpawnTime()
{
	return this->spawnTime;
}

inline float Notify::GetLifeValue(float maxLifeTime)
{
	return 1 - (Animation::GetTime() - this->spawnTime) / maxLifeTime;
}

class NotifyManager
{
private:
	static void AddNotify(std::string text, std::string title = "Notify");

	struct NotifyData
	{
		std::string text;
		std::string title;
	};

public:
	static void AddNotifyToQueue(std::string text, std::string title = "Notify");
	static void HandleNotifyData();
	static void HandleNotifyList();
	static std::vector<Notify>& GetList();
	static inline float maxLifeTime = 3;
	static inline int maxNotifyAmount = 4;

private:
	static inline std::vector<Notify> notifyList;
	static inline std::vector<NotifyData> notifyListQueue;
};

inline void NotifyManager::AddNotify(std::string text, std::string title)
{
	notifyList.insert(notifyList.begin(), Notify(text, title));
}

inline void NotifyManager::AddNotifyToQueue(std::string text, std::string title)
{
	notifyListQueue.insert(notifyListQueue.begin(), NotifyData{ text, title });
}

inline void NotifyManager::HandleNotifyData()
{
	if (notifyListQueue.size() > 5)
		notifyListQueue.erase(notifyListQueue.begin(), notifyListQueue.begin() + 5);

	while (!notifyListQueue.empty())
	{
		notifyList.insert(notifyList.begin(), Notify(notifyListQueue.back().text, notifyListQueue.back().title));
		notifyListQueue.pop_back();
	}
}

inline void NotifyManager::HandleNotifyList()
{
	HandleNotifyData();

	if (notifyList.empty())
		return;

	if (notifyList.size() > (size_t)maxNotifyAmount * 3)
	{
		notifyList.clear();
		return;
	}

	float time = Animation::GetTime();

	for (size_t i = 0; i < notifyList.size(); i++)
	{
		notifyList[i].animation.Proceed();

		float lifeTime = notifyList[i].GetSpawnTime() + maxLifeTime;
		float duration = time + notifyList[i].animation.GetDuration();

		if (lifeTime < duration || (int)i >= maxNotifyAmount || lifeTime < time)
			notifyList[i].animation.Start(Animation::back);
	}

	if (!notifyList.empty() && notifyList.back().animation.AnimationEnded(Animation::back))
		notifyList.pop_back();
}

inline std::vector<Notify>& NotifyManager::GetList()
{
	HandleNotifyList();
	return notifyList;
}