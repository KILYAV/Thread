#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <future>
#include <string>

using  PCHAR = char*;

struct match
{
	unsigned line;
	unsigned pos;
	std::string string;
};
using RESULT = std::list<match>;

std::ifstream file;

std::promise<RESULT> promise;
std::future<RESULT> future;

PCHAR mask, endMask;

unsigned long long sizeMask;
unsigned threadCount;

void find(PCHAR text, unsigned sizeText, std::promise<RESULT> promise)
{
	auto Compare = [](PCHAR text, PCHAR mask, unsigned long long sizeMask)
	{
		for (unsigned pos = 1; pos < sizeMask; pos++)
			if (char& letter = mask[pos]; '?' != letter && text[pos] != letter)
				return false;
		return true;
	};

	std::thread next;
	std::future<RESULT> goal;

	if (--threadCount)
	{
		unsigned pos = sizeText / (threadCount + 1);
		for (; pos > 0; pos--)
			if (text[pos] == '\n') break;
		if (pos != 0)
		{
			std::promise<RESULT> pass;
			goal = pass.get_future();
			pos++;
			next = std::thread{ find, text + pos, sizeText - pos, std::move(pass) };
			sizeText = pos;
		}
	};

	unsigned line = 1;
	PCHAR startLine = text;
	RESULT temp;

	for (PCHAR endText = text + sizeText - sizeMask; text < endText; text++)
		if ('\n' == *text)
			line++, startLine = text += 1;
		else if ('\n' == *(text + sizeMask - 1))
			startLine = text += sizeMask, line++;
		else if (char letter = *mask; *text == letter || '?' == letter)
			if (Compare(text, mask, sizeMask))
			{
				temp.push_back(match{ line, static_cast<unsigned>(text - startLine + 1),
					std::string(text, sizeMask) });
				text += sizeMask;
			};

	if (goal.valid())
	{
		next.join();
		RESULT prey = goal.get();
		for (auto& member : prey)
		{
			member.line += line;
			temp.push_back(member);
		}
	}

	promise.set_value(temp);
}

int main(int argc, char** argv)
{
	file.open(argv[1]);
	mask = argv[2];
	sizeMask = std::strlen(mask);
	threadCount = std::thread::hardware_concurrency();

	unsigned sizeText = static_cast<unsigned>(file.seekg(0, std::ios::end).tellg());
	file.seekg(0, std::ios::beg);

	PCHAR text = new char[sizeText];
	file.read(text, sizeText);
	file.close();

	future = promise.get_future();
	find(text, sizeText, std::move(promise));

	RESULT result = future.get();
	delete[] text;

	std::cout << result.size() << std::endl;
	for (auto& member : result)
		std::cout << member.line << " " << member.pos << " " << member.string << std::endl;
}
