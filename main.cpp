#include <iostream>
#include <random>
#include <chrono>
#include <OpenXLSX.hpp>
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
			value >>= 32ull;
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
	wks.column(3).setWidth(18);
	wks.column(4).setWidth(16);
	wks.column(5).setWidth(16);
	wks.column(6).setWidth(12);
	wks.column(7).setWidth(10);

	wks.cell(1, 1).value() = u8"Имя";
	wks.cell(1, 2).value() = u8"Возраст";
	wks.cell(1, 3).value() = u8"Гражданство";
	wks.cell(1, 4).value() = u8"Откуда";
	wks.cell(1, 5).value() = u8"Куда";
	wks.cell(1, 6).value() = u8"Стоимость, $";
	wks.cell(1, 7).value() = u8"Класс";

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

		wks.cell(i, 1).value() = dre.get_name();
		wks.cell(i, 2).value() = dre() % 85 + 5;
		wks.cell(i, 3).value() = country;
		wks.cell(i, 4).value() = cities[city_id[0]];
		wks.cell(i, 5).value() = cities[city_id[1]];
		wks.cell(i, 6).value() = dre.get_small(5000) + 30;
		wks.cell(i, 7).value() = classes[dre.get_small(class_num, 2)];

		float progress = (i - 1) * (100.0f / Count);
		printf("\r%.1f%%", progress);
	}
	doc.save();
	return 0;
}