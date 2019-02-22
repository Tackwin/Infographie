#ifdef _WIN32

#include "OS/OpenFile.hpp"

#include "Common.hpp"

#include <Windows.h>
#include <ShObjIdl_core.h>

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

	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
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

	if (GetOpenFileNameA(&ofn)) {
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


void open_dir_async(
	std::function<void(std::optional<std::filesystem::path>)>&& callback
) noexcept {
	std::thread([callback]() {callback(open_dir());}).detach();
}
std::optional<std::filesystem::path> open_dir() noexcept {
	std::optional<std::filesystem::path> result = std::nullopt;
	std::thread{ [&result] {
		constexpr auto BUFFER_SIZE = 2048;
	
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (FAILED(hr)) return;

		IFileDialog* file_dialog;
		auto return_code = CoCreateInstance(
			CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&file_dialog)
		);
		if (FAILED(return_code)) return;
		defer{ file_dialog->Release(); };

		DWORD options;
		if (FAILED(file_dialog->GetOptions(&options))) return;
	
		file_dialog->SetOptions(options | FOS_PICKFOLDERS);

		if (FAILED(file_dialog->Show(NULL))) return;

		IShellItem* psi;
		if (FAILED(file_dialog->GetResult(&psi))) return;
		defer{ psi->Release(); };

		LPWSTR pointer;
		if (FAILED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pointer))) return;
		assert(pointer);

		result = std::filesystem::path{ std::wstring{pointer} };
	}}.join();

	return result;
}

#endif