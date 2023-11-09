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
#ifndef KYRLUK_DATA_ENCRYPTION_HPP_INCLUDED
#define KYRLUK_DATA_ENCRYPTION_HPP_INCLUDED

#include <random>
#include <array>
#include <vector>
#include <iomanip>
#include <type_traits>
#include "AES.h"

namespace kurlyk {
    namespace utility {

        /** \brief Сгенерировать ключ
         * \param seed Семя
         * \return Сгенерированный ключ, длиной 32 байта
         */
        std::array<uint8_t, 32> get_generated_key(const uint32_t seed) const noexcept {
            std::mt19937 rand_engine(seed);
            /* Реализация функции rand() в Visual Studio
             * имеет один существенный недостаток —
             * первое генерируемое случайное число не сильно отличается от стартового.
             */
            std::uniform_int_distribution<> dist(0x00, 0xFF);
            std::array<uint8_t, 32> data;
            for(size_t i = 0; i < data.size(); ++i) {
                data[i] = dist(rand_engine);
            }
            return std::move(data);
        }

        /** \brief Зашифровать данные строки
         * Для шифрования используется алгоритм AES ECB с ключем длиной 256 бит.
         * \param data Строка для шифрования
         * \param key Ключ
         * \return Зашифрованная строка
         */
        std::string encrypt_data(
                const std::string &data,
                const std::array<uint8_t, 32> &key) const noexcept {
            if(data.size() == 0) return std::string();
            AES aes(256);
            unsigned int length = 0;
            unsigned char *out = aes.EncryptECB((uint8_t*)data.c_str(), data.size(), (uint8_t*)key.data(), length);
            std::string temp((char*)out, (char*)out + length);
            delete[] out;
            return std::move(temp);
        }

        /** \brief Зашифровать данные массива
         * Для шифрования используется алгоритм AES ECB с ключем длиной 256 бит.
         * \param data Массив данных для шифрования
         * \param key Ключ
         * \return Зашифрованный массив данных
         */
        std::string decrypt_data(
                const std::string &data,
                const uint32_t decrypted_data_length,
                const std::array<uint8_t, 32> &key) const noexcept {
            if(data.size() == 0) return std::string();
            AES aes(256);
            unsigned char *innew = aes.DecryptECB((uint8_t*)data.c_str(), data.size(), (uint8_t*)key.data());
            std::string temp((const char*)innew, (const char*)innew + decrypted_data_length);
            delete[] innew;
            return std::move(temp);
        }

        /** \brief Зашифровать данные массива
         * Для шифрования используется алгоритм AES ECB с ключем длиной 256 бит.
         * \param data Массив для шифрования
         * \param key Ключ
         * \return Зашифрованный массив данных
         */
        std::vector<uint8_t> encrypt_data(
                const std::vector<uint8_t> &data,
                const std::array<uint8_t, 32> &key) const noexcept {
            if(data.size() == 0) return std::vector<uint8_t>();
            AES aes(256);
            unsigned int length = 0;
            unsigned char *out = aes.EncryptECB((uint8_t*)data.data(), data.size(), (uint8_t*)key.data(), length);
            std::vector<uint8_t> temp(out, out + length);
            delete[] out;
            return std::move(temp);
        }

        /** \brief Дешифровать данные массива
         * Для шифрования используется алгоритм AES ECB с ключем длиной 256 бит.
         * \param data Массив для дешифрования
         * \param decrypted_data_length Размер дешифрованных данных
         * \param key Ключ
         * \return Дешифрованный массив данных
         */
        std::vector<uint8_t> decrypt_data(
                const std::vector<uint8_t> &data,
                const uint32_t decrypted_data_length,
                const std::array<uint8_t, 32> &key) const noexcept {
            if(data.size() == 0) return std::vector<uint8_t>();
            AES aes(256);
            unsigned char *innew = aes.DecryptECB((uint8_t*)data.data(), data.size(), (uint8_t*)key.data());
            std::vector<uint8_t> temp(innew, innew + decrypted_data_length);
            delete[] innew;
            return std::move(temp);
        }

        /** \brief Зашифровать данные массива
         * Для шифрования используется алгоритм AES ECB с ключем длиной 256 бит.
         * \param data Массив для шифрования
         * \param key Ключ
         * \return Зашифрованный массив данных
         */
        std::vector<uint8_t> encrypt_data_with_size(
                const std::vector<uint8_t> &data,
                const std::array<uint8_t, 32> &key) const noexcept {
            if(data.size() == 0) return std::vector<uint8_t>();
            AES aes(256);
            unsigned int length = 0;
            unsigned char *out = aes.EncryptECB((uint8_t*)data.data(), data.size(), (uint8_t*)key.data(), length);
            std::vector<uint8_t> temp(out, out + length);
            delete[] out;
            const uint32_t data_length = data.size();
            temp.push_back((0xFF & (data_length >> 24)));
            temp.push_back((0xFF & (data_length >> 16)));
            temp.push_back((0xFF & (data_length >> 8)));
            temp.push_back((0xFF & (data_length >> 0)));
            return std::move(temp);
        }

        /** \brief Дешифровать данные массива
         * Для шифрования используется алгоритм AES ECB с ключем длиной 256 бит.
         * \param data Массив для дешифрования. Конец массива содержит размер дешифрованных данных
         * \param key Ключ
         * \return Дешифрованный массив данных
         */
        std::vector<uint8_t> decrypt_data_with_size(
                const std::vector<uint8_t> &data,
                const std::array<uint8_t, 32> &key) const noexcept {
            if(data.size() < 4) return std::vector<uint8_t>();
            const uint32_t max_index = data.size() - 1;
            const uint32_t decrypted_data_length =
                ((uint32_t)data[max_index - 3] << 24) |
                ((uint32_t)data[max_index - 2] << 16) |
                ((uint32_t)data[max_index - 1] << 8) |
                ((uint32_t)data[max_index - 0] << 0);
            AES aes(256);
            unsigned char *innew = aes.DecryptECB((uint8_t*)data.data(), data.size() - sizeof(uint32_t), (uint8_t*)key.data());
            std::vector<uint8_t> temp(innew, innew + decrypted_data_length);
            delete[] innew;
            return std::move(temp);
        }
    };
};

#endif // KYRLUK_DATA_ENCRYPTION_HPP_INCLUDED
