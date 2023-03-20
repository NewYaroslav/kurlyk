/*
* kurlyk - C ++ library for easy curl work
*
* Copyright (c) 2021-2023 Elektro Yar. Email: git.electroyar@gmail.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#pragma once
#ifndef KYRLUK_ENCRYPTED_FILE_HPP_INCLUDED
#define KYRLUK_ENCRYPTED_FILE_HPP_INCLUDED

#include "kurlyk-data-encryption.hpp"
#include "nlohmann/json.hpp"
#include <fstream>
#include <dir.h>

namespace kurlyk {
	namespace utility {

		/** \brief Разобрать путь на составляющие
		 * Данная функция парсит путь, например C:/Users\\user/Downloads разложит на
		 * C:, Users, user и Downloads
		 * \param path путь, который надо распарсить
		 * \param output_list список полученных элементов
		 */
		void parse_path(std::string path, std::vector<std::string> &output_list) const noexcept {
			if(path.back() != '/' && path.back() != '\\') path += "/";
			std::size_t start_pos = 0;
			while(true) {
				std::size_t found_beg = path.find_first_of("/\\~", start_pos);
				if(found_beg != std::string::npos) {
					std::size_t len = found_beg - start_pos;
					if(len > 0) {
						if(output_list.size() == 0 && path.size() > 3 && path.substr(0, 2) == "~/") {
							output_list.push_back(path.substr(0, 2));
						} else
						if(output_list.size() == 0 && path.size() > 2 && path.substr(0, 1) == "/") {
							output_list.push_back(path.substr(0, 1));
						}
						output_list.push_back(path.substr(start_pos, len));
					}
					start_pos = found_beg + 1;
				} else break;
			}
		}

		/** \brief Создать директорию
		 * \param path директория, которую необходимо создать
		 * \param is_file Флаг, по умолчанию false. Если указать true, последний раздел пути будет являться именем файла и не будет считаться "папкой".
		 */
		void create_directory(std::string path, const bool is_file = false) const noexcept {
			std::vector<std::string> dir_list;
			parse_path(path, dir_list);
			std::string name;
			size_t dir_list_size = is_file ? dir_list.size() - 1 : dir_list.size();
			for(size_t i = 0; i < dir_list_size; i++) {
				if(i > 0) name += dir_list[i] + "/";
				else if(i == 0 &&
					(dir_list[i] == "/" ||
					dir_list[i] == "~/")) {
					name += dir_list[i];
				} else name += dir_list[i] + "/";
				if (dir_list[i] == ".." ||
					dir_list[i] == "/" ||
					dir_list[i] == "~/") continue;
				mkdir(name.c_str());
			}
		}

		/** \brief Открыть зашифрованный файл
		 * \param file_name Имя файла
		 * \param data Массив данных файла
		 * \param key Ключ для шифрования
		 * \return Вернет true в случае удачного считывания
		 */
		bool open_encrypt_file(
				const std::string &file_name,
				std::vector<uint8_t> &data,
				const std::array<uint8_t, 32> &key) const noexcept {
			std::ifstream file(file_name, std::ios_base::binary);
			if(!file) return false;
			file.seekg(0, std::ios_base::end);
			std::ifstream::pos_type file_size = file.tellg();
			if((size_t)file_size <= sizeof(uint32_t)) {
				file.close();
				return false;
			}
			file.seekg(0);
			std::vector<uint8_t> buffer(file_size);
			file.read((char*)buffer.data(), file_size);
			file.close();
			data = decrypt_data_with_size(buffer, key);
			if(data.size() == 0) return false;
			return true;
		}

		/** \brief Сохранить зашифрованный файл
		 * \param file_name Имя файла
		 * \param data Массив данных файла
		 * \param key Ключ для шифрования
		 * \return Вернет true в случае удачной записи
		 */
		bool save_encrypt_file(
				const std::string &file_name,
				const std::vector<uint8_t> &data,
				const std::array<uint8_t, 32> &key) const noexcept {
			if(data.size() == 0) return false;
			create_directory(file_name,true);
			std::vector<uint8_t> buffer = easy_license::encrypt_data_with_size(data, key);
			if(buffer.size() == 0) return false;
			std::ofstream file(file_name, std::ios_base::binary);
			if(!file) return false;
			file.write(reinterpret_cast<const char*>(&buffer[0]), buffer.size());
			file.close();
			return true;
		}

		/** \brief Открыть зашифрованный файл
		 * \param file_name Имя файла
		 * \param data Строка данных файла
		 * \param key Ключ для шифрования
		 * \return Вернет true в случае удачного считывания
		 */
		bool open_encrypt_file(
				const std::string &file_name,
				std::string &data,
				const std::array<uint8_t, 32> &key) const noexcept {
			std::vector<uint8_t> buffer;
			if(!open_encrypt_file(file_name, buffer, key)) return false;
			data.assign(buffer.begin(), buffer.end());
			return true;
		}

		/** \brief Сохранить зашифрованный файл
		 * \param file_name Имя файла
		 * \param data Строка данных файла
		 * \param key Ключ для шифрования
		 * \return Вернет true в случае удачной записи
		 */
		bool save_encrypt_file(
				const std::string &file_name,
				const std::string &data,
				const std::array<uint8_t, 32> &key) const noexcept {
			return save_encrypt_file(
				file_name,
				std::vector<uint8_t>(data.begin(), data.end()),
				key);
		}

		/** \brief Открыть зашифрованный файл
		 * \param file_name Имя файла
		 * \param j_data Структура JSON данных файла
		 * \param key Ключ для шифрования
		 * \return Вернет true в случае удачного считывания
		 */
		bool open_encrypt_file(
				const std::string &file_name,
				nlohmann::json &j_data,
				const std::array<uint8_t, 32> &key) const noexcept {
			std::string buffer;
			if(!open_encrypt_file(file_name, buffer, key)) return false;
			try {
				j_data = nlohmann::json::parse(buffer);
			}
			catch(...) {
				return false;
			};
			return true;
		}

		/** \brief Сохранить зашифрованный файл
		 * \param file_name Имя файла
		 * \param j_data Структура JSON данных файла
		 * \param key Ключ для шифрования
		 * \return Вернет true в случае удачной записи
		 */
		bool save_encrypt_file(
				const std::string &file_name,
				const nlohmann::json &j_data,
				const std::array<uint8_t, 32> &key) const noexcept {
			return save_encrypt_file(file_name, j_data.dump(), key);
		}
	}; // utility
}; // kurlyk

#endif // KYRLUK_ENCRYPTED_FILE_HPP_INCLUDED
