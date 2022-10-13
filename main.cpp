#include <iostream>
#include <random>
#include <chrono>
#include <OpenXLSX.hpp>
#include <time.h>
#include <string.h>
#include <array>
#include "sha256.h"
#include "head.h"
using namespace std;
using namespace OpenXLSX;

const int Count = 10000;

extern const array<const char*, 40> m_names;
extern const array<const char*, 44> f_names;
extern const array<const char*, 250> surnames;
extern const array<place, 209> cities;
extern const array<place, 16> summer;
extern const array<const char*, 3> classes;

class randomaizer {
	struct city {
		char* name;
		char* coutry;
	};

	std::default_random_engine dre;
public:
	randomaizer() : dre(
		std::chrono::system_clock().now().time_since_epoch().count()) {}

	uint32_t operator()() { return dre(); }
	inline uint32_t get_small(uint32_t range, int pow = 1) {
		uint64_t value = dre();
		do value = value * value / dre.max();
		while (--pow);
		return value * range / dre.max();
	}

	char* get_name() {
		static char buffer[128];
		const char* input;
		bool m = dre() % 2;
		if (m)	input = m_names[dre() % m_names.size()];
		else	input = f_names[dre() % f_names.size()];
		strcpy(buffer, input);

		auto ptr = buffer + strlen(input);
		*ptr++ = ' ';

		input = surnames[dre() % surnames.size()];
		strcpy(ptr, input);

		ptr += strlen(input);
		if (!m) {
			strcpy(ptr, u8"а");
			ptr += 2;
		}
		*ptr++ = 0;
		return buffer;
	}
};

int main() {
	XLDocument doc;
	doc.create("Spreadsheet.xlsx");
	auto wb = doc.workbook();
	wb.addWorksheet("Sheet2");
	auto wks1 = wb.worksheet("Sheet1");
	auto wks2 = wb.worksheet("Sheet2");

	wks1.cell(1, 1).value() = u8"Имя";
	wks1.cell(1, 2).value() = u8"Возраст";
	wks1.cell(1, 3).value() = u8"Паспорт";
	wks1.cell(1, 4).value() = u8"Гражданство";
	wks1.cell(1, 5).value() = u8"SHA256";

	wks1.column(1).setWidth(21);
	wks1.column(2).setWidth(8);
	wks1.column(3).setWidth(11);
	wks1.column(4).setWidth(17);
	wks1.column(5).setWidth(65);

	wks2.cell(1, 1).value() = u8"Откуда";
	wks2.cell(1, 2).value() = u8"Куда";
	wks2.cell(1, 3).value() = u8"Стоимость, $";
	wks2.cell(1, 4).value() = u8"Класс";
	wks2.cell(1, 5).value() = u8"Дата";
	wks2.cell(1, 6).value() = u8"SHA256";

	wks2.column(1).setWidth(16);
	wks2.column(2).setWidth(16);
	wks2.column(3).setWidth(12);
	wks2.column(4).setWidth(10);
	wks2.column(5).setWidth(11);
	wks2.column(6).setWidth(65);

	std::cout.precision(3);
	std::cout.width(8);
	randomaizer dre;
	for (int i = 2; i <= Count+1; i++) {
		char date[128];
		time_t raw = time(0) + dre.get_small(365 * 60*60*24);
		tm info = *localtime(&raw);
		strftime(date, 128, "%d.%m.%Y", &info);

		place places[2];
		const char* country;
		if (info.tm_mon >= 5 && info.tm_mon <= 9 && dre() % 3) {
			places[0] = cities[dre() % (int)cities.size()];
			places[1] = summer[dre() % (int)summer.size()];
			country = places[0].country;
			if (dre() % 2)
				std::swap(places[0], places[1]);
		}
		else {
			places[0] = cities[dre() % (int)cities.size()];
			places[1] = cities[dre() % (int)cities.size()];
			auto r = dre() % 11;
			if (r < 5)		country = places[0].country;
			else if (r < 10)country = places[1].country;
			else			country = cities[dre() % summer.size()].country;
		}

		auto name = dre.get_name();
		auto age = dre() % 85 + 5;
		auto class_id = dre.get_small(classes.size(), 2);
		int cost;
		switch (class_id) {
		case 0:
			cost = dre.get_small(1000) + 30;
			break;
		case 1:
			cost = dre() % 2000 + 500;
			break;
		case 2:
			cost = dre() % 3500 + 1500;
			break;
		default:
			cost = 0;
		}


		wks1.cell(i, 1).value() = name;
		wks1.cell(i, 2).value() = age;
		wks1.cell(i, 3).value() = (dre() ^ (dre() << 16)) % 99999999 + 500000000;
		wks1.cell(i, 4).value() = country;
		wks2.cell(i, 1).value() = places[0].city;
		wks2.cell(i, 2).value() = places[1].city;
		wks2.cell(i, 3).value() = cost;
		wks2.cell(i, 4).value() = classes[class_id];
		wks2.cell(i, 5).value() = date;

		*(int*)(name + strlen(name)) = age;
		auto sha = sha256(name);
		wks1.cell(i, 5).value() = sha;
		wks2.cell(i, 6).value() = sha;

		float progress = (i - 1) * (100.0f / Count);
		printf("\r%.1f%%", progress);
	}
	doc.save();
	return 0;
}
