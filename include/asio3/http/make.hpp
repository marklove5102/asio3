/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include <asio3/core/stdutil.hpp>

#include <asio3/http/core.hpp>
#include <asio3/http/mime_types.hpp>

#ifdef ASIO3_HEADER_ONLY
namespace bho::beast::http
#else
namespace boost::beast::http
#endif
{
	template<class StringT = std::string_view>
	inline std::string make_error_page(http::status result, StringT&& desc = std::string_view{})
	{
		std::string_view reason = http::obsolete_reason(result);
		std::string_view descrb = asio::to_string_view(desc);
		std::string content;
		if (descrb.empty())
			content.reserve(reason.size() * 2 + 67);
		else
			content.reserve(reason.size() * 2 + 67 + descrb.size() + 21);
		content += "<html><head><title>";
		content += reason;
		content += "</title></head><body><h1>";
		content += std::to_string(std::to_underlying(result));
		content += " ";
		content += reason;
		content += "</h1>";
		if (!descrb.empty())
		{
			content += "<p>Description : ";
			content += descrb;
			content += "</p>";
		}
		content += "</body></html>";
		return content;
	}

	/**
	 * @brief Respond to http request with plain text content
	 * @param content - the response body, it's usually a simple string,
	 * and the content-type is "text/plain" by default.
	 */
	template<typename = void>
	inline http::response<http::string_body> make_text_response(
		asio::is_string auto&& content, http::status result = http::status::ok,
		std::string_view mimetype = "text/plain", unsigned version = 11)
	{
		http::response<http::string_body> rep{};

		rep.set(http::field::server, BEAST_VERSION_STRING);
		rep.set(http::field::content_type, mimetype.empty() ? "text/plain" : mimetype);

		rep.result(result);
		rep.version(version < 10 ? 11 : version);

		rep.body() = asio::to_string(std::forward_like<decltype(content)>(content));

		try
		{
			rep.prepare_payload();
		}
		catch (const std::exception&)
		{
			// The response body MUST be empty for this case
			rep.body() = "";
			rep.prepare_payload();
		}

		return rep;
	}

	/**
	 * @brief Respond to http request with json content
	 */
	template<typename = void>
	inline http::response<http::string_body> make_json_response(
		asio::is_string auto&& content, http::status result = http::status::ok,
		std::string_view mimetype = "application/json", unsigned version = 11)
	{
		return make_text_response(std::forward_like<decltype(content)>(content), result,
			mimetype.empty() ? "application/json" : mimetype, version);
	}

	/**
	 * @brief Respond to http request with html content
	 * @param content - the response body, may be a plain text string, or a stardand
	 * <html>...</html> string, it's just that the content-type is "text/html" by default.
	 */
	template<typename = void>
	inline http::response<http::string_body> make_html_response(
		asio::is_string auto&& content, http::status result = http::status::ok,
		std::string_view mimetype = "text/html", unsigned version = 11)
	{
		return make_text_response(std::forward_like<decltype(content)>(content), result,
			mimetype.empty() ? "text/html" : mimetype, version);
	}

	/**
	 * @brief Respond to http request with pre-prepared error page content
	 * Generated a standard html error page automatically use the status coe 'result',
	 * like <html>...</html>, and the content-type is "text/html" by default.
	 */
	template<class StringT = std::string_view>
	inline http::response<http::string_body> make_error_page_response(
		http::status result, StringT&& desc = std::string_view{},
		std::string_view mimetype = "text/html", unsigned version = 11)
	{
		return make_text_response(http::make_error_page(result, std::forward<StringT>(desc)), result,
			mimetype.empty() ? "text/html" : mimetype, version);
	}

	/**
	 * @brief Respond to http request with local file
	 */
	template<typename = void>
	inline std::expected<http::response<http::file_body>, error_code> make_file_response(
		std::filesystem::path filepath,
		http::status result = http::status::ok, unsigned version = 11)
	{
		// Attempt to open the file
		beast::error_code ec;
		http::file_body::value_type body;
		body.open(filepath.string().c_str(), beast::file_mode::scan, ec);

		// Handle the case where the file doesn't exist
		if (ec == beast::errc::no_such_file_or_directory)
			return std::unexpected(ec);

		// Handle an unknown error
		if (ec)
			return std::unexpected(ec);

		// Cache the size since we need it after the move
		auto const size = body.size();

		// Respond to GET request
		http::response<http::file_body> res{
			std::piecewise_construct,
			std::make_tuple(std::move(body)),
			std::make_tuple(http::status::ok, version) };
		res.set(http::field::server, BEAST_VERSION_STRING);
		res.set(http::field::content_type, http::extension_to_mimetype(filepath.extension().string()));
		res.content_length(size);
		res.result(result);
		res.version(version < 10 ? 11 : version);

		return res;
	}

	/**
	 * @brief Respond to http request with local file
	 */
	template<typename = void>
	inline std::expected<http::response<http::file_body>, error_code> make_file_response(
		std::filesystem::path root_path,
		std::filesystem::path file_path,
		http::status result, unsigned version = 11)
	{
		std::filesystem::path filepath;

		if (root_path.empty())
		{
			filepath = std::move(file_path);
		}
		else
		{
			filepath = asio::make_filepath(root_path, file_path);
		}

		filepath.make_preferred();

		return make_file_response(std::move(filepath), result, version);
	}

	/**
	 * @brief Respond to http request with local file
	 */
	template<typename = void>
	inline std::expected<http::response<http::file_body>, error_code> make_file_response(
		std::filesystem::path root_path,
		beast::string_view file_path,
		http::status result = http::status::ok, unsigned version = 11)
	{
		return make_file_response(std::move(root_path), std::filesystem::path(std::string_view(file_path)), result, version);
	}
}
