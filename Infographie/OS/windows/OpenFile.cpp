#ifdef _WIN32

#include "OS/OpenFile.hpp"

#include "Common.hpp"

#include <Windows.h>

const char* create_cstr_extension_label_map(
	decltype(Open_File_Opts::ext_filters) filters
) noexcept {
	std::string result;

	for (auto&[label, exts] : filters) {
		result += label + '\0';

		for (auto& ext : exts) {
			result += ext + ';';
		}

		if (!exts.empty()) {
			result.pop_back();
		}

		result += '\0';
	}

	char* result_cstr = new char[result.size() + 1];
	memcpy(result_cstr, result.c_str(), sizeof(char) * (result.size() + 1));
	result_cstr[result.size()] = '\0';
	return result_cstr;
}

void open_file_async(
	std::function<void(Open_File_Result)>&& callback, Open_File_Opts opts
) noexcept {
	auto thread = std::thread([opts, callback]() {
		callback(open_file(opts));
	});
	thread.detach();
}

Open_File_Result open_file(Open_File_Opts opts) noexcept {
	constexpr auto BUFFER_SIZE = 512;

	char* filepath = new char[BUFFER_SIZE];
	memcpy(
		filepath,
		opts.filepath.string().c_str(),
		opts.filepath.string().size() + 1
	);
	defer{ delete filepath; };

	char* filename = new char[BUFFER_SIZE];
	memcpy(
		filename,
		opts.filename.string().c_str(),
		opts.filename.string().size() + 1
	);
	defer{ delete filename; };

	const char* filters = create_cstr_extension_label_map(opts.ext_filters);
	defer{ delete filters; };

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = (HWND)opts.owner;
	ofn.lpstrFilter = filters;
	ofn.lpstrFile = filepath;
	ofn.nMaxFile = BUFFER_SIZE;
	ofn.lpstrFileTitle = filename;
	ofn.nMaxFileTitle = BUFFER_SIZE;
	ofn.Flags =
		(opts.allow_multiple ? OFN_ALLOWMULTISELECT : 0) ||
		(opts.prompt_for_create ? OFN_CREATEPROMPT : 0) ||
		(opts.allow_redirect_link ? 0 : OFN_NODEREFERENCELINKS);

	Open_File_Result result;

	if (GetOpenFileName(&ofn)) {
		result.succeded = true;

		// To make sure they are generic.
		result.filename = std::filesystem::path{ filename };
		result.filepath = std::filesystem::path{ filepath };
	}
	else {
		result.succeded = false;
		result.error_code = CommDlgExtendedError();
	}
	return result;
}

#endif