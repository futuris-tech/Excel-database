#include <iostream>
#include <random>
#include <chrono>
#include <OpenXLSX.hpp>
#include <time.h>
#include "sha256.h"
using namespace OpenXLSX;

const int Count = 1000;

extern const char* const m_names[];
extern const char* const f_names[];
extern const char* const surnames[];
extern const char* const cities[];
extern const char* const countries[];
extern const char* const classes[];
extern const int m_name_num;
extern const int f_name_num;
extern const int surname_num;
extern const int city_num;
extern const int class_num;


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
		do {
			value *= value;
			value /= dre.max();
		} while (--pow);
		return (value * range) >> 32;
	}

	char* get_name() {
		static char buffer[128];
		const char* input;
		bool m = dre() % 2;
		if (m)	input = m_names[dre() % m_name_num];
		else	input = f_names[dre() % f_name_num];
		strcpy(buffer, input);

		auto ptr = buffer + strlen(input);
		*ptr++ = ' ';

		input = surnames[dre() % surname_num];
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
	auto wks = doc.workbook().worksheet("Sheet1");

	wks.column(1).setWidth(21);
	wks.column(2).setWidth(8);
	wks.column(3).setWidth(11);
	wks.column(4).setWidth(17);
	wks.column(5).setWidth(16);
	wks.column(6).setWidth(16);
	wks.column(7).setWidth(12);
	wks.column(8).setWidth(10);
	wks.column(9).setWidth(11);
	wks.column(10).setWidth(65);

	wks.cell(1, 1).value() = u8"Имя";
	wks.cell(1, 2).value() = u8"Возраст";
	wks.cell(1, 3).value() = u8"Паспорт";
	wks.cell(1, 4).value() = u8"Гражданство";
	wks.cell(1, 5).value() = u8"Откуда";
	wks.cell(1, 6).value() = u8"Куда";
	wks.cell(1, 7).value() = u8"Стоимость, $";
	wks.cell(1, 8).value() = u8"Класс";
	wks.cell(1, 9).value() = u8"Дата";
	wks.cell(1, 10).value() = u8"SHA256";

	std::cout.precision(3);
	std::cout.width(8);
	randomaizer dre;
	for (int i = 2; i <= Count+1; i++) {
		int city_id[] = { dre() % city_num, dre() % city_num };
		const char* country;
		auto r = dre() % 11;
		if (r < 5)		country = countries[city_id[0]];
		else if (r < 10)country = countries[city_id[1]];
		else			country = countries[dre() % city_num];

		auto name = dre.get_name();
		auto age = dre() % 85 + 5;
		auto class_id = dre.get_small(class_num, 2);
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

		char date[128];
		time_t raw = time(0) + dre.get_small(365 * 60*60*24, 3);
		tm info = *localtime(&raw);
		strftime(date, 128, "%d.%m.%Y", &info);

		wks.cell(i, 1).value() = name;
		wks.cell(i, 2).value() = age;
		wks.cell(i, 3).value() = (dre() ^ (dre() << 16)) % 99999999 + 500000000;
		wks.cell(i, 4).value() = country;
		wks.cell(i, 5).value() = cities[city_id[0]];
		wks.cell(i, 6).value() = cities[city_id[1]];
		wks.cell(i, 7).value() = cost;
		wks.cell(i, 8).value() = classes[class_id];
		wks.cell(i, 9).value() = date;

		itoa(age, name + strlen(name), 10);
		wks.cell(i, 10).value() = sha256(name);

		float progress = (i - 1) * (100.0f / Count);
		printf("\r%.1f%%", progress);
	}
	doc.save();
	return 0;
}
